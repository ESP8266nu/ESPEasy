#include "../WebServer/FileList.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"

#include "../ESPEasyCore/ESPEasyRules.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Numerical.h"



#ifdef USES_C016
#include "../Globals/C016_ControllerCache.h"
#endif


#ifdef WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface file list
// ********************************************************************************
void handle_filelist_json() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_filelist"));
  #endif

  if (!clientIPallowed()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startJsonStream();

  String fdelete = webArg(F("delete"));

  if (tryDeleteFile(fdelete)) {
    # if defined(ESP32)

    // flashCount();
    # endif // if defined(ESP32)
    # if defined(ESP8266)
    checkRuleSets();
    # endif // if defined(ESP8266)
  }

  const int pageSize = 25;
  int startIdx       = 0;

  String fstart = webArg(F("start"));

  if (fstart.length() > 0)
  {
    validIntFromString(fstart, startIdx);
  }
  int endIdx = startIdx + pageSize - 1;

  addHtml(F("[{"));
  bool firstentry = true;
  # if defined(ESP32)
  File root  = ESPEASY_FS.open("/");
  File file  = root.openNextFile();
  int  count = -1;

  while (file and count < endIdx)
  {
    if (!file.isDirectory()) {
      ++count;

      if (count >= startIdx)
      {
        if (firstentry) {
          firstentry = false;
        } else {
          addHtml(F(",{"));
        }
        stream_next_json_object_value(F("fileName"), String(file.name()));
        stream_next_json_object_value(F("index"),    String(startIdx));
        stream_last_json_object_value(F("size"), String(file.size()));
      }
    }
    file = root.openNextFile();
  }
  # endif // if defined(ESP32)
  # if defined(ESP8266)
  fs::Dir dir = ESPEASY_FS.openDir("");

  int count = -1;

  while (dir.next())
  {
    ++count;

    if (count < startIdx)
    {
      continue;
    }

    if (firstentry) {
      firstentry = false;
    } else {
      addHtml(F(",{"));
    }

    stream_next_json_object_value(F("fileName"), String(dir.fileName()));

    fs::File f = dir.openFile("r");

    if (f) {
      stream_next_json_object_value(F("size"), String(f.size()));
      f.close();
    }

    stream_last_json_object_value(F("index"), String(startIdx));

    if (count >= endIdx)
    {
      break;
    }
  }

  if (firstentry) {
    addHtml('}');
  }

  # endif // if defined(ESP8266)
  addHtml(']');
  TXBuffer.endStream();
}

#endif // WEBSERVER_NEW_UI

#ifdef WEBSERVER_FILELIST
void handle_filelist() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_filelist"));
  #endif

  if (!clientIPallowed()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();

  String fdelete = webArg(F("delete"));

  if (tryDeleteFile(fdelete))
  {
    checkRuleSets();
  }
  # ifdef USES_C016

  if (web_server.hasArg(F("delcache"))) {
    while (C016_deleteOldestCacheBlock()) {
      delay(1);
    }

    while (GarbageCollection()) {
      delay(1);
    }
  }
  # endif // ifdef USES_C016
  const int pageSize = 25;
  int startIdx       = 0;
  String fstart      = webArg(F("start"));

  if (fstart.length() > 0)
  {
    validIntFromString(fstart, startIdx);
  }
  int endIdx = startIdx + pageSize - 1;
  html_table_class_multirow();
  html_table_header(F(""),        50);
  html_table_header(F("Filename"));
  html_table_header(F("Size"), 80);
  int count = -1;

  bool moreFilesPresent  = false;
  bool cacheFilesPresent = false;

# if defined(ESP8266)

  fs::Dir dir = ESPEASY_FS.openDir("");

  while (dir.next() && count < endIdx)
  {
    ++count;

    if (count >= startIdx)
    {
      int filesize = -1;
      fs::File f   = dir.openFile("r");

      if (f) {
        filesize = f.size();
      }

      if (!cacheFilesPresent && (getCacheFileCountFromFilename(dir.fileName()) != -1))
      {
        cacheFilesPresent = true;
      }
      handle_filelist_add_file(dir.fileName(), filesize, startIdx);
    }
  }
  moreFilesPresent = dir.next();
# endif // if defined(ESP8266)
# if defined(ESP32)
  File root = ESPEASY_FS.open("/");
  File file = root.openNextFile();

  while (file && count < endIdx)
  {
    if (!file.isDirectory()) {
      ++count;

      if (count >= startIdx)
      {
        if (!cacheFilesPresent && (getCacheFileCountFromFilename(file.name()) != -1))
        {
          cacheFilesPresent = true;
        }
        handle_filelist_add_file(file.name(), file.size(), startIdx);
      }
    }
    file = root.openNextFile();
  }
  moreFilesPresent = file;
# endif // if defined(ESP32)

  int start_prev = -1;

  if (startIdx > 0)
  {
    start_prev = startIdx < pageSize ? 0 : startIdx - pageSize;
  }
  int start_next = -1;

  if ((count >= endIdx) && moreFilesPresent) {
    start_next = endIdx + 1;
  }
  handle_filelist_buttons(start_prev, start_next, cacheFilesPresent);
}

void handle_filelist_add_file(const String& filename, int filesize, int startIdx) {
  html_TR_TD();

  if (!isProtectedFileType(filename))
  {
    html_add_button_prefix();
    addHtml(F("filelist?delete="));
    addHtml(filename);

    if (startIdx > 0)
    {
      addHtml(F("&start="));
      addHtmlInt(startIdx);
    }
    addHtml(F("'>Del</a>"));
  }
  {
    String html;
    html.reserve(30 + 2 * filename.length());

    html += F("<TD><a href=\"");
    html += filename;
    html += "\">";
    html += filename;
    html += F("</a><TD>");

    if (filesize >= 0) {
      html += filesize;
    }
    addHtml(html);
  }
}

void handle_filelist_buttons(int start_prev, int start_next, bool cacheFilesPresent) {
  html_end_table();
  html_end_form();
  html_BR();
  addButton(F("/upload"), F("Upload"));

  if (start_prev >= 0)
  {
    html_add_button_prefix();
    String html;
    html.reserve(36);
    html += F("/filelist?start=");
    html += start_prev;
    html += F("'>Previous</a>");
    addHtml(html);
  }

  if (start_next >= 0)
  {
    html_add_button_prefix();
    String html;
    html.reserve(36);

    html += F("/filelist?start=");
    html += start_next;
    html += F("'>Next</a>");
    addHtml(html);
  }

  if (cacheFilesPresent) {
    html_add_button_prefix(F("red"), true);
    addHtml(F("filelist?delcache'>Delete Cache Files</a>"));
  }
  addHtml(F("<BR><BR>"));
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
}

#endif // ifdef WEBSERVER_FILELIST

// ********************************************************************************
// Web Interface SD card file and directory list
// ********************************************************************************
#ifdef FEATURE_SD
void handle_SDfilelist() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_SDfilelist"));
  #endif

  if (!clientIPallowed()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();


  String fdelete;
  String ddelete;
  String change_to_dir;
  String current_dir;
  String parent_dir;

  for (uint8_t i = 0; i < web_server.args(); i++) {
    if (web_server.argName(i) == F("delete"))
    {
      fdelete = webArg(i);
    }

    if (web_server.argName(i) == F("deletedir"))
    {
      ddelete = webArg(i);
    }

    if (web_server.argName(i) == F("chgto"))
    {
      change_to_dir = webArg(i);
    }
  }

  if (fdelete.length() > 0)
  {
    SD.remove((char *)fdelete.c_str());
  }

  if (ddelete.length() > 0)
  {
    SD.rmdir((char *)ddelete.c_str());
  }

  if (change_to_dir.length() > 0)
  {
    current_dir = change_to_dir;
  }
  else
  {
    current_dir = "/";
  }

  File root = SD.open(current_dir.c_str());
  root.rewindDirectory();
  File entry = root.openNextFile();
  parent_dir = current_dir;

  if (!current_dir.equals("/"))
  {
    /* calculate the position to remove
       /
       / current_dir = /dir1/dir2/   =>   parent_dir = /dir1/
       /                     ^ position to remove, second last index of "/" + 1
       /
       / current_dir = /dir1/   =>   parent_dir = /
       /                ^ position to remove, second last index of "/" + 1
     */
    parent_dir.remove(parent_dir.lastIndexOf("/", parent_dir.lastIndexOf("/") - 1) + 1);
  }


  String subheader = "SD Card: " + current_dir;
  addFormSubHeader(subheader);
  html_BR();
  html_table_class_multirow();
  html_table_header(F(""), 50);
  html_table_header(F("Name"));
  html_table_header(F("Size"));
  html_TR_TD();
  {
    String html;
    html.reserve(50 + parent_dir.length());
    html += F("<TD><a href=\"SDfilelist?chgto=");
    html += parent_dir;
    html += F("\">..");
    html += F("</a><TD>");
    addHtml(html);
  }

  while (entry)
  {
    html_TR_TD();
    size_t entrynameLength = strlen(entry.name());
    if (entry.isDirectory())
    {
      char SDcardChildDir[80];

      // take a look in the directory for entries
      String child_dir = current_dir + entry.name();
      child_dir.toCharArray(SDcardChildDir, child_dir.length() + 1);
      File child         = SD.open(SDcardChildDir);
      File dir_has_entry = child.openNextFile();

      // when the directory is empty, display the button to delete them
      if (!dir_has_entry)
      {
        addHtml(F("<a class='button link' onclick=\"return confirm('Delete this directory?')\" href=\"SDfilelist?deletedir="));
        String html;
        html.reserve(20 + 2 * current_dir.length() + entrynameLength);
        html += current_dir;
        html += entry.name();
        html += '/';
        html += F("&chgto=");
        html += current_dir;
        html += F("\">Del</a>");
        addHtml(html);
      }
      {
        String html;
        html.reserve(48 + current_dir.length() + 2 * entrynameLength);

        html += F("<TD><a href=\"SDfilelist?chgto=");
        html += current_dir;
        html += entry.name();
        html += '/';
        html += "\">";
        html += entry.name();
        html += F("</a><TD>");
        html += F("dir");
        addHtml(html);
      }
      dir_has_entry.close();
    }
    else
    {

      if (isProtectedFileType(String(entry.name())))
      {
        addHtml(F("<a class='button link' onclick=\"return confirm('Delete this file?')\" href=\"SDfilelist?delete="));
        String html;
        html.reserve(20 + 2 * current_dir.length() + entrynameLength);

        html += current_dir;
        html += entry.name();
        html += F("&chgto=");
        html += current_dir;
        html += F("\">Del</a>");
        addHtml(html);
      }
      {
        String html;
        html.reserve(48 + current_dir.length() + 2 * entrynameLength);
        html += F("<TD><a href=\"");
        html += current_dir;
        html += entry.name();
        html += "\">";
        html += entry.name();
        html += F("</a><TD>");
        html += entry.size();
        addHtml(html);
      }
    }
    entry.close();
    entry = root.openNextFile();
  }
  root.close();
  html_end_table();
  html_end_form();

  // addHtml(F("<BR><a class='button link' href=\"/upload\">Upload</a>"));
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
}

#endif // ifdef FEATURE_SD
