/*
  process_c.h: central managing instance for all process data
    informations in the system

  (C) Copyright 2009 - 2010 by OSB AG and developing partners

  See the repository-log for details on the authors and file-history.
  (Repository information can be found at <http://isoaglib.com/download>)

  Use, modification and distribution are subject to the GNU General
  Public License with exceptions for ISOAgLib. (See accompanying
  file LICENSE.txt or copy at <http://isoaglib.com/download/license>)
*/
#ifndef PROCESS_H
#define PROCESS_H

/* *************************************** */
/* ********** include headers ************ */
/* *************************************** */
#include <functional>

#include <IsoAgLib/util/impl/singleton.h>
#include <IsoAgLib/util/impl/container.h>
#include <IsoAgLib/hal/hal_typedef.h>
#include <IsoAgLib/util/config.h>
#include <IsoAgLib/driver/can/impl/cancustomer_c.h>
#include <IsoAgLib/scheduler/impl/schedulertask_c.h>
#include <IsoAgLib/comm/Part10_TaskController_Client/impl/procdatalocalbase_c.h>
#include "processpkg_c.h"
#include <IsoAgLib/comm/Part10_TaskController_Client/processdatachangehandler_c.h>
#include <IsoAgLib/comm/Part10_TaskController_Client/impl/processwsmtaskmsghandler_c.h>
#if defined(USE_ISO_TASKCONTROLLER_CLIENT)
  #include <IsoAgLib/comm/Part10_TaskController_Client/impl/devpropertyhandler_c.h>
#endif

#include <list>

namespace IsoAgLib { class iProcess_c;class iDevPropertyHandler_c;}


#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4355 )
#endif


// Begin Namespace IsoAgLib
namespace __IsoAgLib {

class Process_c : public Scheduler_Task_c
{
#if 1 < PRT_INSTANCE_CNT
  MACRO_MULTITON_CONTRIBUTION();
#else
  MACRO_SINGLETON_CONTRIBUTION();
#endif
public:
  /** initialisation for Process_c
  */
  void init( void );
  /** every subsystem of IsoAgLib has explicit function for controlled shutdown
    */
  void close( void );

  virtual ~Process_c() {}

  /**
    start processing of a process msg
    ignore all invalid messages where sender has no address (0xFE)
    @return true -> message was processed; else the received CAN message will be served to other matching CanCustomer_c
  */
  bool processMsg( const CanPkg_c& arc_data );

#if defined(USE_ISO_TASKCONTROLLER_CLIENT)
  DevPropertyHandler_c& getDevPropertyHandlerInstance( void );
#endif

  /**
    checks if a suitable ProcDataLocalBase_c item exist
    ISO parameter
    @param aui16_DDI
    @param aui16_element
    @param acrc_isoNameReceiver isoName code of searched local Process Data instance
    @return true -> suitable instance found
  */
  bool existProcDataLocal( uint16_t aui16_DDI, uint16_t aui16_element, const IsoName_c& acrc_isoNameReceiver);

  /**
    checks if a suitable ProcDataRemoteBase_c item exist
    ISO parameter
    @param aui16_DDI
    @param aui16_element
    @param acrc_isoNameSender isoName of the sender (used for check against ownerISOName())
    @param acrc_isoNameReceiver isoName code of searched local Process Data instance
    @return true -> suitable instance found
  */
  bool existProcDataRemote( uint16_t aui16_DDI, uint16_t aui16_element,
                            const IsoName_c& acrc_isoNameSender, const IsoName_c& acrc_isoNameReceiver);

  /**
    search for suitable ProcDataLocalBase_c item; create on if not found AND if wanted

    possible errors:
        * Err_c::badAlloc not enough memory to add new ProcDataLocal_c
          (can cause badAlloc exception)
        * Err_c::elNonexistent if element not found and ab_doCreate == false
    ISO parameter
    @param aui16_DDI
    @param aui16_element
    @param acrc_isoNameReceiver isoName code of searched local Process Data instance
    @return reference to searched/created ProcDataLocalBase_c instance
  */
  ProcDataLocalBase_c& procDataLocal( uint16_t aui16_DDI, uint16_t aui16_element, const IsoName_c& acrc_isoNameReceiver);

  /**
    performs periodically actions

    @return true -> all planned activities performed in allowed time
  */
  bool timeEvent( void );

  /** called when a new measurement is started */
  void resetTimerPeriod( void );

  /** handler function for access to undefined client.
    * the base Singleton calls this function, if it detects an error
    */
  void registerAccessFlt( void );

  /** register pointer to a new local process data instance
    * this function is called within construction of new local process data instance
    */
  bool registerLocalProcessData( ProcDataLocalBase_c* pc_localClient)
  { return registerC1( pc_localClient );}

  /** unregister pointer to a already registered local process data instance
    * this function is called within destruction of local process data instance
    */
  void unregisterLocalProcessData( ProcDataLocalBase_c* pc_localClient)
  { unregisterC1( pc_localClient );}

  void registerWsmTaskMsgHandler(ProcessWsmTaskMsgHandler_c* apc_processWsmTaskMsgHandler)
  { mpc_processWsmTaskMsgHandler = apc_processWsmTaskMsgHandler; }

  /**
    delete FilterBox_c for receive from remote isoName if needed
    (important to delete old Filter Boxes after deletion of
    of remote device from monitor list or after re-adressclaim with different SA)
    @param acrc_isoName ISOName code of remote owner who sent the message
    @return true -> member exist and Filter Box deleted
  */
  bool deleteRemoteFilter(const IsoName_c& acrc_isoName);

  /** this function is called by IsoMonitor_c on addition, state-change and removal of an IsoItem.
   * @param at_action enumeration indicating what happened to this IsoItem. @see IsoItemModification_en / IsoItemModification_t
   * @param acrc_isoItem reference to the (const) IsoItem which is changed (by existance or state)
   */
  void reactOnIsoItemModification (ControlFunctionStateHandler_c::IsoItemModification_t /*at_action*/, IsoItem_c const& /*acrc_isoItem*/);

  /**
    process TC status messages:
    - task status suspended: stop running measurement (started by default data logging)
    @param ui8_tcStatus
    @param rc_isoName         device key of TC
    @param ab_skipLastTcStatus TRUE => don't check for changed TC status
    @return TRUE
  */
  bool processTcStatusMsg(uint8_t ui8_tcStatus, const IsoName_c& rc_isoName, bool ab_skipLastTcStatus = FALSE);

  /**
    @return isoName, saved from TC status messages
  */
  const IsoName_c* getTcISOName() { return mpc_tcISOName; };

  /**
    process working set task messages
    @param ui8_tcStatus
    @param rc_isoName         device key of working set
    @return TRUE
  */
  bool processWorkingSetTaskMsg(uint8_t /* ui8_tcStatus */, const IsoName_c& /* rc_isoName */);

#if DEBUG_SCHEDULER
  virtual const char* getTaskName() const;
#endif

  /** set the pointer to the handler class (used for callback when TC status message is processed)
    * @param apc_processDataChangeHandler pointer to handler class of application
    */
  void setProcessDataChangeHandler( IsoAgLib::ProcessDataChangeHandler_c *apc_processDataChangeHandler )
   { mpc_processDataChangeHandler = apc_processDataChangeHandler; }

protected:
  //! Function set ui16_earlierInterval and
  //! ui16_laterInterval that will be used by
  //! getTimeToNextTrigger(retriggerType_t)
  //! can be overloaded by Childclass for special condition
  virtual void updateEarlierAndLatestInterval();

private: // Private methods
  /**
    update the cache with search for according ProcDataLocalBase_c item
    ISO parameter
    @param aui16_DDI
    @param aui16_element
    @param acrc_isoNameReceiver isoName code of created local Process Data instance
  */
  bool updateLocalCache( uint16_t aui16_DDI, uint16_t aui16_element, const IsoName_c& acrc_isoNameReceiver);

  virtual bool reactOnStreamStart(
      ReceiveStreamIdentifier_c const &ac_ident,
      uint32_t aui32_totalLen)
  {
    return mt_customer.reactOnStreamStartDefault(ac_ident, aui32_totalLen);
  }

  virtual void reactOnAbort(Stream_c &arc_stream)
  {
    mt_customer.reactOnAbortDefault(arc_stream);
  }

  virtual bool processPartStreamDataChunk(
      Stream_c &apc_stream,
      bool ab_isFirstChunk,
      bool ab_isLastChunk)
  {
    return mt_customer.processPartStreamDataChunkDefault(
        apc_stream,
        ab_isFirstChunk,
        ab_isLastChunk);
  }

  virtual void notificationOnMultiReceiveError(
      ReceiveStreamIdentifier_c const &ac_streamIdent,
      uint8_t aui8_multiReceiveError,
      bool ab_isGlobal)
  {
    mt_customer.notificationOnMultiReceiveErrorDefault(
        ac_streamIdent,
        aui8_multiReceiveError,
        ab_isGlobal);
  }

  virtual uint16_t getForcedMinExecTime() const
  {
    return getForcedMinExecTimeDefault();
  }

private: // Private attributes

  class CanCustomerProxy_c : public CanCustomer_c {
  public:
    typedef Process_c Owner_t;

    CanCustomerProxy_c(Owner_t &art_owner) : mrt_owner(art_owner) {}

    virtual ~CanCustomerProxy_c() {}

  private:
    virtual bool processMsg( const CanPkg_c& arc_data ) {
      return mrt_owner.processMsg( arc_data );
    }

    virtual bool reactOnStreamStart(
        ReceiveStreamIdentifier_c const &ac_ident,
        uint32_t aui32_totalLen)
    {
      return mrt_owner.reactOnStreamStart(ac_ident, aui32_totalLen);
    }

    virtual void reactOnAbort(Stream_c &arc_stream)
    {
      mrt_owner.reactOnAbort(arc_stream);
    }

    virtual bool processPartStreamDataChunk(
        Stream_c &apc_stream,
        bool ab_isFirstChunk,
        bool ab_isLastChunk)
    {
      return mrt_owner.processPartStreamDataChunk(
          apc_stream,
          ab_isFirstChunk,
          ab_isLastChunk);
    }

    virtual void notificationOnMultiReceiveError(
        ReceiveStreamIdentifier_c const &ac_streamIdent,
        uint8_t aui8_multiReceiveError,
        bool ab_isGlobal)
    {
      mrt_owner.notificationOnMultiReceiveError(
          ac_streamIdent,
          aui8_multiReceiveError,
          ab_isGlobal);
    }

    // CanCustomerProxy_c shall not be copyable. Otherwise the
    // reference to the containing object would become invalid.
    CanCustomerProxy_c(CanCustomerProxy_c const &);

    CanCustomerProxy_c &operator=(CanCustomerProxy_c const &);

    Owner_t &mrt_owner;
  };
  typedef CanCustomerProxy_c Customer_t;
  class ControlFunctionStateHandlerProxy_c : public ControlFunctionStateHandler_c {
  public:
    typedef Process_c Owner_t;

    ControlFunctionStateHandlerProxy_c(Owner_t &art_owner) : mrt_owner(art_owner) {}

    virtual ~ControlFunctionStateHandlerProxy_c() {}

  private:
    virtual void reactOnIsoItemModification(
        IsoItemModification_t at_action,
        IsoItem_c const &acrc_isoItem)
    {
      mrt_owner.reactOnIsoItemModification(at_action, acrc_isoItem);
    }

    // ControlFunctionStateHandlerProxy_c shall not be copyable. Otherwise the
    // reference to the containing object would become invalid.
    ControlFunctionStateHandlerProxy_c(ControlFunctionStateHandlerProxy_c const &);

    ControlFunctionStateHandlerProxy_c &operator=(ControlFunctionStateHandlerProxy_c const &);

    Owner_t &mrt_owner;
  };
  typedef ControlFunctionStateHandlerProxy_c Handler_t;

#ifdef __IAR_SYSTEMS_ICC__
  friend class IarSingletonDerived<Process_c,Scheduler_Task_c>;
#else
#endif
  friend class IsoAgLib::iProcess_c;
  friend class IsoAgLib::iDevPropertyHandler_c;
  /**
    HIDDEN constructor for a Process_c object instance
    NEVER instantiate a variable of type Process_c within application
    only access Process_c via getProcessInstance() or getProcessInstance( int riLbsBusNr ) in case more than one ISO11783 BUS is used for IsoAgLib
    */
  Process_c() :
    mt_handler(*this),
    mt_customer(*this),
    CONTAINER_CLIENT1_CTOR_INITIALIZER_LIST
  {}

#if defined(USE_ISO_TASKCONTROLLER_CLIENT)
  /**
    deliver reference to process pkg as reference to DevPropertyHandler_c which
    handles sending and processing of messages from can
  */
  DevPropertyHandler_c mc_devPropertyHandler;
#endif

  /** last timestamp with FilterBox_c check */
  int32_t mi32_lastFilterBoxTime;

  STL_NAMESPACE::list<IsoName_c> ml_filtersToDeleteISO;
  const IsoName_c* mpc_tcISOName;
  uint8_t mui8_lastTcStatus;

  ProcessWsmTaskMsgHandler_c* mpc_processWsmTaskMsgHandler;

  /** pointer to applications handler class, with handler functions
      which shall be called when a TC status message arrives
  */
  IsoAgLib::ProcessDataChangeHandler_c* mpc_processDataChangeHandler;
  Handler_t mt_handler;
  Customer_t mt_customer;
  CONTAINER_CLIENT1_MEMBER_FUNCTIONS_MAIN(ProcDataLocalBase_c);
  friend Process_c &getProcessInstance( uint8_t aui8_instance );
};


  /** C-style function, to get access to the unique Process_c singleton instance
    * if more than one CAN BUS is used for IsoAgLib, an index must be given to select the wanted BUS
    */
  Process_c &getProcessInstance( uint8_t aui8_instance = 0 );
}

#if defined(_MSC_VER)
#pragma warning( pop )
#endif

#endif