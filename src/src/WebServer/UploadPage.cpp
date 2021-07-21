#include "../WebServer/UploadPage.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/HTML_wrappers.h"

#include "../Globals/Cache.h"
#include "../Helpers/ESPEasy_Storage.h"

#include "../../ESPEasy-Globals.h"


#ifdef WEBSERVER_UPLOAD

// ********************************************************************************
// Web Interface upload page
// ********************************************************************************
uploadResult_e uploadResult = uploadResult_e::UploadStarted;

void handle_upload() {
  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();

  addHtml(F(
            "<form enctype='multipart/form-data' method='post'><p>Upload settings file:<br><input type='file' name='datafile' size='40'></p><div><input class='button link' type='submit' value='Upload'></div><input type='hidden' name='edit' value='1'></form>"));
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
  printWebString = "";
  printToWeb     = false;
}

// ********************************************************************************
// Web Interface upload page
// ********************************************************************************
void handle_upload_post() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_upload_post"));
  #endif

  if (!isLoggedIn()) { return; }

  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();

  switch (uploadResult) {
    case uploadResult_e::Success:
      addHtml(F("Upload OK!<BR>You may need to reboot to apply all settings..."));
      clearAllCaches();
      LoadSettings();
      break;
    case uploadResult_e::InvalidFile:
      addHtml(F("<font color=\"red\">Upload file invalid!</font>"));
      break;
    case uploadResult_e::NoFilename:
      addHtml(F("<font color=\"red\">No filename!</font>"));
      break;
    case uploadResult_e::UploadStarted:
      break;
  }

  addHtml(F("Upload finished"));
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
  printWebString = "";
  printToWeb     = false;
}

#ifdef WEBSERVER_NEW_UI
void handle_upload_json() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_upload_post"));
  #endif
  uint8_t result = static_cast<int>(uploadResult);

  if (!isLoggedIn()) { result = 255; }

  TXBuffer.startJsonStream();
  addHtml('{');
  stream_next_json_object_value(F("status"), String(result));
  addHtml('}');

  TXBuffer.endStream();
}

#endif // WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface upload handler
// ********************************************************************************
fs::File uploadFile;
void handleFileUpload() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handleFileUpload"));
  #endif

  if (!isLoggedIn()) { return; }

  static boolean valid = false;

  HTTPUpload& upload = web_server.upload();

  if (upload.filename.c_str()[0] == 0)
  {
    uploadResult = uploadResult_e::NoFilename;
    return;
  }

  if (upload.status == UPLOAD_FILE_START)
  {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("Upload: START, filename: ");
      log += upload.filename;
      addLog(LOG_LEVEL_INFO, log);
    }
    valid        = false;
    uploadResult = uploadResult_e::UploadStarted;
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    // first data block, if this is the config file, check PID/Version
    if (upload.totalSize == 0)
    {
      if (matchFileType(upload.filename, FileType::CONFIG_DAT))
      {
        struct TempStruct {
          unsigned long PID;
          int           Version;
        } Temp;

        for (unsigned int x = 0; x < sizeof(struct TempStruct); x++)
        {
          uint8_t b = upload.buf[x];
          memcpy((uint8_t *)&Temp + x, &b, 1);
        }

        if ((Temp.Version == VERSION) && (Temp.PID == ESP_PROJECT_PID)) {
          valid = true;
        }
      }
      else
      {
        // other files are always valid...
        valid = true;
      }

      if (valid)
      {
        String filename;
#if defined(ESP32)
        filename += '/';
#endif // if defined(ESP32)
        filename += upload.filename;

        // once we're safe, remove file and create empty one...
        tryDeleteFile(filename);
        uploadFile = tryOpenFile(filename.c_str(), "w");

        // dont count manual uploads: flashCount();
      }
    }

    if (uploadFile) { uploadFile.write(upload.buf, upload.currentSize); }

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("Upload: WRITE, Bytes: ");
      log += upload.currentSize;
      addLog(LOG_LEVEL_INFO, log);
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (uploadFile) { uploadFile.close(); }

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("Upload: END, Size: ");
      log += upload.totalSize;
      addLog(LOG_LEVEL_INFO, log);
    }
  }

  if (valid) {
    uploadResult = uploadResult_e::Success;
  }
  else {
    uploadResult = uploadResult_e::InvalidFile;
  }
}

#endif // ifdef WEBSERVER_UPLOAD
