
#include "python.h"


#include "config.h"
#include "common.h"
#include "db.h"
#include "epgservice.h"


cDbTable* eventsDb = 0;
cDbConnection* connection = 0;
const char* logPrefix = "";

//***************************************************************************
// Init / Exit
//***************************************************************************

void initConnection()
{
   cDbConnection::init();

   cDbConnection::setEncoding("utf8");
   cDbConnection::setHost("localhost");

   cDbConnection::setPort(3306);
   cDbConnection::setName("epg2vdr");
   cDbConnection::setUser("epg2vdr");
   cDbConnection::setPass("epg");
   cDbConnection::setConfPath("/etc/epgd/");

   connection = new cDbConnection();
}

void exitConnection()
{
   cDbConnection::exit();

   if (connection)
      delete connection;
}

int init()
{
   eventsDb = new cDbTable(connection, "useevents");
   if (eventsDb->open() != success) return fail;

   return success;
}

int exit()
{
   delete eventsDb;
   return done;
}

//***************************************************************************
// Main
//***************************************************************************

int main(int argc, char** argv)
{
   cConfigBase::logstdout = yes;
   cConfigBase::loglevel = 0;
   int namingmode = tnmAuto;

   if (argc < 3)
   {
      tell(0, "Usage: pytst <channnelid> <eventid> [<namingmode>]");
      return 1;
   }

   if (argc >= 4)
      namingmode = atoi(argv[3]);

   // at first allpy locale !!

   setlocale(LC_CTYPE, "");

   // read dictionary

   if (dbDict.in("/etc/epgd/epg.dat") != success)
   {
      tell(0, "Invalid dictionary configuration, aborting!");
      return 1;
   }

   initConnection();
   init();

   eventsDb->clear();
   eventsDb->setValue("CNTSOURCE", "tvm");
   eventsDb->setValue("CHANNELID", argv[1]);
   eventsDb->setBigintValue("CNTEVENTID", atol(argv[2]));

   if (!eventsDb->find())
   {
      tell(0, "Event %s/%ld not found", argv[1], atol(argv[2]));
      return 1;
   }

   tell(2, "Event '%s/%s' found",
        eventsDb->getStrValue("TITLE"), eventsDb->getStrValue("SHORTTEXT"));

   // Python stuff ..

   Python py("recording", "name");

   if (py.init("/etc/epgd") != success)
   {
      tell(0, "Init of python failed!");
      return 1;
   }

   if (py.execute(eventsDb, namingmode) == success)
      tell(0, "Info: The recording name calculated by 'recording.py' (with namingmode %d) is '%s'", namingmode, py.getResult());

   py.exit();
   exitConnection();

   return 0;
}
