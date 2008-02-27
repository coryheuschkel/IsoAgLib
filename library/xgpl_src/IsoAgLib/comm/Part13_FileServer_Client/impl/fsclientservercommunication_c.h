#ifndef FSCLIETSERVERCOMMUNICATION_C_H
#define FSCLIETSERVERCOMMUNICATION_C_H

#include <IsoAgLib/comm/Part5_NetworkManagement/impl/identitem_c.h>

#include "../ifsclient_c.h"

#include <IsoAgLib/comm/Scheduler/ischedulertask_c.h>

#include "fsserverinstance_c.h"
#include "fscommand_c.h"

namespace IsoAgLib {
  class iFsClientServerCommunication_c;
}

// Begin Namespace __IsoAgLib
namespace __IsoAgLib {

/**
  * class FsClientServerCommunication_c, managing the communication betweent a fileserver client and
  * a fileserver. The CAN-BUS communication is done by the FsClientServerCommunication_c's FsResponse_c.
  */
class FsClientServerCommunication_c
{
  private:
    /**
      * The whitelist of fileserver-properties that are desired for this client server communication.
      * Only fileservers of that type will be forwarded to the iFsClient (c_fsClient).
      */
    std::vector<iFsWhitelist_c *> v_fsWhitelist;
    /** The fileserver client for the client-server communication */
    IsoAgLib::iFsClient_c &c_fsClient;
    /**
      * The fileserver for the client-server communication. Has to be
      * registered using the void requestFsConnection(FsServerInstance_c &rc_FileServer) function.
      */
    FsServerInstance_c *c_fileServer;
    /**
      * The fileserver client's IdentItem_c.
      */
    IdentItem_c &c_identItem;
    /**
      * Indicates if the FsClientServerCommunication_c object has been notified on fileservers
      * yet. If no notification has been done so far, all existing fileservers have to be reported to
      * the object.
      */
    bool b_notifiedOnFileservers;
    /**
      * The current directory of the used fileserver.
      */
    uint8_t *pui8_currentDirectory;
    /**
      * The pointer to the FsRequest_c object used for the CAN-BUS communication between the fileserver client and fileserver.
      */
    FsCommand_c *pc_commandHandler;
    /**
      * Flag indication if the registration process of a fileserver has been done successfully. This means that the current
      * directory as well as the existing volumes of the fileserver have been requested successfully.
      */
    bool b_finishedRegistering;

  public:

    /**
      *
      */
    FsServerInstance_c *getFileserver() { return c_fileServer; }

    /**
      * Get method to request the current directory of the used fileserver.
      * @return the used fileserver's current directory.
      */
    uint8_t *getCurrentDirectory() { return pui8_currentDirectory; }

    /**
      * Indicates if the requested fileserver connection has been registered with success. The registration process is
      * considered to have been successfull, if the current directory and all volumes of the fileserver have been set.
      * @return true if filerserver registered with success, false else.
      */
    bool finishedRegistering() { return b_finishedRegistering; }

    /**
      * Method used to get the used fileserver's existing volumes.
      * @return std::vector<struct ::fsDirectory *> the volumes of the used fileserver.
      */
    std::vector<struct ::iFsDirectory *> getFsVolumes() { return c_fileServer->getFsVolumes();}

    /** constructor to init client-server communication without fileserver*/
    FsClientServerCommunication_c(IdentItem_c &rc_identItem, IsoAgLib::iFsClient_c &rc_fsClient, std::vector<iFsWhitelist_c *> v_fsWhitelist);

    /** explicit conversion to reference of interface class type*/
    IsoAgLib::iFsClientServerCommunication_c* toInterfacePointer();

    /**
      * returns the client's IdentItem_c as const IdentItem_c
      */
    const IdentItem_c &getClientIdentItem() { return c_identItem; }

    /**
      * returns the iFsClient_c implementation used for this fileserver - client communication.
      */
    const IsoAgLib::iFsClient_c &getFsClient() { return c_fsClient; }

    /**
      * method used to indicate if this FsClientServerCommunication_c object has been notified on fileservers yet.
      * @return true if fileserver has been reported yet, false else.
      */
    bool getHasBeenNotifiedOnFileServers() { return b_notifiedOnFileservers; }

    /**
      * Method called by FsManager_c to notify FsClientServerCommunication_c on new fileserver. Information is passed
      * directly to iFsClient_c implementation.
      * @return true if freported to iFsClient, false else
      */
    bool notifyOnNewFileServer(FsServerInstance_c &rc_fsServerInstance);

    /**
      * Method called by FsManager_c to notify FsClientServerCommunication_c that it's used fileserver
      * has gone ofline. Information is passed to the iFsClient_c implementation as well.
      * @return true if freported to iFsClient, false else
      */
    bool notifyOnFileServerOffline(FsServerInstance_c &rc_fsServerInstance);

    /**
      * Method called to request conncetion to specified fileserver
      * @param rc_FileServer The fileserver for the requested connection.
      */
    void requestFsConnection(FsServerInstance_c &rc_FileServer);

    /// FileServer access functions as defined in iFsClientServerCommunication_c
    iFsCommandErrors getFileServerProperties();

    iFsCommandErrors changeCurrentDirectory(uint8_t *pui8_newDirectory);

    iFsCommandErrors openFile(uint8_t *pui8_fileName, bool b_openExclusive, bool b_openForAppend, bool b_createNewFile, bool b_openForReading, bool b_openForWriting, bool b_openDirectory);
    iFsCommandErrors seekFile(uint8_t ui8_fileHandle, uint8_t ui8_possitionMode, int32_t i32_offset);
    iFsCommandErrors readFile(uint8_t ui8_fileHandle, uint16_t ui16_count, bool b_reportHiddenFiles);
    iFsCommandErrors writeFile(uint8_t ui8_fileHandle, uint16_t ui16_count, uint8_t *pui8_data);
    iFsCommandErrors closeFile(uint8_t ui8_fileHandle);

    iFsCommandErrors moveFile(uint8_t *pui8_sourceName, uint8_t *pui8_destName, bool b_recursive, bool b_force, bool b_copy);
    iFsCommandErrors deleteFile(uint8_t *pui8_fileName, bool b_recursive, bool b_force);
    iFsCommandErrors getFileAttributes(uint8_t *pui8_fileName);
    iFsCommandErrors setFileAttributes(uint8_t *pui8_fileName, uint8_t ui8_hiddenAtt, uint8_t ui8_readOnlyAtt);
    iFsCommandErrors getFileDateTime(uint8_t *pui8_fileName);

    iFsCommandErrors initializeVolume(uint8_t *pui8_pathName, uint32_t ui32_space, bool b_createVolumeUsingSpace, bool b_createNewVolume);
    /// FileServer access functions END

    /// FileServer access response functions as defined in iFsClient_c

    void getFileServerPropertiesResponse(uint8_t ui8_versionNumber, uint8_t ui8_maxNumberOpenFiles, bool b_fsMultiVolumes);

    void getCurrentDirectoryResponse(iFsError ui8_errorCode, uint8_t *piu8_currentDirectory);
    void changeCurrentDirectoryResponse(iFsError ui8_errorCode, uint8_t *piu8_newCurrentDirectory);

    void openFileResponse(iFsError ui8_errorCode,
                          uint8_t ui8_fileHandle,
                          bool b_caseSensitive,
                          bool b_removable,
                          bool b_longFilenames,
                          bool b_directory, 
                          bool b_volume,
                          bool b_hidden,
                          bool b_readOnly)
    { c_fsClient.openFileResponse(ui8_errorCode, ui8_fileHandle, b_caseSensitive, b_removable, b_longFilenames, b_directory,  b_volume, b_hidden, b_readOnly); }
    void seekFileResponse(iFsError ui8_errorCode, uint32_t ui32_position)
    { c_fsClient.seekFileResponse(ui8_errorCode, ui32_position); }
    void readFileResponse(iFsError ui8_errorCode, uint16_t ui16_dataLength, uint8_t *pui8_data)
    { c_fsClient.readFileResponse(ui8_errorCode, ui16_dataLength, pui8_data); }
    void readDirectoryResponse(iFsError ui8_errorCode, std::vector<struct iFsDirectory *> v_directories)
    { c_fsClient.readDirectoryResponse(ui8_errorCode, v_directories); }
    void writeFileResponse(iFsError ui8_errorCode, uint16_t ui16_dataWritten)
    { c_fsClient.writeFileResponse(ui8_errorCode, ui16_dataWritten); }
    void closeFileResponse(iFsError ui8_errorCode)
    { c_fsClient.closeFileResponse(ui8_errorCode); }

    void moveFileResponse(iFsError ui8_errorCode);
    void deleteFileResponse(iFsError ui8_errorCode);
    void getFileAttributesResponse(iFsError ui8_errorCode,
                                   bool b_caseSensitive,
                                   bool b_removable,
                                   bool b_longFilenames,
                                   bool b_directory, 
                                   bool b_volume,
                                   bool b_hidden,
                                   bool b_readOnly);
    void setFileAttributesResponse(iFsError ui8_errorCode);
    void getFileDateTimeResponse(iFsError ui8_errorCode,
                                 uint16_t ui16_fileYear,
                                 uint8_t ui8_fileMonth,
                                 uint8_t ui8_fileDay,
                                 uint8_t ui8_fileHour,
                                 uint8_t ui8_fileMinute,
                                 uint8_t ui8_fileSecond);

    void initializeVolumeResponse(iFsError ui8_errorCode,
                                   bool b_caseSensitive,
                                   bool b_removable,
                                   bool b_longFilenames,
                                   bool b_directory, 
                                   bool b_volume,
                                   bool b_hidden,
                                   bool b_readOnly);
    /// FileServer access response functions END
};

//End namespace
}

#endif
