//***************************************************************************
// Web Socket Interface
// File websock.h
// This code is distributed under the terms and conditions of the
// GNU GENERAL PUBLIC LICENSE. See the file LICENSE for details.
// Date 16.04.2020 - Jörg Wendel
//***************************************************************************

#pragma once

#include <queue>
#include <jansson.h>
#include <libwebsockets.h>

//***************************************************************************
// Class OSD Service
//***************************************************************************

class cOsdService
{
   public:

      enum ObjectShape
      {
         osText   = 0x01,
         osSmall  = 0x02,
         osLarge  = 0x04
      };

      enum Event
      {
         evUnknown,
         evTakeFocus,
         evLeaveFocus,
         evKeyPress,
         evChannels,
         evMaxLines,
         evLogin,
         evLogout,
         evCommand,

         evCount
      };

      enum ClientType
      {
         ctInactive = na,
         ctInteractive,            // = 0 - for interactive browser session
         ctView,                   // = 1 - e.g. for graphtft display
         ctFB                      // = 2 - pure FB (remote control) clients
      };

      static const char* toName(Event event);
      static Event toEvent(const char* name);

      static const char* events[];

      struct cCutMark
      {
         int position;
         std::string comment;
      };

      struct cCuttingMarks
      {
         cMutex mutex;
         std::queue<cCutMark> queue;
         int isSet;
      };
};

//***************************************************************************
// Class Web Interface
//***************************************************************************

class cWebInterface // : public cWebService
{
   public:

      virtual const char* myName() = 0;
      virtual int pushInMessage(const char* data) = 0;
};

//***************************************************************************
// Class cWebSock
//***************************************************************************

class cWebSock : public cOsdService
{
   public:

      enum MsgType
      {
         mtNone = na,
         mtPing,    // 0
         mtData     // 1
      };

      enum Protokoll
      {
         sizeLwsPreFrame  = LWS_SEND_BUFFER_PRE_PADDING,
         sizeLwsPostFrame = LWS_SEND_BUFFER_POST_PADDING,
         sizeLwsFrame     = sizeLwsPreFrame + sizeLwsPostFrame
      };

      struct SessionData
      {
         char* buffer {};
         int bufferSize {0};
         int payloadSize {0};
         int dataPending {no};
      };

      struct Client
      {
         ~Client() { free(msgBuffer); }

         ClientType type {};
         int tftprio {0};
         std::queue<std::string> messagesOut;
         cMutex messagesOutMutex;
         void* wsi {};

         // buffer to send the payload in chunks

         unsigned char* msgBuffer {};
         int msgBufferSize {0};
         int msgBufferPayloadSize {0};
         int msgBufferSendOffset {0};
         bool msgBufferDataPending() { return msgBufferSendOffset < msgBufferPayloadSize; }

         // push next message

         void pushMessage(const char* p)
         {
            cMutexLock lock(&messagesOutMutex);
            messagesOut.push(p);
         }

         void cleanupMessageQueue()
         {
            cMutexLock lock(&messagesOutMutex);

            // just in case client is not connected and wasted messages are pending

            tell(0, "Info: Flushing (%zu) old 'wasted' messages of client (%p)", messagesOut.size(), wsi);

            while (!messagesOut.empty())
               messagesOut.pop();
         }

         std::string buffer;   // for chunked messages
      };

      cWebSock(cWebInterface* aProcess, const char* aHttpPath);
      virtual ~cWebSock();

      int init(int aPort, int aTimeout, const char* confDir, bool ssl);
      int exit();

      // sync thread stuff

      struct ThreadControl
      {
         bool active {false};
         bool close {false};
         cWebSock* webSock {};
         int timeout {60};
      };

      pthread_t syncThread {0};
      ThreadControl threadCtl;

      static void* syncFct(void* user);
      static int service(ThreadControl* threadCtl);

      int performData(MsgType type);

      // status callback methods

      static int wsLogLevel;
      static int callbackHttp(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len);
      static int callbackWs(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len);

      // static interface

      static void activateAvailableClient();
      static int isHighestViewClient(lws* wsi);
      static void atLogin(lws* wsi, const char* message, const char* clientInfo);
      static void atLogout(lws* wsi, const char* message, const char* clientInfo);
      static int getClientCount();

      static void pushOutMessage(const char* p, lws* wsi = 0);
      static void writeLog(int level, const char* line);

   private:

      static int serveFile(lws* wsi, const char* path);
      static int dispatchDataRequest(lws* wsi, SessionData* sessionData, const char* url);
      static int doEventImg(lws* wsi);
      static int doRecordingImg(lws* wsi);
      static int doChannelLogo(lws* wsi);
      static int doEnvironment(lws* wsi, SessionData* sessionData);

      static const char* methodOf(const char* url);
      static const char* getStrParameter(lws* wsi, const char* name, const char* def = 0);
      static int getIntParameter(lws* wsi, const char* name, int def = na);

      //

      int port {na};
      char* certFile {};
      char* certKeyFile {};
      lws_protocols protocols[3];
      lws_http_mount mounts[1];
#if defined (LWS_LIBRARY_VERSION_MAJOR) && (LWS_LIBRARY_VERSION_MAJOR >= 4)
      lws_retry_bo_t retry;
#endif

      // statics

      static lws_context* context;
      static cWebInterface* singleton;
      static char* httpPath;
      static char* epgImagePath;
      static int timeout;
      static void* activeClient;
      static std::map<void*,Client> clients;
      static cMutex clientsMutex;
      static MsgType msgType;

      // only used in callback

      // static char* msgBuffer;
      // static int msgBufferSize;
};
