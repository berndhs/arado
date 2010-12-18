
/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (C) 2010, Arado Team
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef WIN32
#include <winsock2.h>
#define snprintf _snprintf
#endif
#include "miniupnp/miniwget.h"
#include "miniupnp/miniupnpc.h"
#include "miniupnp/upnpcommands.h"
#include "miniupnp/upnperrors.h"

#include "upnpclient.h"

namespace arado
{

static const char proto[4] = { 'T', 'C', 'P', 0};
static const char mappingName[]="arado";


UPnPClient::UPnPClient(QObject *parent) :
  QObject(parent)
{
  lanaddr[0]=0;
  hasUPnP=false;
}

UPnPClient::~UPnPClient()
{
  if(hasUPnP) {
    RemoveRedirection();
  }
}


bool UPnPClient::RemoveRedirection()
{
  if (!hasUPnP) return false;
  struct UPNPUrls *urls=&this->urls;
  struct IGDdatas *data=&this->data;
  if (eport[0]!=0) {
    return UPNP_DeletePortMapping(urls->controlURL, data->first.servicetype, eport, proto, 0);
  }
  return false;
}

bool UPnPClient::ChangeRedirection(const char * eport)
{
  if (!hasUPnP) {
     printf ("No uPnP\n");
    return false;
  }
  printf ("uPhP for port %s\n",eport);
  if (strcmp(eport,this->eport)==0) return true;
  RemoveRedirection();
  strcpy (this->eport,eport);
  return AddRedirection(eport,eport);
}

bool UPnPClient::AddRedirection(const char * iport,const char * eport)
{
  char externalIPAddress[16];
  char intClient[16];
  char intPort[6];
  int r;
  struct UPNPUrls *urls=&this->urls;
  struct IGDdatas *data=&this->data;

  if(!hasUPnP) return false;

  if (!iport || !eport) {
    fprintf(stderr, "Wrong arguments\n");
    return false;
  }

  UPNP_GetExternalIPAddress(urls->controlURL,
                            data->first.servicetype,
                            externalIPAddress);
  if (externalIPAddress[0]) {
    printf("ExternalIPAddress = %s\n", externalIPAddress);
  } else {
    printf("GetExternalIPAddress failed.\n");
  }

  r = UPNP_AddPortMapping(urls->controlURL, data->first.servicetype,
                          eport, iport, lanaddr, mappingName, proto, 0);
  if(r!=UPNPCOMMAND_SUCCESS) {
    printf("AddPortMapping(%s, %s, %s) failed with code %d (%s)\n",
           eport, iport, lanaddr, r, strupnperror(r));
  }

  r = UPNP_GetSpecificPortMappingEntry(urls->controlURL,
                                       data->first.servicetype,
                                       eport, proto,
                                       intClient, intPort);
  if (r!=UPNPCOMMAND_SUCCESS) {
    printf("GetSpecificPortMappingEntry() failed with code %d (%s)\n",
           r, strupnperror(r));
  }

  if (intClient[0]) {
    printf("InternalIP:Port = %s:%s\n", intClient, intPort);
    printf("external %s:%s %s is redirected to internal %s:%s\n",
           externalIPAddress, eport, proto, intClient, intPort);
  }
  return true;
}



bool UPnPClient::Init()
{
  struct UPNPDev * devlist = 0;
  int i;
  const char * rootdescurl = 0;
  const char * multicastif = 0;
  const char * minissdpdpath = 0;


  if ( rootdescurl 
      || (devlist = upnpDiscover(2000, multicastif, minissdpdpath, 0))) {
    struct UPNPDev * device;

    if(devlist) {
      printf("List of UPNP devices found on the network :\n");
      for(device = devlist; device; device = device->pNext) {
        printf(" desc: %s\n st: %s\n\n",
               device->descURL, device->st);
      }
    }
    i = 1;
    if ( (rootdescurl && UPNP_GetIGDFromUrl(rootdescurl, 
                          &urls, &data, lanaddr, sizeof(lanaddr)))
        || (i = UPNP_GetValidIGD(devlist, &urls, 
                          &data, lanaddr, sizeof(lanaddr)))) {
      switch(i) {
      case 1:
        printf("Found valid IGD : %s\n", urls.controlURL);
        hasUPnP=true;
        return true;
      case 2:
        printf("Found a (not connected?) IGD : %s\n", urls.controlURL);
        printf("Trying to continue anyway\n");
        break;
      case 3:
        printf("UPnP device found. Is it an IGD ? : %s\n", urls.controlURL);
        printf("Trying to continue anyway\n");
        break;
      default:
        printf("Found device (igd ?) : %s\n", urls.controlURL);
        printf("Trying to continue anyway\n");
      }
      printf("Local LAN ip address : %s\n", lanaddr);
    }
  }
  return false;
}

};
