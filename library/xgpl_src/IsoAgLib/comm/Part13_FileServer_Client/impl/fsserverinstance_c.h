/*
  fsserverinstance_c.h

  (C) Copyright 2009 - 2010 by OSB AG and developing partners

  See the repository-log for details on the authors and file-history.
  (Repository information can be found at <http://isoaglib.com/download>)

  Use, modification and distribution are subject to the GNU General
  Public License with exceptions for ISOAgLib. (See accompanying
  file LICENSE.txt or copy at <http://isoaglib.com/download/license>)
*/
#ifndef FSSERVERINSTANCE_C_H
#define FSSERVERINSTANCE_C_H

// own
#include "../ifsstructs.h"

// ISOAgLib
#include <IsoAgLib/hal/hal_typedef.h>
#include <IsoAgLib/driver/can/impl/cancustomer_c.h>
#include <IsoAgLib/comm/Part5_NetworkManagement/impl/isoname_c.h>
#include <IsoAgLib/comm/Part5_NetworkManagement/impl/isoitem_c.h>
#include <IsoAgLib/comm/Part5_NetworkManagement/impl/isofilterbox_c.h>
#include <IsoAgLib/util/iassert.h>


namespace IsoAgLib
{
  // forward declarations
  class iFsServerInstance_c;
}


namespace __IsoAgLib
{
// forward declarations
class FsManager_c;

class FsServerInstance_c : public CanCustomer_c
{
  public:
    // c'tor
    FsServerInstance_c(const IsoItem_c &pref_newItem, FsManager_c &ref_fsManager);
    // d'tor
    ~FsServerInstance_c();


    /** States that are set by the fileserver itself with status messages. */
    enum FsBusy_en
    {
      busy_none = 0x00,
      busy_reading = 0x01,
      busy_writing = 0x02
    };

    /** Main FS states */
    enum FsState_en
    {
      offline,  // AC is there, node is known as FileServerOrPrinter
      online,   // FS Status Message is being received => start communication
      usable,   // FsCSCs now know about this FS and it is free to be used now...
      unusable  // FS did something uncompliant (not answered or wrong answers, etc.)
    };

    enum FsVersion_en
    {
      FsVersionDIS = 0,
      FsVersionFDIS = 1,
      FsVersionIS = 2
    };

  private:
    /** the fileservers manager */
    FsManager_c &c_fsManager;
    /** the fileserver's isoitem */
    const IsoItem_c &pc_newItem;
    /** fileserver property, busy wrinting/reading */
    FsBusy_en en_busy;
    /** fileserver property number of open files */
    uint8_t ui8_nrOpenFiles;
    /** last time a fileserver status message has been received. */
    int32_t i32_lastTime;

    /** The fileServer Properties */
    /** fileserver's version number*/
    uint8_t ui8_versionNumber;
    /** fileservers maximum number of simultaniously open files. */
    uint8_t ui8_maximumNrFiles;
    /** does fileserver support multiple volumes? etc. etc... */
    uint8_t ui8_capabilities;

    /** the fileservers volumes*/
    IsoAgLib::iFsDirList v_volumes;

    /** initialization state of the fileserver, as described at the enum */
    FsState_en men_state;

    /** Initialization-Filter stored for proper destruction */
    IsoFilter_s ms_receiveFilter;

  public:
    /**
      * Get the fileserver's state. The values are described in the corresponding enum.
      */
    FsState_en getState() { return men_state; }

    /**
      * Set the fileserver's state. The values are described in the corresponding enum.
      */
    void setState (FsState_en aen_newState);

    // verify if FS in being queried for properties/volumes
    bool isBeingInitialized() { return (getState() <= online); }

    /**
      * sets the fileserver's volumes. this value is set after the property-request.
      * therefore it is reported to all known fsclientservercommunciations.
      */
    void setVolumes(IsoAgLib::iFsDirList v_inVolumes);

    /**
      * Get the fileserver's volumes.
      */
    IsoAgLib::iFsDirList getFsVolumes() {return v_volumes; }

    /**
      * have to fileserver's volumes been requested successfully?
      */
    bool knowsVolumes() { return (v_volumes.size() != 0);}

    /**
      * get the fileserver's source-addresse.
      */
    uint8_t getSa() {return pc_newItem.nr(); }

    /**
      * get the fileserver's isoitem.
      */
    const IsoItem_c &getIsoItem() {  return pc_newItem; }

    /**
      * get the fileserver's isoname
      */
    const IsoName_c &getIsoName() {  return pc_newItem.isoName(); }

    /**
      * returns the last time a fileserver statusmessage has been received.
      */
    int32_t getLastTime() { return i32_lastTime; }

    /**
      * sets the fileservers properties as defined in the ISO-11783 standard
      */
    void setFsProperties(uint8_t rui8_versionNumber, uint8_t rui8_maximumNrFiles, uint8_t rui8_capabilities);

    uint8_t getMaxNrOpenFiles() { return ui8_maximumNrFiles; }
    /**
      * does the fileserver support multivolumes?
      */
    bool getSupportsMultiVolumes() { return (ui8_capabilities & 0x01); }

    /** explicit conversion to reference of interface class type */
    IsoAgLib::iFsServerInstance_c* toInterfacePointer();

    /** gets the version of the ISO-standard used by the fileserver
      * @return number of standard used as defined in Part 13 Filserver
      */
   uint8_t getStandardVersion() { return ui8_versionNumber; }

    /**
      * get the fileserver's busy status. the values are described in the corresponding enum.
      */
   FsBusy_en getBusy() { return en_busy; }

   /**
    * stuff used for receiving can-packets
    */
   CANPkgExt_c& dataBase(){return c_data;}
   CANPkgExt_c& data(){return c_data;}
   bool processMsg();

   CANPkgExt_c c_data;

   /** time-triggered operations */
   void timeEvent();
};


} // namespace __IsoAgLib

#endif
