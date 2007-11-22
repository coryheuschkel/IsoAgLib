/***************************************************************************
              can_server_A1.cpp -
                    interface for A1 can card

                             -------------------
    begin                : Tue Oct 2 2001
    copyright            : (C) 1999 - 2004 Dipl.-Inform. Achim Spangler
    email                : a.spangler@osb-ag:de
 ***************************************************************************/


/***************************************************************************
 *                                                                         *
 * This file is part of the "IsoAgLib", an object oriented program library *
 * to serve as a software layer between application specific program and   *
 * communication protocol details. By providing simple function calls for  *
 * jobs like starting a measuring program for a process data value on a    *
 * remote ECU, the main program has not to deal with single CAN telegram   *
 * formatting. This way communication problems between ECU's which use     *
 * this library should be prevented.                                       *
 * Everybody and every company is invited to use this library to make a    *
 * working plug and play standard out of the printed protocol standard.    *
 *                                                                         *
 * Copyright (C) 1999 - 2004 Dipl.-Inform. Achim Spangler                  *
 *                                                                         *
 * The IsoAgLib is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published          *
 * by the Free Software Foundation; either version 2 of the License, or    *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This library is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * General Public License for more details.                                *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with IsoAgLib; if not, write to the Free Software Foundation,     *
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA           *
 *                                                                         *
 * As a special exception, if other files instantiate templates or use     *
 * macros or inline functions from this file, or you compile this file and *
 * link it with other works to produce a work based on this file, this file*
 * does not by itself cause the resulting work to be covered by the GNU    *
 * General Public License. However the source code for this file must still*
 * be made available in accordance with section (3) of the                 *
 * GNU General Public License.                                             *
 *                                                                         *
 * This exception does not invalidate any other reasons why a work based on*
 * this file might be covered by the GNU General Public License.           *
 *                                                                         *
 * Alternative licenses for IsoAgLib may be arranged by contacting         *
 * the main author Achim Spangler by a.spangler@osb-ag:de                  *
 ***************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/version.h>

#define DEF_USE_SERVER_SPECIFIC_HEADER
#include "can_server.h"

struct canMsgA1_s {
        unsigned        id;
        int             msg_type;
        int             len;
        unsigned char   data[8];
        unsigned long   time;           /* timestamp in msec, at read only */
};

/* ioctl request codes */
#define CAN_MAGIC_NUMBER        'z'
#define MYSEQ_START             0x80

using namespace __HAL;

// kernel 2.6 needs type for third argument and not sizeof()
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#define CAN_WRITE_MSG   _IOW(CAN_MAGIC_NUMBER, MYSEQ_START + 1, sizeof(struct canMsgA1_s))
#define CAN_READ_MSG    _IOR(CAN_MAGIC_NUMBER, MYSEQ_START + 2, sizeof(struct canMsgA1_s))
#else
#define CAN_WRITE_MSG   _IOW(CAN_MAGIC_NUMBER, MYSEQ_START + 1, struct canMsgA1_s)
#define CAN_READ_MSG    _IOR(CAN_MAGIC_NUMBER, MYSEQ_START + 2, struct canMsgA1_s)
#endif


static bool  canBusIsOpen[cui32_maxCanBusCnt];

bool isBusOpen(uint8_t ui8_bus)
{
  return canBusIsOpen[ui8_bus];
}

uint32_t initCardApi ()
{
  for( uint32_t i=0; i<cui32_maxCanBusCnt; i++ )
  {
    canBusIsOpen[i] = false;
  }

  return 1;
}

bool resetCard(void)
{
  return true;
}

// PURPOSE: To initialize the specified CAN BUS to begin sending/receiving msgs
bool openBusOnCard(uint8_t ui8_bus, uint32_t wBitrate, server_c* pc_serverData)
{
  DEBUG_PRINT1("init can bus %d\n", ui8_bus);

  int btr0 = 0;
  int btr1 = 1;

  switch ( wBitrate ) {
    case 100: { btr0 = 0xc3; btr1 = 0x3e;} break;
    case 125: { btr0 = 0xc3; btr1 = 0x3a;} break;
    case 250: { btr0 = 0xc1; btr1 = 0x3a;} break;
    case 500: { btr0 = 0xc0; btr1 = 0x3a;} break;
  }

  if( !canBusIsOpen[ui8_bus] )
  {
    DEBUG_PRINT1("Opening CAN BUS ui8_bus=%d\n", ui8_bus);

    char fname[32];
    sprintf( fname, "/dev/wecan%u", ui8_bus );

    DEBUG_PRINT1("open( \"%s\", O_RDRWR)\n", fname);

    pc_serverData->marri16_can_device[ui8_bus] = open(fname, O_RDWR | O_NONBLOCK);

    if (pc_serverData->marri16_can_device[ui8_bus] == -1) {
      DEBUG_PRINT1("Could not open CAN bus%d\n",ui8_bus);
      return 0;
    }

    // Set baud rate to 250 and turn on extended IDs
    // For Opus A1, it is done by sending the following string to the marri16_can_device
    char buf[16];
    sprintf( buf, "i 0x%2x%2x e\n", btr0 & 0xFF, btr1 & 0xFF );     //, (extended?" e":" ") extended is not being passed in! Don't use it!

    DEBUG_PRINT3("write( device-\"%s\"\n, \"%s\", %d)\n", fname, buf, strlen(buf));
    write(pc_serverData->marri16_can_device[ui8_bus], buf, strlen(buf));

    canBusIsOpen[ui8_bus] = true;
  }

  return true;
}


void closeBusOnCard(uint8_t ui8_bus, server_c* /*pc_serverData*/)
{
  DEBUG_PRINT1("close can bus %d\n", ui8_bus);
  //canBusIsOpen[ui8_bus] = false;
  // do not call close or CAN_CLOSE because COMMAND_CLOSE is received during initialization!
}

void __HAL::updatePendingMsgs(server_c* /* pc_serverData */, int8_t /* i8_bus */)
{
/// @todo not implemented for right now on A1!
}

// PURPOSE: To send a msg on the specified CAN BUS
// RETURNS: non-zero if msg was sent ok
//          0 on error
int16_t sendToBus(uint8_t ui8_bus, canMsg_s* ps_canMsg, server_c* pc_serverData)
{
  canMsgA1_s msg;
  msg.id = ps_canMsg->ui32_id;
  msg.msg_type = ( ps_canMsg->i32_msgType ? MSGTYPE_EXTENDED : MSGTYPE_STANDARD );
  msg.len = ps_canMsg->i32_len;
  msg.time = 0;

  for( int i=0; i<msg.len; i++ )
    msg.data[i] = ps_canMsg->ui8_data[i];

  int ret = 0;

  if ((ui8_bus < HAL_CAN_MAX_BUS_NR) && canBusIsOpen[ui8_bus]) {
    ret = ioctl(pc_serverData->marri16_can_device[ui8_bus], CAN_WRITE_MSG, &msg);

#ifdef DEBUG_IOCTL
    if (ret < 0) {
      perror("ca_TransmitCanCard_1 ioctl");

      // try to read
      canMsgA1_s msg;
      ret = ioctl(pc_serverData->marri16_can_device[ui8_bus], CAN_READ_MSG, &msg);
      printf("id 0x%x msg_type 0x%x len 0x%x data 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x time 0x%x\n", msg.id, msg.msg_type, msg.len, msg.data[0], msg.data[1], msg.data[2], msg.data[3], msg.data[4], msg.data[5], msg.data[6], msg.data[7], msg.time);

      if (ret < 0) {
        perror("ioctl read after write");
      }
    }
#endif
  }

  if (ret < 0)
    return 0;
  else
    return 1;
}

uint32_t readFromBus(uint8_t ui8_bus, canMsg_s* ps_canMsg, server_c* pc_serverData)
{
  canMsgA1_s msg;

  if (ioctl(pc_serverData->marri16_can_device[ui8_bus], CAN_READ_MSG, &msg) == 0)
  {
    ps_canMsg->ui32_id = msg.id;

    if (msg.msg_type > 0)
      ps_canMsg->i32_msgType = 1;
    else
      ps_canMsg->i32_msgType = 0;

    ps_canMsg->i32_len = msg.len;

    memcpy(ps_canMsg->ui8_data, msg.data, msg.len );

    return msg.len;
  }

  return 0;

}

void addSendTimeStampToList(client_c * /*ps_client*/, int32_t /*i32_sendTimeStamp*/)
{
}

