#include "component_tecu.h"

#include <IsoAgLib/comm/Part7_ApplicationLayer/itracgeneral_c.h>
#include <IsoAgLib/comm/Part7_ApplicationLayer/itracmove_c.h>
#include <IsoAgLib/comm/Part7_ApplicationLayer/itracpto_c.h>

IsoAgLibTutorialDisplay::TutorialDisplayTecu_c::TutorialDisplayTecu_c() {}

IsoAgLibTutorialDisplay::TutorialDisplayTecu_c::~TutorialDisplayTecu_c() {}

void IsoAgLibTutorialDisplay::TutorialDisplayTecu_c::init( IsoAgLib::iIdentItem_c* ap_ident ) {
  IsoAgLib::getITracGeneralInstance().config( &( ap_ident->isoName() ), IsoAgLib::IdentModeImplement );
  IsoAgLib::getITracMoveInstance().config( &( ap_ident->isoName() ), IsoAgLib::IdentModeImplement );
  IsoAgLib::getITracPtoInstance().config( &( ap_ident->isoName() ), IsoAgLib::IdentModeImplement );
}

void IsoAgLibTutorialDisplay::TutorialDisplayTecu_c::registerDisplay( iDisplay_c* ap_display ) {
  mvec_display.push_back( ap_display );
}

void IsoAgLibTutorialDisplay::TutorialDisplayTecu_c::unRegisterDisplay( iDisplay_c* ap_display ) {
  for ( std::vector<iDisplay_c*>::iterator it = mvec_display.begin(); it != mvec_display.end(); ++it ) {
    if (( *it ) == ap_display ) {
      mvec_display.erase( it );
      return;
    }
  }
}

bool IsoAgLibTutorialDisplay::TutorialDisplayTecu_c::timeEvent( void ) {
  checkGeneralUpdate( false );
  checkMoveUpdate( false );
  checkPtoUpdate( false );
  return true;
}

void IsoAgLibTutorialDisplay::TutorialDisplayTecu_c::close( void ) { }

const char* IsoAgLibTutorialDisplay::TutorialDisplayTecu_c::getTaskName() const {
  return const_cast<const char*>( "EcuDisplayTecu" );
}

void IsoAgLibTutorialDisplay::TutorialDisplayTecu_c::checkMoveUpdate( bool ab_force ) {
  MoveCtn_s c;
  c.speedReal = IsoAgLib::getITracMoveInstance().speedReal();
  c.speedTheor = IsoAgLib::getITracMoveInstance().speedTheor();
  c.distReal = IsoAgLib::getITracMoveInstance().distReal();
  c.distTheor = IsoAgLib::getITracMoveInstance().distTheor();

  if (( c != ms_lastMoveData ) || ab_force ) {
    ms_lastMoveData = c;
    sendVtMoveValues( c );
  }
}

void IsoAgLibTutorialDisplay::TutorialDisplayTecu_c::sendVtMoveValues( const MoveCtn_s& c ) {
  if ( ! mvec_display.empty() ) {
    for ( std::vector<iDisplay_c*>::iterator it = mvec_display.begin(); it != mvec_display.end(); ++it ) {
      ( *it )->handleNewMoveValues( c.speedReal, c.speedTheor, c.distReal, c.distTheor );
    }
  }
}

void IsoAgLibTutorialDisplay::TutorialDisplayTecu_c::checkPtoUpdate( bool ab_force ) {
  PtoCtn_s c;
  c.ptoFront = IsoAgLib::getITracPtoInstance().ptoFront();
  c.ptoFrontEngaged = ( IsoAgLib::getITracPtoInstance().ptoFrontEngaged() == IsoAgLib::IsoActive );
  c.ptoRear = IsoAgLib::getITracPtoInstance().ptoRear();
  c.ptoRearEngaged = ( IsoAgLib::getITracPtoInstance().ptoRearEngaged() == IsoAgLib::IsoActive );

  if (( c != ms_lastPtoData ) || ab_force ) {
    ms_lastPtoData = c;
    sendVtPtoValues( c );
  }
}

void IsoAgLibTutorialDisplay::TutorialDisplayTecu_c::sendVtPtoValues( const PtoCtn_s& c ) {
  if ( ! mvec_display.empty() ) {
    for ( std::vector<iDisplay_c*>::iterator it = mvec_display.begin(); it != mvec_display.end(); ++it ) {
      ( *it )->handleNewPto( c.ptoFront, c.ptoFrontEngaged, c.ptoRear, c.ptoRearEngaged );
    }
  }
}

void IsoAgLibTutorialDisplay::TutorialDisplayTecu_c::checkGeneralUpdate( bool ab_force ) {
  GeneralCtn_s c;
  c.keySwitch = ( IsoAgLib::getITracGeneralInstance().keySwitch() == IsoAgLib::IsoActive );
  c.maxPowerTime = IsoAgLib::getITracGeneralInstance().maxPowerTime();

  if (( c != ms_lastGeneralData ) || ab_force ) {
    ms_lastGeneralData = c;
    sendVtGeneralValues( c );
  }
}

void IsoAgLibTutorialDisplay::TutorialDisplayTecu_c::sendVtGeneralValues( const GeneralCtn_s& c ) {
  if ( ! mvec_display.empty() ) {
    for ( std::vector<iDisplay_c*>::iterator it = mvec_display.begin(); it != mvec_display.end(); ++it ) {
      ( *it )->handleNewGeneralValues( c.keySwitch, c.maxPowerTime );
    }
  }
}
