/***************************************************************************
                          idevpropertyhandler_c.h  - central managing instance for
                                           all process data informations in
                                           the system
                             -------------------
    begin                : Fri Apr 07 2000
    copyright            : (C) 2000 - 2009 by Dipl.-Inform. Achim Spangler
    email                : a.spangler@osb-ag:de
    type                 : Header
    $LastChangedDate$
    $LastChangedRevision$
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
 * Copyright (C) 2000 - 2009 Dipl.-Inform. Achim Spangler                  *
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
#ifndef IDEVPROPERTYHANDLER_C_H
#define IDEVPROPERTYHANDLER_C_H

/* *************************************** */
/* ********** include headers ************ */
/* *************************************** */
#include "impl/devpropertyhandler_c.h"


// Begin Namespace IsoAgLib::iDevPropertyHandler_c
namespace IsoAgLib {

/**
*/
class iDevPropertyHandler_c : public __IsoAgLib::DevPropertyHandler_c  {
public:

  /**
  Receive pointer to byte array, create LanguageLabel, replace old one in Map or create a new one
  */
  bool registerDevicePool (const IsoAgLib::iIdentItem_c* apc_wsMasterIdentItem, const HUGE_MEM uint8_t* apc_devicePoolByteArray, const uint32_t aui32_bytestreamLength, bool ab_setToDefault)
    { return DevPropertyHandler_c::registerDevicePool (static_cast<const __IsoAgLib::IdentItem_c*>(apc_wsMasterIdentItem), apc_devicePoolByteArray, aui32_bytestreamLength, ab_setToDefault); }

  bool sendCommandChangeDesignator(uint16_t apui16_objectID, const char* apc_newString, uint8_t stringLength)
    { return DevPropertyHandler_c::sendCommandChangeDesignator(apui16_objectID, apc_newString, stringLength); }

private:
  friend class iProcess_c;
};

} // end of namespace IsoAgLib::iDevPropertyHandler_c
#endif
