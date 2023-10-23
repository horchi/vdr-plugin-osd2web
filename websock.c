/**
 *  websock.c
 *
 *  (c) 2017-2021 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 **/

#include "update.h"
#include "config.h"

#include "websock.h"

// LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO | LLL_DEBUG | LLL_PARSER |
// LLL_HEADER | LLL_EXT | LLL_CLIENT | LLL_LATENCY | LLL_USER | LLL_THREAD

int cWebSock::wsLogLevel {LLL_ERR | LLL_WARN};
lws_context* cWebSock::context {};
int cWebSock::timeout {0};
cWebSock::MsgType cWebSock::msgType {mtNone};
std::map<void*,cWebSock::Client> cWebSock::clients;
cMutex cWebSock::clientsMutex;
char* cWebSock::httpPath {};
void* cWebSock::activeClient {};

cWebInterface* cWebSock::singleton {};

//***************************************************************************
// Web Socket
//***************************************************************************

cWebSock::cWebSock(cWebInterface* aProcess, const char* aHttpPath)
{
   cWebSock::singleton = aProcess;
   httpPath = strdup(aHttpPath);
}

cWebSock::~cWebSock()
{
   exit();
   free(certFile);
   free(certKeyFile);
   free(httpPath);
}

int cWebSock::init(int aPort, int aTimeout, const char* confDir, bool ssl)
{
   lws_context_creation_info info {0};

   lws_set_log_level(wsLogLevel, writeLog);

   port = aPort;
   timeout = aTimeout;

   // setup websocket protocol

   protocols[0].name = "http-only";
   protocols[0].callback = callbackHttp;
   protocols[0].per_session_data_size = sizeof(SessionData);
   protocols[0].rx_buffer_size = 0;

   protocols[1].name = singleton->myName();
   protocols[1].callback = callbackWs;
   protocols[1].per_session_data_size = 0;
   protocols[1].rx_buffer_size = 128*1024;
   protocols[1].tx_packet_size = 0;

   protocols[2].name = 0;
   protocols[2].callback = 0;
   protocols[2].per_session_data_size = 0;
   protocols[2].rx_buffer_size = 0;

   // mounts

   bool noStore {false};       // for debugging on iOS
   lws_http_mount mount {0};

   mount.mount_next = (lws_http_mount*)nullptr;
   mount.mountpoint = "/";
   mount.origin = httpPath;
   mount.mountpoint_len = 1;
   mount.cache_max_age = noStore ? 0 : 86400;
   mount.cache_reusable = !noStore;       // 0 => no-store
   mount.cache_revalidate = 1;
   mount.cache_intermediaries = 1;

#ifdef LWS_FEATURE_MOUNT_NO_CACHE
   mount.cache_no = noStore ? 0 : 1;
#endif
   mount.origin_protocol = LWSMPRO_FILE;
   mount.basic_auth_login_file = nullptr;
   mounts[0] = mount;

   // setup websocket context info

   memset(&info, 0, sizeof(info));
   info.options = 0;
   info.mounts = mounts;
   info.gid = -1;
   info.uid = -1;
   info.port = port;
   info.protocols = protocols;
#if defined (LWS_LIBRARY_VERSION_MAJOR) && (LWS_LIBRARY_VERSION_MAJOR < 4)
   info.iface = nullptr;
#endif
   info.ssl_cert_filepath = nullptr;
   info.ssl_private_key_filepath = nullptr;

   if (ssl)
   {
      free(certFile);
      free(certKeyFile);
      asprintf(&certFile, "%s/epg2vdr.cert", confDir);
      asprintf(&certKeyFile, "%s/epg2vdr.key", confDir);
      tell(0, "Starting SSL mode with '%s' / '%s'", certFile, certKeyFile);

      info.ssl_cert_filepath = certFile;
      info.ssl_private_key_filepath = certKeyFile;
      info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT; // | LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
   }

#if defined (LWS_LIBRARY_VERSION_MAJOR) && (LWS_LIBRARY_VERSION_MAJOR >= 4)
   retry.secs_since_valid_ping = timeout;
   retry.secs_since_valid_hangup = timeout + 10;
   info.retry_and_idle_policy = &retry;
#else
   info.ws_ping_pong_interval = timeout;
#endif

   // create libwebsocket context representing this server

   context = lws_create_context(&info);

   if (!context)
   {
      tell(0, "Error: libwebsocket init failed");
      return fail;
   }

   tell(0, "Listener at port (%d) established", port);
   tell(1, "Using libwebsocket version '%s'", lws_get_library_version());

   threadCtl.webSock = this;
   threadCtl.timeout = timeout;

   if (pthread_create(&syncThread, NULL, syncFct, &threadCtl))
   {
      tell(0, "Error: Failed to start client daemon thread");
      return fail;
   }

   return success;
}

void cWebSock::writeLog(int level, const char* line)
{
   std::string message = strReplace("\n", "", line);
   tell(0, "WS: (%d) %s", level, message.c_str());
}

int cWebSock::exit()
{
   if (syncThread)
   {
      threadCtl.close = true;
      lws_cancel_service(context);

      time_t endWait = time(0) + 2;  // 2 second for regular end

      while (threadCtl.active && time(0) < endWait)
         usleep(100);

      if (threadCtl.active)
         pthread_cancel(syncThread);
      else
         pthread_join(syncThread, 0);

      syncThread = 0;
   }

   lws_context_destroy(context);

   return success;
}

//***************************************************************************
// Sync
//***************************************************************************

void* cWebSock::syncFct(void* user)
{
   ThreadControl* threadCtl = (ThreadControl*)user;
   threadCtl->active = true;

   tell(3, " :: started syncThread");

   while (!threadCtl->close)
   {
      service(threadCtl);
   }

   threadCtl->active = false;
   // printf(" :: stopped syncThread regular\n");

   return nullptr;
}

//***************************************************************************
// Service
//***************************************************************************

int cWebSock::service(ThreadControl* threadCtl)
{
   static time_t nextWebSocketPing {0};

#if defined (LWS_LIBRARY_VERSION_MAJOR) && (LWS_LIBRARY_VERSION_MAJOR >= 4)
   lws_service(context, 0);   // timeout parameter is not supported by the lib anymore
#else
   lws_service(context, 100);
#endif

   threadCtl->webSock->performData(cWebSock::mtData);

   if (nextWebSocketPing < time(0))
   {
      threadCtl->webSock->performData(cWebSock::mtPing);
      nextWebSocketPing = time(0) + threadCtl->timeout-5;
   }

   return done;
}

//***************************************************************************
// Perform Data
//***************************************************************************

int cWebSock::performData(MsgType type)
{
   int count {0};

   cMyMutexLock lock(&clientsMutex);

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

void getClientInfo(lws* wsi, std::string* clientInfo)
{
   char clientName[100+TB] {"unknown"};
   // char clientIp[50+TB] {""};

   if (wsi)
   {
      // int fd {0};
      // lws_get_peer_addresses take up to 10 seconds on some environments !!
      // if ((fd = lws_get_socket_fd(wsi)))
      //    lws_get_peer_addresses(wsi, fd, clientName, sizeof(clientName), clientIp, sizeof(clientIp));

      // we can use lws_get_peer_simple instead

      lws_get_peer_simple(wsi, clientName, sizeof(clientName));
   }

   *clientInfo = clientName; //  + std::string("/") + clientIp;
}

//***************************************************************************
// HTTP Callback
//***************************************************************************

int cWebSock::callbackHttp(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len)
{
   SessionData* sessionData {(SessionData*)user};
   std::string clientInfo {"unknown"};

   tell(4, "DEBUG: 'callbackHttp' got (%d)", reason);

   switch (reason)
   {
      case LWS_CALLBACK_CLOSED:
      {
         getClientInfo(wsi, &clientInfo);

         tell(2, "DEBUG: Got unexpected LWS_CALLBACK_CLOSED for client '%s' (%p)", clientInfo.c_str(), (void*)wsi);
         break;
      }
      case LWS_CALLBACK_HTTP_BODY:
      {
         const char* message {(const char*)in};
         int s {lws_hdr_total_length(wsi, WSI_TOKEN_POST_URI)};
         tell(1, "DEBUG: Got unexpected LWS_CALLBACK_HTTP_BODY with [%.*s] lws_hdr_total_length is (%d)", (int)len+1, message, s);
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

         if (!sessionData || !sessionData->dataPending)
         {
            tell(1, "Info: No more session data pending");
            return -1;
         }

         long long m {lws_get_peer_write_allowance(wsi)};

         if (!m)
            tell(3, "right now, peer can't handle anything :o");
         else if (m != -1 && m < sessionData->payloadSize)
            tell(0, "peer can't handle %lld but %d is needed", m, sessionData->payloadSize);
         else if (m != -1)
            tell(3, "all fine, peer can handle %lld bytes", m);

         res = lws_write(wsi, (unsigned char*)sessionData->buffer+sizeLwsPreFrame,
                         sessionData->payloadSize, LWS_WRITE_HTTP);

         if (res < 0)
            tell(0, "Failed writing '%s'", sessionData->buffer+sizeLwsPreFrame);
         else
            tell(2, "WROTE '%s' (%d)", sessionData->buffer+sizeLwsPreFrame, res);

         free(sessionData->buffer);
         *sessionData = {};

         if (lws_http_transaction_completed(wsi))
            return -1;

         break;
      }

      case LWS_CALLBACK_HTTP:
      {
         int res {success};
         char* file {};
         const char* url {(char*)in};
         *sessionData = {};

         tell(1, "HTTP: Requested uri: (%ld) '%s'", (ulong)len, url);

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

            // security, force httpPath path to inhibit access to the whole filesystem

            if (!isEmpty(config.scraper2VdrPath) && strncmp(url, config.scraper2VdrPath, strlen(config.scraper2VdrPath)) == 0)
               asprintf(&file, "%s", url);  // access to scraper2vdr cache is allowed
            else
               asprintf(&file, "%s/%s", httpPath, url);

            res = serveFile(wsi, file);
            free(file);

            if (res < 0)
            {
               tell(2, "HTTP: Failed to serve url '%s' (%d)", url, res);
               return -1;
            }

            tell(3, "HTTP: Done, url: '%s'", url);
         }

         break;
      }

      case LWS_CALLBACK_HTTP_FILE_COMPLETION:     // 15
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
      case LWS_CALLBACK_HTTP_BIND_PROTOCOL:       // 49
      case LWS_CALLBACK_HTTP_DROP_PROTOCOL:       // 50
#if LWS_LIBRARY_VERSION_MAJOR >= 3
      case LWS_CALLBACK_HTTP_CONFIRM_UPGRADE:
      case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
      case LWS_CALLBACK_HTTP_BODY_COMPLETION:
#endif
         break;
      case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS: // 21,
      case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS: // 22,
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

   // LogDuration ld("serveFile", 1);

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
      else if (strcmp(suffix, "js") == 0)    mime = "application/javascript";
      else if (strcmp(suffix, "map") == 0)   mime = "application/json";
   }

   // printf("serve file '%s' with mime type '%s'\n", path, mime);

   return lws_serve_http_file(wsi, path, mime, 0, 0);
}

//***************************************************************************
// Callback for my Protocol
//***************************************************************************

int cWebSock::callbackWs(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len)
{
   std::string clientInfo = "unknown";

   tell(4, "DEBUG: 'callback' got (%d)", reason);

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
            if (!clients[wsi].msgBufferDataPending() && clients[wsi].messagesOut.empty())
               return 0;
            if (lws_send_pipe_choked(wsi))
               return 0;

            if (!clients[wsi].msgBufferDataPending() && !clients[wsi].messagesOut.empty())
            {
               cMyMutexLock clock(&clientsMutex);
               cMyMutexLock lock(&clients[wsi].messagesOutMutex);
               const char* msg = clients[wsi].messagesOut.front().c_str();
               int msgSize = clients[wsi].messagesOut.front().length();
               int neededSize = sizeLwsFrame + msgSize;
               unsigned char* newBuffer {};

               if (neededSize > clients[wsi].msgBufferSize)
               {
                  // tell(eloDebugWebSock, "Debug: re-allocate buffer to %d bytes", neededSize);

                  if (!(newBuffer = (unsigned char*)realloc(clients[wsi].msgBuffer, neededSize)))
                  {
                     tell(2, "Fatal: Can't allocate memory!");
                     return -1;
                  }

                  clients[wsi].msgBuffer = newBuffer;
                  clients[wsi].msgBufferSize = neededSize;
               }

               strncpy((char*)clients[wsi].msgBuffer + sizeLwsPreFrame, msg, msgSize);
               clients[wsi].msgBufferPayloadSize = msgSize;
               clients[wsi].msgBufferSendOffset = 0;
               clients[wsi].messagesOut.pop();  // remove sent message

               tell(1, "=> (%d) %.*s -> to '%s' (%p)", msgSize, msgSize,
                    clients[wsi].msgBuffer+sizeLwsPreFrame, clientInfo.c_str(), (void*)wsi);
            }

            enum { maxChunk = 10*1024 };

            if (clients[wsi].msgBufferPayloadSize)
            {
               unsigned char* p = clients[wsi].msgBuffer + sizeLwsPreFrame + clients[wsi].msgBufferSendOffset;
               int pending = clients[wsi].msgBufferPayloadSize - clients[wsi].msgBufferSendOffset;
               int chunkSize = pending > maxChunk ? maxChunk : pending;
               int flags = lws_write_ws_flags(LWS_WRITE_TEXT, !clients[wsi].msgBufferSendOffset, pending <= maxChunk);
               int res = lws_write(wsi, p, chunkSize, (lws_write_protocol)flags);

               if (res < 0)
               {
                  tell(0, "Error: lws_write chunk failed with (%d) to (%p) failed (%d) [%.*s]", res, (void*)wsi, chunkSize, chunkSize, p);
                  return -1;
               }

               clients[wsi].msgBufferSendOffset += chunkSize;

               if (clients[wsi].msgBufferSendOffset >= clients[wsi].msgBufferPayloadSize)
               {
                  clients[wsi].msgBufferPayloadSize = 0;
                  clients[wsi].msgBufferSendOffset = 0;
               }
               else
                  lws_callback_on_writable(wsi);
            }
         }

         break;
      }

      case LWS_CALLBACK_RECEIVE:                           // data from client
      {
         json_t *oData {}, *oObject {};
         json_error_t error;
         Event event;

         tell(3, "DEBUG: 'LWS_CALLBACK_RECEIVE' [%.*s]", (int)len, (const char*)in);

         {
            cMyMutexLock lock(&clientsMutex);

            if (lws_is_first_fragment(wsi))
               clients[wsi].buffer.clear();

            clients[wsi].buffer.append((const char*)in, len);

            if (!lws_is_final_fragment(wsi))
            {
               tell(3, "DEBUG: Got %zd bytes, have now %zd -> more to get", len, clients[wsi].buffer.length());
               break;
            }

            oData = json_loadb(clients[wsi].buffer.c_str(), clients[wsi].buffer.length(), 0, &error);

            if (!oData)
            {
               tell(0, "Error: Ignoring invalid jason request [%s]", clients[wsi].buffer.c_str());
               tell(0, "Error decoding json: %s (%s, line %d column %d, position %d)",
                    error.text, error.source, error.line, error.column, error.position);
               break;
            }

            clients[wsi].buffer.clear();
         }

         char* message = strndup((const char*)in, len);
         const char* strEvent = getStringFromJson(oData, "event", "<null>");
         event = cOsdService::toEvent(strEvent);
         oObject = json_object_get(oData, "object");

         tell(3, "DEBUG: Got '%s'", message);

         if (event == evLogin)                             // { "event" : "login", "object" : { "type" : 0 } }
         {
            clients[wsi].type = (ClientType)getIntFromJson(oObject, "type", ctInteractive);
            clients[wsi].tftprio = (ClientType)getIntFromJson(oObject, "tftprio", 100);
            atLogin(wsi, message, clientInfo.c_str());

            // set only to 'active' if client is interested

            if (clients[wsi].type == ctInteractive)
               activeClient = wsi;
         }
         else if (event == evLogout)                       // { "event" : "logout", "object" : { } }
         {
            clients[wsi].type = ctInactive;
            activateAvailableClient();
            clients[wsi].cleanupMessageQueue();
         }
         else if (event == evChannels)
            singleton->pushInMessage(message);
         else if (activeClient && activeClient == wsi)       // accept data only from active client
            singleton->pushInMessage(message);

         else if (event == evKeyPress && clients[wsi].type == ctFB)
            singleton->pushInMessage(message);

         else if (!activeClient && isHighestViewClient(wsi)) // or no active is available and it is the view clinet with best prio
         {
            tell(2, "Debug: Taking data of view client, prio (%d) [%s]", clients[wsi].tftprio, message);
            singleton->pushInMessage(message);
         }
         else
            tell(2, "Debug: Ignoring data of not 'active' client (%p) prio (%d) %s [%s]",
                 (void*)wsi, clients[wsi].tftprio,
                 activeClient ? "at least one active clinet is connected" : "", message);

         json_decref(oData);
         free(message);

         break;
      }

      case LWS_CALLBACK_ESTABLISHED:                       // someone connecting
      {
         tell(1, "Client '%s' connected (%p), ping time set to (%d)", clientInfo.c_str(), (void*)wsi, timeout);
         clients[wsi].wsi = wsi;
         clients[wsi].type = ctInactive;
         clients[wsi].tftprio = 100;

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
         break;
      }

      case LWS_CALLBACK_ADD_HEADERS:
      case LWS_CALLBACK_PROTOCOL_INIT:
      case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
#if LWS_LIBRARY_VERSION_MAJOR >= 3
      case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
      case LWS_CALLBACK_HTTP_BIND_PROTOCOL:
#endif
         break;

      default:
         tell(1, "DEBUG: Unhandled 'callbackWs' got (%d)", reason);
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
   int prio = 999;

   for (auto it = clients.begin(); it != clients.end(); ++it)
   {
      if (it->second.type == ctView && it->second.tftprio < prio)
         prio = it->second.tftprio;
   }

   tell(1, "lowest prio is (%d)", prio);

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

   singleton->pushInMessage(p);
   free(p);
}

void cWebSock::atLogout(lws* wsi, const char* message, const char* clientInfo)
{
   tell(1, "%s '%s' (%p)", clientInfo, message, (void*)wsi);

   cMutexLock lock(&clientsMutex);

   clients.erase(wsi);

   if (activeClient == wsi)
      activeClient = 0;

   if (!activeClient)
      activateAvailableClient();
}

//***************************************************************************
// Client Count
//***************************************************************************

int cWebSock::getClientCount()
{
   int count = 0;

   cMyMutexLock lock(&clientsMutex);

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

void cWebSock::pushOutMessage(const char* message, lws* wsi)
{
   cMutexLock lock(&clientsMutex);

   if (wsi)
   {
      if (clients.find(wsi) != clients.end())
         clients[wsi].pushMessage(message);
      else if ((ulong)wsi != (ulong)-1)
         tell(0, "client %ld not found!", (ulong)wsi);
   }
   else
   {
      for (auto it = clients.begin(); it != clients.end(); ++it)
      {
         if (it->second.type != ctInactive && it->second.type != ctFB)
            it->second.pushMessage(message);
      }
   }
}

//***************************************************************************
// Get Integer Parameter
//***************************************************************************

int cWebSock::getIntParameter(lws* wsi, const char* name, int def)
{
   char buf[100] {};
   const char* arg = lws_get_urlarg_by_name(wsi, name, buf, 100);

   return arg ? atoi(arg) : def;
}

//***************************************************************************
// Get String Parameter
//***************************************************************************

const char* cWebSock::getStrParameter(lws* wsi, const char* name, const char* def)
{
   static char buf[512+TB] {};
   const char* arg = lws_get_urlarg_by_name(wsi, name, buf, 512);

   return arg ? arg : def;
}

//***************************************************************************
// Method Of
//***************************************************************************

const char* cWebSock::methodOf(const char* url)
{
   const char* p {};

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
   if (!sessionData)
   {
      tell(0, "Fatal: Missing session data, aborting");
      return fail;
   }

   static unsigned char header[4096+sizeLwsFrame];
   unsigned char* p = header + sizeLwsPreFrame;
   unsigned char* e = p + sizeof(header) - sizeLwsPreFrame;

   DIR* dirSkin {};
   int result {0};
   char* path {};

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
   dirent* pSkinEntry {};

   while ((pSkinEntry = readdir(dirSkin)))
#else
   dirent skinEntry;
   dirent* pSkinEntry {&skinEntry};
   dirent* skinRes {};

   // deprecated but the only reentrant with old libc!

   while (readdir_r(dirSkin, pSkinEntry, &skinRes) == 0 && skinRes)
#endif
   {
      char* themePath {};
      DIR* dirTheme {};

      if (dirTypeOf(path, pSkinEntry) != DT_DIR || pSkinEntry->d_name[0] == '.')
         continue;

      tell(4, "Checking '%s'", pSkinEntry->d_name);

      json_t* oSkin {json_object()};
      json_t* oThemes {json_array()};

      asprintf(&themePath, "%s/skins/%s/themes/", config.httpPath, pSkinEntry->d_name);

      tell(4, "DDD: Open theme path '%s'", themePath);

      if (!(dirTheme = opendir(themePath)))
      {
         tell(1, "Can't open directory '%s', '%s'", pSkinEntry->d_name, strerror(errno));
         free(themePath);
         continue;
      }

      tell(4, "Scanning '%s' for themes", themePath);

#ifndef HAVE_READDIR_R
      dirent* pThemeEntry {};

      while ((pThemeEntry = readdir(dirTheme)))
#else
      dirent themeEntry;
      dirent* pThemeEntry {&themeEntry};
      dirent* themeRes {};

      // deprecated but the only reentrant with old libc!

      while (readdir_r(dirTheme, pThemeEntry, &themeRes) == 0 && themeRes)
#endif
      {
         if (dirTypeOf(themePath, pThemeEntry) != DT_REG || pThemeEntry->d_name[0] == '.')
            continue;

         if (rep(pThemeEntry->d_name, ".css$") != success)
            continue;

         tell(3, "Appending '%s' to array", pThemeEntry->d_name);
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

   if (!result && lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_SERVER, (unsigned char*)"osd2web", 7, &p, e))
   {
      tell(0, "Error lws_add_http_header_by_token");
      result = 1;
   }

   if (!result && lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE, (unsigned char*)"application/json", 16, &p, e))
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
