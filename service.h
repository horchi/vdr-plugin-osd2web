/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  service.h
 *
 *  (c) 2017 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 **/

#ifndef __SERVICE_H
#define __SERVICE_H

//***************************************************************************
// Services
//***************************************************************************

#define OSD2WEB_PORT_SERVICE	"osd2web-WebPortService-v1.0"

struct Osd2Web_Port_v1_0
{
   int webPort;
};

#endif // __SERVICE_H
