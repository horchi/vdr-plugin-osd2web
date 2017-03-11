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

char* cWebSock::msgBuffer = 0;
int cWebSock::msgBufferSize = 0;
void* cWebSock::activeClient = 0;
int cWebSock::timeout = 0;
cWebSock::MsgType cWebSock::msgType = mtNone;
std::map<void*,cWebSock::Client> cWebSock::clients;

//***************************************************************************
// Init
//***************************************************************************

cWebSock::cWebSock()
{
   port = 4444;
   context = 0;
}

cWebSock::~cWebSock()
{
   exit();

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
   protocols[1].rx_buffer_size = 40*1024;

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
      tell(0, "libwebsocket init failed");
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

int cWebSock::callbackHttp(lws* wsi,
                           lws_callback_reasons reason, void* user,
                           void* in, size_t len)
{
   switch (reason)
   {
      case LWS_CALLBACK_CLIENT_WRITEABLE:
      {
         tell(0, "HTTP: connection established\n");
         break;
      }

      case LWS_CALLBACK_HTTP:
      {
         const char* mime = "text/plain";
         char* extension;
         char* file = 0;
         const char* url = (char*)in;

         tell(0, "HTTP: Requested uri: (%ld) '%s'", len, url);

         if (strcmp(url, "/") == 0)
            url = "index.html";

         asprintf(&file, "%s/%s", "/var/lib/vdr/plugins/osd2web/http", url);
         extension = strrchr(file, '.');

         // choose mime type based on the file extension

         if (!isEmpty(extension))
         {
            if      (strcmp(extension, ".png") == 0)   mime = "image/png";
            else if (strcmp(extension, ".jpg") == 0)   mime = "image/jpg";
            else if (strcmp(extension, ".gif") == 0)   mime = "image/gif";
            else if (strcmp(extension, ".html") == 0)  mime = "text/html";
            else if (strcmp(extension, ".css") == 0)   mime = "text/css";
         }

         lws_serve_http_file(wsi, file, mime, 0, 0);

         break;
      }

      case LWS_CALLBACK_GET_THREAD_ID:
         break;

      default:
         tell(2, "DEBUG: 'callbackHttp' got (%d)", reason);
         break;
   }

   return 0;
}

//***************************************************************************
// Callback for osd2vdr Protocol
//***************************************************************************

int cWebSock::callbackOsd2Vdr(lws* wsi, lws_callback_reasons reason,
                              void* user, void* in, size_t len)
{
   switch (reason)
   {
      case LWS_CALLBACK_SERVER_WRITEABLE:         // data to client
      {
         if (msgType == mtPing)
         {
            if (clients[wsi].type != ctInactive)
            {
               char buffer[sizeLwsFrame];
               tell(1, "DEBUG: Write 'PING' (%p)", (void*)wsi);
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
                  cMutexLock(&cUpdate::messagesOutMutex);
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

               tell(0, "DEBUG: Write (%d) [%.*s] to (%p)\n",
                    msgSize, msgSize,
                    msgBuffer+sizeLwsPreFrame, (void*)wsi);

               res = lws_write(wsi, (unsigned char*)msgBuffer + sizeLwsPreFrame, msgSize, LWS_WRITE_TEXT);

               if (res != msgSize)
                  tell(0, "Error: Only (%d) bytes of (%d) sendet", res, msgSize);
            }
         }

         break;
      }

      case LWS_CALLBACK_RECEIVE:                  // data from client
      {
         json_t *oData, *oObject;
         json_error_t error;
         Event event;
         const char* message = (const char*)in;

         tell(3, "DEBUG: 'LWS_CALLBACK_RECEIVE' [%s]", message);

         if (!( oData = json_loads(message, 0, &error)))
         {
            tell(0, "Error: Ignoring unexpeted client request [%s]", message);
            break;
         }

         event = cOsdService::toEvent(getStringFromJson(oData, "event", "<null>"));
         oObject = json_object_get(oData, "object");

         if (event == evLogin)                             // { "event" : "login", "object" : { "type" : 0 } }
         {
            clients[wsi].type = (ClientType)getIntFromJson(oObject, "type", ctInteractive);

            if (clients[wsi].type == ctInteractive)
            {
               if (activeClient)
                  clients[activeClient].pushMessage("{ \"event\" : \"rolechange\", \"object\" : { \"role\" : \"passive\" } }");

               activeClient = wsi;
               clients[activeClient].pushMessage("{ \"event\" : \"rolechange\", \"object\" : { \"role\" : \"active\" } }");
            }
         }
         else if (event == evLogout)                       // { "event" : "logout", "object" : { } }
         {
            clients[wsi].type = ctInactive;
            activateAvailableClient();
            clients[wsi].cleanupMessageQueue();
         }
         else if (activeClient && activeClient == wsi)     // take data only from active client
            cUpdate::messagesIn.push(message);
         else
            tell(0, "Ignoring data of not 'active client (%p)", (void*)wsi);

         json_decref(oData);

         break;
      }

      case LWS_CALLBACK_ESTABLISHED:              // someone connecting
      {
         if (timeout)
            lws_set_timeout(wsi, PENDING_TIMEOUT_AWAITING_PING, timeout);

         tell(0, "Client connected (%p)", (void*)wsi);
         clients[wsi].wsi = wsi;
         clients[wsi].type = ctInactive;

         break;
      }

      case LWS_CALLBACK_CLOSED:                   // someone dis-connecting
      {
         tell(0, "Client disconnected (%p)", (void*)wsi);
         clients.erase(wsi);

         if (!activeClient || activeClient == wsi)
            activateAvailableClient();

         break;
      }

      case LWS_CALLBACK_RECEIVE_PONG:             // ping / pong
      {
         tell(1, "DEBUG: Got 'PONG'");

         if (timeout)
            lws_set_timeout(wsi, PENDING_TIMEOUT_AWAITING_PING, timeout);

         break;
      }

      default:
         tell(0, "DEBUG: 'callbackOsd2Vdr' got (%d)", reason);
         break;
   }

   return 0;
}

//***************************************************************************
// Check Activate Available Client
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
            tell(0, "Set client (%p) to active", activeClient);
            break;
         }
      }
   }
}

//***************************************************************************
// Push Message
//***************************************************************************

void cWebSock::pushMessage(const char* message)
{
   // push message only to connected, not inactiv clients

   for (auto it = clients.begin(); it != clients.end(); ++it)
   {
      if (it->second.type != ctInactive)
         it->second.pushMessage(message);
   }
}
