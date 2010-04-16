/*
  tracmovesetpoint_c.cpp: working on Base Data Msg Type 1; stores,
    updates and delivers all base data informations from CanCustomer_c
    derived for CAN sending and receiving interaction; from
    Scheduler_Task_c derived for interaction with other IsoAgLib
    objects

  (C) Copyright 2009 - 2010 by OSB AG and developing partners

  See the repository-log for details on the authors and file-history.
  (Repository information can be found at <http://isoaglib.com/download>)

  Use, modification and distribution are subject to the GNU General
  Public License with exceptions for ISOAgLib. (See accompanying
  file LICENSE.txt or copy at <http://isoaglib.com/download/license>)
*/

#include <IsoAgLib/driver/can/impl/canio_c.h>
#include <IsoAgLib/comm/Part5_NetworkManagement/impl/isomonitor_c.h>
#include <IsoAgLib/comm/Part5_NetworkManagement/impl/isomonitor_c.h>
#if defined(USE_BASE) || defined(USE_TRACTOR_MOVE)
  #include "tracmove_c.h"
#endif
#include "tracmovesetpoint_c.h"


using namespace std;

namespace __IsoAgLib { // Begin Namespace __IsoAglib

  #if defined(PRT_INSTANCE_CNT) && (PRT_INSTANCE_CNT > 1)
  /** C-style function, to get access to the unique TracMoveSetPoint_c singleton instance
    * if more than one CAN BUS is used for IsoAgLib, an m_index must be given to select the wanted BUS
    */
  TracMoveSetPoint_c& getTracMoveSetPointInstance( uint8_t aui8_instance )
  { // if > 1 singleton instance is used, no static reference can be used
    return TracMoveSetPoint_c::instance( aui8_instance );
  };
  #else
  /** C-style function, to get access to the unique TracMoveSetPoint_c singleton instance */
  TracMoveSetPoint_c& getTracMoveSetPointInstance( void )
  {
    static TracMoveSetPoint_c& c_lbsTracMoveSetPoint = TracMoveSetPoint_c::instance();
    return c_lbsTracMoveSetPoint;
  };
  #endif


    /** initialize directly after the singleton instance is created.
        this is called from singleton.h and should NOT be called from the user again.
        users please use init(...) instead.
      */
      void TracMoveSetPoint_c::singletonInit()
      { // singletonInit is called, AFTER the initializing instance() function has assigned a suitable
        // singleton vec key - this key value is NOT available at construction time!!!
        BaseCommon_c::singletonInitBase(SINGLETON_VEC_KEY);
      }



  /** initialise element which can't be done during construct;
      above all create the needed FilterBox_c instances
      possible errors:
        * dependant error in CanIo_c problems during insertion of new FilterBox_c entries for IsoAgLibBase
      @param apc_isoName optional pointer to the ISOName variable of the ersponsible member instance (pointer enables automatic value update if var val is changed)
      @param ai_singletonVecKey singleton vector key in case PRT_INSTANCE_CNT > 1
      @param at_identMode either IsoAgLib::IdentModeImplement or IsoAgLib::IdentModeTractor
    */
  void TracMoveSetPoint_c::init_base (const IsoName_c* apc_isoName, int /*ai_singletonVecKey*/, IsoAgLib::IdentMode_t at_identMode)
  {
    //call init for handling which is base data independent
    BaseCommon_c::init_base (apc_isoName, getSingletonVecKey(), at_identMode);
  };

  /** config the TracMoveSetPoint_c object after init -> set pointer to isoName and
      config send/receive of different base msg types
      @param apc_isoName pointer to the ISOName variable of the ersponsible member instance (pointer enables automatic value update if var val is changed)
      @param at_identMode either IsoAgLib::IdentModeImplement or IsoAgLib::IdentModeTractor
      @return true -> configuration was successfull
    */
  bool TracMoveSetPoint_c::config_base (const IsoName_c* apc_isoName, IsoAgLib::IdentMode_t at_identMode, uint16_t aui16_suppressMask)
  {
    //call config for handling which is base data independent
    //if something went wrong leave function before something is configured
    if ( !BaseCommon_c::config_base (apc_isoName, at_identMode, aui16_suppressMask) ) return false;

    ///Set time Period for Scheduler_c --> periodic action only if in IMPLEMENT MODE!!
    if (at_identMode == IsoAgLib::IdentModeImplement) setTimePeriod( (uint16_t) 100);
    else  setTimePeriod( (uint16_t) 1000   );

    // set configure values
    mi16_selectedSpeedSetPointLimit = 0;
    mi16_selectedSpeedSetPointCmd = 0;
    mt_selectedDirectionCmd = IsoAgLib::IsoNotAvailableDirection;

    return true;
  }

  /** check if filter boxes shall be created - create only ISO filters based
      on active local idents which has already claimed an address
      --> avoid to much Filter Boxes
    */
  void TracMoveSetPoint_c::checkCreateReceiveFilter()
  {
    IsoMonitor_c& c_isoMonitor = getIsoMonitorInstance4Comm();
    CanIo_c &c_can = getCanInstance4Comm();

    if ((!checkFilterCreated()) && (c_isoMonitor.existActiveLocalIsoMember()))
    { // check if needed receive filters for ISO are active
      setFilterCreated();

      // create FilterBox_c for PGN SELECTED_SPEED_CMDClientBase, PF 254 - mask for DP, PF and PS
      // mask: (0x3FFFF << 8) filter: (SELECTED_SPEED_CMD << 8)
      c_can.insertFilter(*this, (static_cast<MASK_TYPE>(0x3FFFF) << 8),
                        (static_cast<MASK_TYPE>(SELECTED_SPEED_CMD) << 8), false, Ident_c::ExtendedIdent);
    }
  }

  /** process a ISO11783 base information PGN
      @pre  sender of message is existent in monitor list
      @see  CanPkgExt_c::resolveSendingInformation()
    */
  bool TracMoveSetPoint_c::processMsg()
  {
    // there is no need to check if sender exist in the monitor list because this is already done
    // in CanPkgExt_c -> resolveSendingInformation
    //IsoName_c const& rcc_tempISOName = data().getISONameForSA();

    switch (data().isoPgn() /*& 0x3FFFF*/) // don't need to &, we're interested in the whole PGN
    {
      case SELECTED_SPEED_CMD:
        if ( checkMode(IsoAgLib::IdentModeTractor) )
        {
          mi16_selectedSpeedSetPointCmd =   data().getUint16Data(0);
          mi16_selectedSpeedSetPointLimit = data().getUint16Data(2);
          mt_selectedDirectionCmd = IsoAgLib::IsoDirectionFlag_t( (data().getUint8Data(7) & 0x3) );
        }
        break;
    }
    return true;
  }

  /** send a ISO11783 base information PGN.
    * this is only called when sending ident is configured and it has already claimed an address
      @pre  function is only called in implement mode
      @see  BaseCommon_c::timeEvent()
      @see  CanIo_c::operator<<
  */
  bool TracMoveSetPoint_c::timeEventImplMode( )
  {

    ///Timeperiod of 100ms is set in ::config
    if ( getAvailableExecTime() == 0 ) return false;

    #if defined(USE_BASE) || defined(USE_TRACTOR_MOVE)
    TracMove_c& c_tracmove = getTracMoveInstance4Comm();
    #endif

    if ( 1
      #if defined(USE_BASE) || defined(USE_TRACTOR_MOVE)
        && c_tracmove.getSelectedDataSourceISONameConst().isSpecified()
      #endif
       )
    { // as BaseCommon_c timeEvent() checks only for adr claimed state in TractorMode, we have to perform those checks here,
      // as we reach this function mostly for ImplementMode, where getISOName() might report NULL at least during init time
      if ( ( NULL == getISOName() ) || ( ! getIsoMonitorInstance4Comm().existIsoMemberISOName( *getISOName(), true ) ) )
        return false;

      CanIo_c& c_can = getCanInstance4Comm();

      data().setISONameForSA( *getISOName() );
      data().setIdentType(Ident_c::ExtendedIdent);
      data().setIsoPri(3);
      data().setLen(8);

      data().setIsoPgn(SELECTED_SPEED_CMD);
      uint8_t ui8_temp = 0;
      data().setUint16Data(0, mi16_selectedSpeedSetPointCmd);
      data().setUint16Data(2, mi16_selectedSpeedSetPointLimit);
      ui8_temp |= mt_selectedDirectionCmd;
      data().setUint8Data(7, ui8_temp);
      //reserved fields
      data().setUint16Data(4, 0xFF);
      data().setUint8Data( 6, 0xFF);

      // CanIo_c::operator<< retreives the information with the help of CanPkg_c::getData
      // then it sends the data
      c_can << data();
    }

    if ( getAvailableExecTime() == 0 ) return false;

    return true;
  }


///  Used for Debugging Tasks in Scheduler_c
const char*
TracMoveSetPoint_c::getTaskName() const
{   return "TracMoveSetPoint_c"; }

/** dummy implementation
    @todo SOON-824: add answering of requestPGN in case this object is configured for sending of these information
           - verify this also for the other TracFoo classes
  */
bool TracMoveSetPoint_c::processMsgRequestPGN (uint32_t /*aui32_pgn*/, IsoItem_c* /*apc_isoItemSender*/, IsoItem_c* /*apc_isoItemReceiver*/)
{
  return false;
}

} // End Namespace __IsoAglib
