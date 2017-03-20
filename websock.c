/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  websock.c
 *
 *  (c) 2017 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 **/

#include "update.h"
#include "config.h"

char* cWebSock::msgBuffer = 0;
int cWebSock::msgBufferSize = 0;
void* cWebSock::activeClient = 0;
int cWebSock::timeout = 0;
cWebSock::MsgType cWebSock::msgType = mtNone;
std::map<void*,cWebSock::Client> cWebSock::clients;
char* cWebSock::cfgPath = 0;
char* cWebSock::epgImagePath = 0;

//***************************************************************************
// Init
//***************************************************************************

cWebSock::cWebSock(const char* aCfgPath, const char* aEpgImagePath)
{
   cfgPath = strdup(aCfgPath);
   epgImagePath = strdup(aEpgImagePath);

   port = 4444;
   context = 0;
}

cWebSock::~cWebSock()
{
   exit();

   free(cfgPath);
   free(msgBuffer);
}

int cWebSock::init(int aPort, int aTimeout)
{
   struct lws_context_creation_info info;
   const char* interface = 0;
   const char* certPath = 0;           // we're not using ssl
   const char* keyPath = 0;

   port = aPort;
   timeout = aTimeout;

   // setup websocket protocol

   protocols[0].name = "http-only";
   protocols[0].callback = callbackHttp;
   protocols[0].per_session_data_size = 0;
   protocols[0].rx_buffer_size = 20;

   protocols[1].name = "osd2vdr";
   protocols[1].callback = callbackOsd2Vdr;
   protocols[1].per_session_data_size = 0;
   protocols[1].rx_buffer_size = 80*1024;

   protocols[2].name = 0;
   protocols[2].callback = 0;
   protocols[2].per_session_data_size = 0;
   protocols[2].rx_buffer_size = 0;

   // setup websocket context info

   memset(&info, 0, sizeof(info));
   info.port = port;
   info.iface = interface;
   info.protocols = protocols;
   info.ssl_cert_filepath = certPath;
   info.ssl_private_key_filepath = keyPath;
   info.gid = -1;
   info.uid = -1;
   info.options = 0;

   // create libwebsocket context representing this server

   context = lws_create_context(&info);

   if (!context)
   {
      tell(0, "Error: libwebsocket init failed");
      return fail;
   }

   tell(0, "Listener at port (%d) established", port);

   return success;
}

int cWebSock::exit()
{
   lws_context_destroy(context);

   return success;
}

//***************************************************************************
// Service
//***************************************************************************

int cWebSock::service(int timeoutMs)
{
   lws_service(context, timeoutMs);
   return done;
}

//***************************************************************************
// Perform Data
//***************************************************************************

int cWebSock::performData(MsgType type)
{
   int count = 0;

   msgType = type;

   for (auto it = clients.begin(); it != clients.end(); ++it)
   {
      if (it->second.type != ctInactive && !it->second.messagesOut.empty())
         count++;
   }

   if (count || msgType == mtPing)
      lws_callback_on_writable_all_protocol(context, &protocols[1]);

   return done;
}

//***************************************************************************
// HTTP Callback
//***************************************************************************

int cWebSock::callbackHttp(lws* wsi, lws_callback_reasons reason, void* user,
                           void* in, size_t len)
{
   switch (reason)
   {
      case LWS_CALLBACK_CLIENT_WRITEABLE:
      {
         tell(2, "HTTP: connection established\n");
         break;
      }

      case LWS_CALLBACK_HTTP:
      {
         int res;
         char* file = 0;
         const char* url = (char*)in;

         tell(2, "HTTP: Requested uri: (%ld) '%s'", len, url);

         // data or file request ...

         if (strncmp(url, "/data/", 6) == 0)
         {
            // data request

            res = dispatchDataRequest(wsi, url);

            if (res < 0 || (res > 0 && lws_http_transaction_completed(wsi)))
               return -1;
         }
         else
         {
            // file  request

            if (strcmp(url, "/") == 0)
               url = "index.html";

            asprintf(&file, "%s/http/%s", cfgPath, url);
            res = serveFile(wsi, file);
            free(file);

            if (res < 0 || (res > 0 && lws_http_transaction_completed(wsi)))
               return -1;
         }

         break;
      }

      case LWS_CALLBACK_GET_THREAD_ID:
         break;

      default:
         tell(2, "DEBUG: Unhandled 'callbackHttp' got (%d)", reason);
         break;
   }

   return 0;
}

//***************************************************************************
// Serve File
//***************************************************************************

int cWebSock::serveFile(lws* wsi, const char* path)
{
   const char* extension;
   const char* mime = "text/plain";

   extension = strrchr(path, '.');

   // choose mime type based on the file extension

   if (!isEmpty(extension))
   {
      if      (strcmp(extension, ".png") == 0)   mime = "image/png";
      else if (strcmp(extension, ".jpg") == 0)   mime = "image/jpg";
      else if (strcmp(extension, ".gif") == 0)   mime = "image/gif";
      else if (strcmp(extension, ".svg") == 0)   mime = "image/svg+xml";
      else if (strcmp(extension, ".html") == 0)  mime = "text/html";
      else if (strcmp(extension, ".css") == 0)   mime = "text/css";
   }

   return lws_serve_http_file(wsi, path, mime, 0, 0);
}

//***************************************************************************
// Callback for osd2vdr Protocol
//***************************************************************************

int cWebSock::callbackOsd2Vdr(lws* wsi, lws_callback_reasons reason,
                              void* user, void* in, size_t len)
{
   switch (reason)
   {
      case LWS_CALLBACK_SERVER_WRITEABLE:                     // data to client
      {
         if (msgType == mtPing)
         {
            if (clients[wsi].type != ctInactive)
            {
               char buffer[sizeLwsFrame];
               tell(2, "DEBUG: Write 'PING' (%p)", (void*)wsi);
               lws_write(wsi, (unsigned char*)buffer + sizeLwsPreFrame, 0, LWS_WRITE_PING);
            }
         }

         else if (msgType == mtData)
         {
            while (!clients[wsi].messagesOut.empty() && !lws_send_pipe_choked(wsi))
            {
               int res;
               std::string msg;  // take a copy of the message for short mutex lock!

               // mutex lock context
               {
                  cMutexLock(&clients[wsi].messagesOutMutex);
                  msg = clients[wsi].messagesOut.front();
                  clients[wsi].messagesOut.pop();
               }

               int msgSize = strlen(msg.c_str());
               int neededSize = sizeLwsFrame + msgSize;
               char* newBuffer = 0;

               if (neededSize > msgBufferSize)
               {
                  if (!(newBuffer = (char*)realloc(msgBuffer, neededSize)))
                  {
                     tell(0, "Fatal: Can't allocate memory!");
                     break;
                  }

                  msgBuffer = newBuffer;
                  msgBufferSize = neededSize;
               }

               strncpy(msgBuffer + sizeLwsPreFrame, msg.c_str(), msgSize);

               tell(1, "DEBUG: Write (%d) [%.*s] to (%p)\n",
                    msgSize, msgSize,
                    msgBuffer+sizeLwsPreFrame, (void*)wsi);

               res = lws_write(wsi, (unsigned char*)msgBuffer + sizeLwsPreFrame, msgSize, LWS_WRITE_TEXT);

               if (res != msgSize)
                  tell(0, "Error: Only (%d) bytes of (%d) sendet", res, msgSize);
            }
         }

         break;
      }

      case LWS_CALLBACK_RECEIVE:                           // data from client
      {
         json_t *oData, *oObject;
         json_error_t error;
         Event event;
         const char* message = (const char*)in;

         tell(3, "DEBUG: 'LWS_CALLBACK_RECEIVE' [%s]", message);

         if (!(oData = json_loads(message, 0, &error)))
         {
            tell(0, "Error: Ignoring unexpeted client request [%s]", message);
            break;
         }

         event = cOsdService::toEvent(getStringFromJson(oData, "event", "<null>"));
         oObject = json_object_get(oData, "object");

         if (event == evLogin)                             // { "event" : "login", "object" : { "type" : 0 } }
         {
            tell(1, "DEBUG: Got '%s'", message);
            clients[wsi].type = (ClientType)getIntFromJson(oObject, "type", ctInteractive);
            atLogin(wsi);
            activeClient = wsi;
         }
         else if (event == evLogout)                       // { "event" : "logout", "object" : { } }
         {
            tell(2, "DEBUG: Got '%s'", message);
            clients[wsi].type = ctInactive;
            activateAvailableClient();
            clients[wsi].cleanupMessageQueue();
         }
         else if (activeClient && activeClient == wsi)     // take data only from active client
            cUpdate::messagesIn.push(message);
         else
            tell(1, "Ignoring data of not 'active client (%p)", (void*)wsi);

         json_decref(oData);

         break;
      }

      case LWS_CALLBACK_ESTABLISHED:                       // someone connecting
      {
         if (timeout)
            lws_set_timeout(wsi, PENDING_TIMEOUT_AWAITING_PING, timeout);

         tell(1, "Client connected (%p)", (void*)wsi);
         clients[wsi].wsi = wsi;
         clients[wsi].type = ctInactive;

         break;
      }

      case LWS_CALLBACK_CLOSED:                           // someone dis-connecting
      {
         tell(1, "Client disconnected (%p)", (void*)wsi);
         clients.erase(wsi);

         if (!activeClient || activeClient == wsi)
            activateAvailableClient();

         break;
      }

      case LWS_CALLBACK_RECEIVE_PONG:                      // ping / pong
      {
         tell(2, "DEBUG: Got 'PONG'");

         if (timeout)
            lws_set_timeout(wsi, PENDING_TIMEOUT_AWAITING_PING, timeout);

         break;
      }

      default:
         tell(2, "DEBUG: Unhandled 'callbackOsd2Vdr' got (%d)", reason);
         break;
   }

   return 0;
}

//***************************************************************************
// Check/Activate Available Client
//***************************************************************************

void cWebSock::activateAvailableClient()
{
   auto it = clients.find(activeClient);

   if (!activeClient || it == clients.end() || it->second.type == ctInactive)
   {
      for (it = clients.begin(); it != clients.end(); ++it)
      {
         if (it->second.type == ctInteractive)
         {
            activeClient = it->first;
            tell(1, "Set client (%p) to active", activeClient);
            break;
         }
      }
   }
}

//***************************************************************************
// At Login
//***************************************************************************

void cWebSock::atLogin(lws* wsi)
{
   json_t* oContents = json_object();

   addToJson(oContents, "type", clients[wsi].type);
   addToJson(oContents, "client", (long)wsi);
   addToJson(oContents, "lastclient", (long)activeClient);

   json_t* obj = json_object();
   addToJson(obj, "event", "login");
   json_object_set_new(obj, "object", oContents);

   char* p = json_dumps(obj, JSON_PRESERVE_ORDER);
   json_decref(obj);

   cUpdate::messagesIn.push(p);
   free(p);
}

//***************************************************************************
// Client Count
//***************************************************************************

int cWebSock::getClientCount()
{
   int count = 0;

   for (auto it = clients.begin(); it != clients.end(); ++it)
   {
      if (it->second.type != ctInactive)
         count++;
   }

   return count;
}

//***************************************************************************
// Push Message
//***************************************************************************

void cWebSock::pushMessage(const char* message, lws* wsi)
{
   // push message only to connected, not inactiv clients

   if (wsi)
   {
      clients[wsi].pushMessage(message);
   }
   else
   {
      for (auto it = clients.begin(); it != clients.end(); ++it)
      {
         if (it->second.type != ctInactive)
            it->second.pushMessage(message);
      }
   }
}

//***************************************************************************
// Get Integer Parameter
//***************************************************************************

int cWebSock::getIntParameter(lws* wsi, const char* name, int def)
{
   char buf[100];
   const char* arg = lws_get_urlarg_by_name(wsi, name, buf, 100);

   return arg ? atoi(arg) : def;
}

//***************************************************************************
// Get String Parameter
//***************************************************************************

const char* cWebSock::getStrParameter(lws* wsi, const char* name, const char* def)
{
   static char buf[100];
   const char* arg = lws_get_urlarg_by_name(wsi, name, buf, 100);

   return arg ? arg : def;
}

//***************************************************************************
// Method Of
//***************************************************************************

const char* cWebSock::methodOf(const char* url)
{
   const char* p;

   if (url && (p = strchr(url+1, '/')))
      return p+1;

   return "";
}

//***************************************************************************
// Dispatch Data Request
//***************************************************************************

int cWebSock::dispatchDataRequest(lws* wsi, const char* url)
{
   int statusCode = HTTP_STATUS_NOT_FOUND;

   const char* method = methodOf(url);

   if (strcmp(method, "eventimg") == 0)
      statusCode = doEventImg(wsi);
   else if (strcmp(method, "channellogo") == 0)
      statusCode = doChannelLogo(wsi);

   return statusCode;
}

//***************************************************************************
// Do Event Img
//***************************************************************************

int cWebSock::doEventImg(lws* wsi)
{
   int result;
   char* path = 0;
   int id = getIntParameter(wsi, "id=");
   int no = getIntParameter(wsi, "no=");

   asprintf(&path, "%s/%d_%d.jpg", epgImagePath, id, no);
   tell(2, "DEBUG: Image for event (%d/%d) was requested [%s]", id, no, path);

   result = serveFile(wsi, path);

   free(path);

   return result;
}

//***************************************************************************
// Do Channel Logo
//***************************************************************************

int cWebSock::doChannelLogo(lws* wsi)
{
   int result;
   char* path = 0;
   char* cnlName = strdup(getStrParameter(wsi, "name=", ""));
   char* cnlId = strdup(getStrParameter(wsi, "id=", ""));

   if (!config.logoNotLower)
      toCase(cLower, cnlName);

   asprintf(&path, "%s/channellogos/%s.%s",
            cfgPath,
            config.logoById ? cnlId : cnlName,
            config.logoSuffix);
   tell(2, "DEBUG: Logo for channel '%s' was requested [%s]", cnlName, path);

   result = serveFile(wsi, path);

   free(cnlName);
   free(cnlId);
   free(path);

   return result;
}
