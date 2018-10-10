/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  websock.c
 *
 *  (c) 2017-2018 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 **/

#include "update.h"
#include "config.h"

int cWebSock::wsLogLevel = LLL_ERR | LLL_WARN; // LLL_INFO | LLL_NOTICE | LLL_WARN | LLL_ERR
lws_context* cWebSock::context = 0;
char* cWebSock::msgBuffer = 0;
int cWebSock::msgBufferSize = 0;
void* cWebSock::activeClient = 0;
int cWebSock::timeout = 0;
cWebSock::MsgType cWebSock::msgType = mtNone;
std::map<void*,cWebSock::Client> cWebSock::clients;
cMutex cWebSock::clientsMutex;
char* cWebSock::httpPath = 0;

//***************************************************************************
// Init
//***************************************************************************

cWebSock::cWebSock(const char* aHttpPath)
{
   httpPath = strdup(aHttpPath);
   port = 4444;
}

cWebSock::~cWebSock()
{
   exit();

   free(httpPath);
   free(msgBuffer);
}

int cWebSock::init(int aPort, int aTimeout)
{
   struct lws_context_creation_info info;
   const char* interface = 0;
   const char* certPath = 0;           // we're not using ssl
   const char* keyPath = 0;

   lws_set_log_level(wsLogLevel, writeLog);

   port = aPort;
   timeout = aTimeout;

   // setup websocket protocol

   protocols[0].name = "http-only";
   protocols[0].callback = callbackHttp;
   protocols[0].per_session_data_size = sizeof(SessionData);
   protocols[0].rx_buffer_size = 0;

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
   info.ws_ping_pong_interval = timeout;
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
   tell(1, "using libwebsocket version '%s'", lws_get_library_version());

   return success;
}

void cWebSock::writeLog(int level, const char* line)
{
   if (wsLogLevel & level)
      tell(2, "WS: %s", line);
}

int cWebSock::exit()
{
   // lws_context_destroy(context);  #TODO ?

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

   cMutexLock lock(&clientsMutex);

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
// Get Client Info of connection
//***************************************************************************

const char* getClientInfo(lws* wsi, std::string* clientInfo)
{
   char clientName[100+TB] = "unknown";
   char clientIp[50+TB] = "";

   if (wsi)
   {
      int fd = lws_get_socket_fd(wsi);

      if (fd)
         lws_get_peer_addresses(wsi, fd, clientName, sizeof(clientName), clientIp, sizeof(clientIp));
   }

   *clientInfo = clientName + std::string("/") + clientIp;

   return clientInfo->c_str();
}

//***************************************************************************
// HTTP Callback
//***************************************************************************

int cWebSock::callbackHttp(lws* wsi, lws_callback_reasons reason, void* user,
                           void* in, size_t len)
{
   SessionData* sessionData = (SessionData*)user;
   std::string clientInfo = "unknown";

   switch (reason)
   {
      case LWS_CALLBACK_CLOSED:
      {
         getClientInfo(wsi, &clientInfo);

         tell(2, "DEBUG: Got unecpected LWS_CALLBACK_CLOSED for client '%s' (%p)",
              clientInfo.c_str(), (void*)wsi);
         break;
      }
      case LWS_CALLBACK_HTTP_BODY:
      {
         const char* message = (const char*)in;
         int s = lws_hdr_total_length(wsi, WSI_TOKEN_POST_URI);

         tell(0, "DEBUG: Got unecpected LWS_CALLBACK_HTTP_BODY with [%.*s] lws_hdr_total_length is (%d)",
              (int)len+1, message, s);
         break;
      }

      case LWS_CALLBACK_CLIENT_WRITEABLE:
      {
         tell(2, "HTTP: Client writeable");
         break;
      }

      case LWS_CALLBACK_HTTP_WRITEABLE:
      {
         int res;

         tell(3, "HTTP: LWS_CALLBACK_HTTP_WRITEABLE");

         // data to write?

         if (!sessionData->dataPending)
         {
            tell(1, "Info: No more session data pending");
            return -1;
         }

         int m = lws_get_peer_write_allowance(wsi);

         if (!m)
            tell(3, "right now, peer can't handle anything :o");
         else if (m != -1 && m < sessionData->payloadSize)
            tell(0, "peer can't handle %d but %d is needed", m, sessionData->payloadSize);
         else if (m != -1)
            tell(3, "all fine, peer can handle %d bytes", m);

         res = lws_write(wsi, (unsigned char*)sessionData->buffer+sizeLwsPreFrame,
                         sessionData->payloadSize, LWS_WRITE_HTTP);

         if (res < 0)
            tell(0, "Failed writing '%s'", sessionData->buffer+sizeLwsPreFrame);
         else
            tell(2, "WROTE '%s' (%d)", sessionData->buffer+sizeLwsPreFrame, res);

         free(sessionData->buffer);
         memset(sessionData, 0, sizeof(SessionData));

         if (lws_http_transaction_completed(wsi))
            return -1;

         break;
      }

      case LWS_CALLBACK_HTTP:
      {
         int res;
         char* file = 0;
         const char* url = (char*)in;

         memset(sessionData, 0, sizeof(SessionData));

         tell(2, "HTTP: Requested uri: (%ld) '%s'", len, url);

         // data or file request ...

         if (strncmp(url, "/data/", 6) == 0)
         {
            // data request

            res = dispatchDataRequest(wsi, sessionData, url);

            if (res < 0 || (res > 0 && lws_http_transaction_completed(wsi)))
               return -1;
         }
         else
         {
            // file request

            if (strcmp(url, "/") == 0)
               url = "index.html";

            // security

            if (!isEmpty(config.scraper2VdrPath) && strncmp(url, config.scraper2VdrPath, strlen(config.scraper2VdrPath)) == 0)
            {
               // access to scraper2vdr cache is allowed

               asprintf(&file, "%s", url);
            }
            else
            {
               // otherwise force httpPath path to inhibit access to the whole filesystem

               asprintf(&file, "%s/%s", httpPath, url);
            }

            res = serveFile(wsi, file);
            free(file);

            if (res < 0)
            {
               tell(2, "HTTP: Failed, uri: '%s' (%d)", url, res);
               return -1;
            }

            tell(3, "HTTP: Done, uri: '%s'", url);
         }

         break;
      }

      case LWS_CALLBACK_HTTP_FILE_COMPLETION:
      {
         if (lws_http_transaction_completed(wsi))
            return -1;

         break;
      }

      case LWS_CALLBACK_PROTOCOL_INIT:
      case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
      case LWS_CALLBACK_FILTER_HTTP_CONNECTION:
      case LWS_CALLBACK_CLOSED_HTTP:
      case LWS_CALLBACK_WSI_CREATE:
      case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
      case LWS_CALLBACK_ADD_POLL_FD:
      case LWS_CALLBACK_DEL_POLL_FD:
      case LWS_CALLBACK_WSI_DESTROY:
      case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
      case LWS_CALLBACK_LOCK_POLL:
      case LWS_CALLBACK_UNLOCK_POLL:
      case LWS_CALLBACK_GET_THREAD_ID:
      case LWS_CALLBACK_HTTP_BIND_PROTOCOL:
      case LWS_CALLBACK_HTTP_DROP_PROTOCOL:
         break;

      default:
         tell(1, "DEBUG: Unhandled 'callbackHttp' got (%d)", reason);
         break;
   }

   return 0;
}

//***************************************************************************
// Serve File
//***************************************************************************

int cWebSock::serveFile(lws* wsi, const char* path)
{
   const char* suffix = suffixOf(path ? path : "");
   const char* mime = "text/plain";

   // choose mime type based on the file extension

   if (!isEmpty(suffix))
   {
      if      (strcmp(suffix, "png") == 0)   mime = "image/png";
      else if (strcmp(suffix, "jpg") == 0)   mime = "image/jpeg";
      else if (strcmp(suffix, "jpeg") == 0)  mime = "image/jpeg";
      else if (strcmp(suffix, "gif") == 0)   mime = "image/gif";
      else if (strcmp(suffix, "svg") == 0)   mime = "image/svg+xml";
      else if (strcmp(suffix, "html") == 0)  mime = "text/html";
      else if (strcmp(suffix, "css") == 0)   mime = "text/css";
   }

   return lws_serve_http_file(wsi, path, mime, 0, 0);
}

//***************************************************************************
// Callback for osd2vdr Protocol
//***************************************************************************

int cWebSock::callbackOsd2Vdr(lws* wsi, lws_callback_reasons reason,
                              void* user, void* in, size_t len)
{
   std::string clientInfo = "unknown";

   switch (reason)
   {
      case LWS_CALLBACK_SERVER_WRITEABLE:
      case LWS_CALLBACK_RECEIVE:
      case LWS_CALLBACK_ESTABLISHED:
      case LWS_CALLBACK_CLOSED:
      case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
      case LWS_CALLBACK_RECEIVE_PONG:
      {
         getClientInfo(wsi, &clientInfo);
         break;
      }

      default: break;
   }

   switch (reason)
   {
      case LWS_CALLBACK_SERVER_WRITEABLE:                     // data to client
      {
         if (msgType == mtPing)
         {
            if (clients[wsi].type != ctInactive)
            {
               char buffer[sizeLwsFrame];

               tell(2, "DEBUG: Write 'PING' to '%s' (%p)", clientInfo.c_str(), (void*)wsi);
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
                  cMutexLock clock(&clientsMutex);
                  cMutexLock lock(&clients[wsi].messagesOutMutex);
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

               tell(3, "DEBUG: Write (%d) <- %.*s -> to '%s' (%p)\n", msgSize, msgSize,
                    msgBuffer+sizeLwsPreFrame, clientInfo.c_str(), (void*)wsi);

               res = lws_write(wsi, (unsigned char*)msgBuffer + sizeLwsPreFrame, msgSize, LWS_WRITE_TEXT);

               if (res != msgSize)
                  tell(0, "Error: Only (%d) bytes of (%d) sended", res, msgSize);
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
            tell(3, "DEBUG: Got '%s'", message);
            clients[wsi].type = (ClientType)getIntFromJson(oObject, "type", ctInteractive);
            clients[wsi].tftprio = (ClientType)getIntFromJson(oObject, "tftprio", na);
            atLogin(wsi, message, clientInfo.c_str());

            // set only to 'active' if client is interested

            if (clients[wsi].type == ctInteractive)
               activeClient = wsi;
         }
         else if (event == evLogout)                       // { "event" : "logout", "object" : { } }
         {
            tell(3, "DEBUG: Got '%s'", message);
            clients[wsi].type = ctInactive;
            activateAvailableClient();
            clients[wsi].cleanupMessageQueue();
         }
         else if (event == evChannels)
            cUpdate::messagesIn.push(message);
         else if (activeClient && activeClient == wsi)      // accept data only from active client
            cUpdate::messagesIn.push(message);
         else if (activeClient && isHighestViewClient(wsi)) // or no active is available and it is the view clinet with best prio
            cUpdate::messagesIn.push(message);
         else
            tell(1, "Ignoring data of not 'active client (%p)", (void*)wsi);

         json_decref(oData);

         break;
      }

      case LWS_CALLBACK_ESTABLISHED:                       // someone connecting
      {
         // if (timeout)
         //    lws_set_timeout(wsi, PENDING_TIMEOUT_AWAITING_PING, timeout);

         tell(1, "Client '%s' connected (%p), ping time set to (%d)", clientInfo.c_str(), (void*)wsi, timeout);
         clients[wsi].wsi = wsi;
         clients[wsi].type = ctInactive;
         clients[wsi].tftprio = na;

         break;
      }

      case LWS_CALLBACK_CLOSED:                            // someone dis-connecting
      {
         atLogout(wsi, "Client disconnected", clientInfo.c_str());
         break;
      }

      case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
      {
         atLogout(wsi, "Client disconnected unsolicited", clientInfo.c_str());
         break;
      }

      case LWS_CALLBACK_RECEIVE_PONG:                      // ping / pong
      {
         tell(2, "DEBUG: Got 'PONG' from client '%s' (%p)", clientInfo.c_str(), (void*)wsi);

         // if (timeout)
         //    lws_set_timeout(wsi, PENDING_TIMEOUT_AWAITING_PING, timeout);

         break;
      }

      case LWS_CALLBACK_ADD_HEADERS:
      case LWS_CALLBACK_PROTOCOL_INIT:
      case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
         break;

      default:
         tell(1, "DEBUG: Unhandled 'callbackOsd2Vdr' got (%d)", reason);
         break;
   }

   return 0;
}

//***************************************************************************
// Check/Activate Available Client
//***************************************************************************

void cWebSock::activateAvailableClient()
{
   cMutexLock lock(&clientsMutex);

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
// Is Highest View Client (highest => lowest prio value)
//***************************************************************************

int cWebSock::isHighestViewClient(lws* wsi)
{
   cMutexLock lock(&clientsMutex);
   int prio = na;

   for (auto it = clients.begin(); it != clients.end(); ++it)
   {
      if (it->second.type == ctView && it->second.tftprio < prio)
         prio = it->second.tftprio;
   }

   if (clients[wsi].tftprio <= prio)
      return yes;

   return no;
}

//***************************************************************************
// At Login / Logout
//***************************************************************************

void cWebSock::atLogin(lws* wsi, const char* message, const char* clientInfo)
{
   json_t* oContents = json_object();

   tell(1, "Client login '%s' (%p) [%s]", clientInfo, (void*)wsi, message);

   addToJson(oContents, "type", clients[wsi].type);
   addToJson(oContents, "tftprio", clients[wsi].tftprio);
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

void cWebSock::atLogout(lws* wsi, const char* message, const char* clientInfo)
{
   cMutexLock lock(&clientsMutex);

   tell(1, "%s '%s' (%p)", clientInfo, message, (void*)wsi);

   clients.erase(wsi);

   if (!activeClient || activeClient == wsi)
      activateAvailableClient();
}

//***************************************************************************
// Client Count
//***************************************************************************

int cWebSock::getClientCount()
{
   int count = 0;

   cMutexLock lock(&clientsMutex);

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
   cMutexLock lock(&clientsMutex);

   // push message only to connected, not inactiv clients

   if (wsi)
   {
      if (clients.find(wsi) != clients.end())
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
   static char buf[512+TB];
   const char* arg = lws_get_urlarg_by_name(wsi, name, buf, 512);

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

int cWebSock::dispatchDataRequest(lws* wsi, SessionData* sessionData, const char* url)
{
   int statusCode = HTTP_STATUS_NOT_FOUND;

   const char* method = methodOf(url);

   if (strcmp(method, "eventimg") == 0)
      statusCode = doEventImg(wsi);
   else if (strcmp(method, "channellogo") == 0)
      statusCode = doChannelLogo(wsi);
   else if (strcmp(method, "getenv") == 0)
      statusCode = doEnvironment(wsi, sessionData);
   else if (strcmp(method, "recordingimg") == 0)
      statusCode = doRecordingImg(wsi);

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

   asprintf(&path, "%s/%d_%d.jpg", config.epgImagePath, id, no);
   tell(2, "DEBUG: Image for event (%d/%d) was requested [%s]", id, no, path);

   result = serveFile(wsi, path);

   free(path);

   return result;
}

//***************************************************************************
// Do Recording Img
//***************************************************************************

int cWebSock::doRecordingImg(lws* wsi)
{
   const char* suffixFilter = "jpg jpeg";

   int result;
   char* path = strdup(getStrParameter(wsi, "path=", ""));
   const char* suffix = suffixOf(path);

   tell(2, "DEBUG: Image for recording was requested [%s] suffix is '%s'", path, suffix);

   if (isEmpty(suffix) || !strstr(suffixFilter, suffix))
   {
      free(path);
      return fail;
   }

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

   asprintf(&path, "%s/%s.%s",
            config.logoPath,
            config.logoById ? cnlId : cnlName,
            config.logoSuffix);

   tell(2, "DEBUG: Logo for channel '%s' was requested [%s]", cnlName, path);

   result = serveFile(wsi, path);

   free(cnlName);
   free(cnlId);
   free(path);

   return result;
}

//***************************************************************************
// Do Environment
//***************************************************************************

int cWebSock::doEnvironment(lws* wsi, SessionData* sessionData)
{
   static unsigned char header[4096+sizeLwsFrame];
   unsigned char* p = header + sizeLwsPreFrame;
   unsigned char* e = p + sizeof(header) - sizeLwsPreFrame;

   DIR* dirSkin;
   int result = 0;
   char* path = 0;

   json_t* obj = json_object();
   json_t* oEnvironment = json_object();
   json_t* oSkins = json_array();

   asprintf(&path, "%s/skins/", config.httpPath);

   if (!(dirSkin = opendir(path)))
   {
      tell(1, "Can't open directory '%s', '%s'", path, strerror(errno));
      free(path);
      return fail;
   }

   tell(2, "Scanning '%s' for skins", path);

#ifndef HAVE_READDIR_R
   dirent* pSkinEntry;

   while ((pSkinEntry = readdir(dirSkin)))
#else
   dirent skinEntry;
   dirent* pSkinEntry = &skinEntry;
   dirent* skinRes;

   // deprecated but the only reentrant with old libc!

   while (readdir_r(dirSkin, pSkinEntry, &skinRes) == 0 && skinRes)
#endif
   {
      char* themePath = 0;
      DIR* dirTheme;

      tell(4, "Checking '%s'", pSkinEntry->d_name);

      if (dirTypeOf(path, pSkinEntry) != DT_DIR || pSkinEntry->d_name[0] == '.')
         continue;

      json_t* oSkin = json_object();
      json_t* oThemes = json_array();

      asprintf(&themePath, "%s/skins/%s/themes/", config.httpPath, pSkinEntry->d_name);

      if (!(dirTheme = opendir(themePath)))
      {
         tell(1, "Can't open directory '%s', '%s'", pSkinEntry->d_name, strerror(errno));
         free(themePath);
         continue;
      }

      tell(3, "Scanning '%s' for themes", themePath);

#ifndef HAVE_READDIR_R
      dirent* pThemeEntry;

      while ((pThemeEntry = readdir(dirTheme)))
#else
      dirent themeEntry;
      dirent* pThemeEntry = &themeEntry;
      dirent* themeRes;

      // deprecated but the only reentrant with old libc!

      while (readdir_r(dirTheme, pThemeEntry, &themeRes) == 0 && themeRes)
#endif
      {
         tell(4, "Checking '%s'", pThemeEntry->d_name);

         if (dirTypeOf(themePath, pThemeEntry) != DT_REG || pThemeEntry->d_name[0] == '.')
            continue;

         if (rep(pThemeEntry->d_name, ".css$") != success)
            continue;

         json_array_append_new(oThemes, json_string(strReplace(".css", "", pThemeEntry->d_name).c_str()));
      }

      closedir(dirTheme);
      free(themePath);

      addToJson(oSkin, "name", pSkinEntry->d_name);
      addToJson(oSkin, "themes", oThemes);
      json_array_append_new(oSkins, oSkin);

      tell(2, "Found skin '%s'", pSkinEntry->d_name);
   }

   closedir(dirSkin);
   free(path);

   // build json object

   addToJson(oEnvironment, "skins", oSkins);
   addToJson(obj, "event", "environment");
   addToJson(obj, "object", oEnvironment);

   char* strJson = json_dumps(obj, JSON_PRESERVE_ORDER);
   json_decref(obj);

   // store to session data

   sessionData->payloadSize = strlen(strJson);
   sessionData->bufferSize = sizeLwsFrame + sessionData->payloadSize + TB;
   sessionData->buffer = (char*)malloc(sessionData->bufferSize);
   sprintf(sessionData->buffer+sizeLwsPreFrame, "%s", strJson);
   sessionData->dataPending = yes;

   free(strJson);

   // prepare header

   if (!result && lws_add_http_header_status(wsi, HTTP_STATUS_OK, &p, e))
   {
      tell(0, "Error lws_add_http_header_status");
      result = 1;
   }

   if (!result && lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_SERVER,
                                               (unsigned char*)"osd2web", 7, &p, e))
   {
      tell(0, "Error lws_add_http_header_by_token");
      result = 1;
   }

   if (!result && lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE,
                                               (unsigned char*)"application/json", 16, &p, e))
   {
      tell(0, "Error lws_add_http_header_by_token");
      result = 1;
   }

   if (!result && lws_add_http_header_content_length(wsi, sessionData->payloadSize, &p, e))
   {
      tell(0, "Error lws_add_http_header_content_length");
      result = 1;
   }

   if (!result && lws_finalize_http_header(wsi, &p, e))
   {
      tell(0, "Error lws_finalize_http_header");
      result = 1;
   }

   *p = 0;

   // write header

   if (!result && lws_write(wsi, header + sizeLwsPreFrame, p - (header + sizeLwsPreFrame), LWS_WRITE_HTTP_HEADERS) < 0)
   {
      tell(0, "Error: lws_write failed");
      result = 1;
   }

   if (result)
      tell(0, "Error building HTTP header");

   tell(3, "WROTE '%.*s'", (int)(p - (header + sizeLwsPreFrame)), header + sizeLwsPreFrame);

   // book us a LWS_CALLBACK_HTTP_WRITEABLE callback

   lws_callback_on_writable(wsi);

   return 0;
}
