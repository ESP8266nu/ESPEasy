
#ifdef WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface file list
// ********************************************************************************
void handle_filelist_json(void) {
  checkRAM(F("handle_filelist"));

  if (!clientIPallowed(void)) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startJsonStream(void);

  String fdelete = WebServer.arg(F("delete"));

  if (tryDeleteFile(fdelete)) {
    # if defined(ESP32)

    // flashCount(void);
    # endif // if defined(ESP32)
    # if defined(ESP8266)
    checkRuleSets(void);
    # endif // if defined(ESP8266)
  }

  const int pageSize = 25;
  int startIdx       = 0;

  String fstart = WebServer.arg(F("start"));

  if (fstart.length(void) > 0)
  {
    startIdx = atoi(fstart.c_str(void));
  }
  int endIdx = startIdx + pageSize - 1;

  TXBuffer += "[{";
  bool firstentry = true;
  # if defined(ESP32)
  File root  = SPIFFS.open("/");
  File file  = root.openNextFile(void);
  int  count = -1;

  while (file and count < endIdx)
  {
    if (!file.isDirectory(void)) {
      ++count;

      if (count >= startIdx)
      {
        if (firstentry) {
          firstentry = false;
        } else {
          TXBuffer += ",{";
        }
        stream_next_json_object_value(F("fileName"), String(file.name(void)));
        stream_next_json_object_value(F("index"),    String(startIdx));
        stream_last_json_object_value(F("size"), String(file.size(void)));
      }
    }
    file = root.openNextFile(void);
  }
  # endif // if defined(ESP32)
  # if defined(ESP8266)
  fs::Dir dir = SPIFFS.openDir("");

  int count = -1;

  while (dir.next(void))
  {
    ++count;

    if (count < startIdx)
    {
      continue;
    }

    if (firstentry) {
      firstentry = false;
    } else {
      TXBuffer += ",{";
    }

    stream_next_json_object_value(F("fileName"), String(dir.fileName(void)));

    fs::File f = dir.openFile("r");

    if (f) {
      stream_next_json_object_value(F("size"), String(f.size(void)));
      f.close(void);
    }

    stream_last_json_object_value(F("index"), String(startIdx));

    if (count >= endIdx)
    {
      break;
    }
  }
  if (firstentry) {
    TXBuffer += "}";
  }

  # endif // if defined(ESP8266)
  TXBuffer += "]";
  TXBuffer.endStream(void);
}

#endif // WEBSERVER_NEW_UI

#ifdef WEBSERVER_FILELIST
void handle_filelist(void) {
  checkRAM(F("handle_filelist"));

  if (!clientIPallowed(void)) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream(void);
  sendHeadandTail_stdtemplate(void);

  String fdelete = WebServer.arg(F("delete"));

  if (tryDeleteFile(fdelete))
  {
    checkRuleSets(void);
  }
  #ifdef USES_C016

  if (WebServer.hasArg(F("delcache"))) {
    while (C016_deleteOldestCacheBlock(void)) {
      delay(1);
    }

    while (GarbageCollection(void)) {
      delay(1);
    }
  }
  #endif // ifdef USES_C016
  const int pageSize = 25;
  int startIdx       = 0;
  String fstart      = WebServer.arg(F("start"));

  if (fstart.length(void) > 0)
  {
    startIdx = atoi(fstart.c_str(void));
  }
  int endIdx = startIdx + pageSize - 1;
  html_table_class_multirow(void);
  html_table_header("",        50);
  html_table_header(F("Filename"));
  html_table_header(F("Size"), 80);
  int count = -1;

  bool moreFilesPresent  = false;
  bool cacheFilesPresent = false;

#if defined(ESP8266)

  fs::Dir dir = SPIFFS.openDir("");

  while (dir.next(void) && count < endIdx)
  {
    ++count;

    if (count >= startIdx)
    {
      int filesize = -1;
      fs::File f   = dir.openFile("r");

      if (f) {
        filesize = f.size(void);
      }

      if (!cacheFilesPresent && (getCacheFileCountFromFilename(dir.fileName(void)) != -1))
      {
        cacheFilesPresent = true;
      }
      handle_filelist_add_file(dir.fileName(void), filesize, startIdx);
    }
  }
  moreFilesPresent = dir.next(void);
#endif // if defined(ESP8266)
#if defined(ESP32)
  File root = SPIFFS.open("/");
  File file = root.openNextFile(void);

  while (file && count < endIdx)
  {
    if (!file.isDirectory(void)) {
      ++count;

      if (count >= startIdx)
      {
        if (!cacheFilesPresent && (getCacheFileCountFromFilename(file.name(void)) != -1))
        {
          cacheFilesPresent = true;
        }
        handle_filelist_add_file(file.name(void), file.size(void), startIdx);
      }
    }
    file = root.openNextFile(void);
  }
  moreFilesPresent = file;
#endif // if defined(ESP32)

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
  html_TR_TD(void);

  if ((filename != F(FILE_CONFIG)) && (filename != F(FILE_SECURITY)) && (filename != F(FILE_NOTIFICATION)))
  {
    html_add_button_prefix(void);
    TXBuffer += F("filelist?delete=");
    TXBuffer += filename;

    if (startIdx > 0)
    {
      TXBuffer += F("&start=");
      TXBuffer += startIdx;
    }
    TXBuffer += F("'>Del</a>");
  }

  TXBuffer += F("<TD><a href=\"");
  TXBuffer += filename;
  TXBuffer += "\">";
  TXBuffer += filename;
  TXBuffer += F("</a>");
  html_TD(void);

  if (filesize >= 0) {
    TXBuffer += String(filesize);
  }
}

void handle_filelist_buttons(int start_prev, int start_next, bool cacheFilesPresent) {
  html_end_table(void);
  html_end_form(void);
  html_BR(void);
  addButton(F("/upload"), F("Upload"));

  if (start_prev >= 0)
  {
    html_add_button_prefix(void);
    TXBuffer += F("/filelist?start=");
    TXBuffer += start_prev;
    TXBuffer += F("'>Previous</a>");
  }

  if (start_next >= 0)
  {
    html_add_button_prefix(void);
    TXBuffer += F("/filelist?start=");
    TXBuffer += start_next;
    TXBuffer += F("'>Next</a>");
  }

  if (cacheFilesPresent) {
    html_add_button_prefix(F("red"), true);
    TXBuffer += F("filelist?delcache");
    TXBuffer += F("'>Delete Cache Files</a>");
  }
  TXBuffer += F("<BR><BR>");
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream(void);
}
#endif // ifdef WEBSERVER_FILELIST

// ********************************************************************************
// Web Interface SD card file and directory list
// ********************************************************************************
#ifdef FEATURE_SD
void handle_SDfilelist(void) {
  checkRAM(F("handle_SDfilelist"));

  if (!clientIPallowed(void)) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream(void);
  sendHeadandTail_stdtemplate(void);


  String fdelete       = "";
  String ddelete       = "";
  String change_to_dir = "";
  String current_dir   = "";
  String parent_dir    = "";

  for (uint8_t i = 0; i < WebServer.args(void); i++) {
    if (WebServer.argName(i) == F("delete"))
    {
      fdelete = WebServer.arg(i);
    }

    if (WebServer.argName(i) == F("deletedir"))
    {
      ddelete = WebServer.arg(i);
    }

    if (WebServer.argName(i) == F("chgto"))
    {
      change_to_dir = WebServer.arg(i);
    }
  }

  if (fdelete.length(void) > 0)
  {
    SD.remove((char *)fdelete.c_str(void));
  }

  if (ddelete.length(void) > 0)
  {
    SD.rmdir((char *)ddelete.c_str(void));
  }

  if (change_to_dir.length(void) > 0)
  {
    current_dir = change_to_dir;
  }
  else
  {
    current_dir = "/";
  }

  File root = SD.open(current_dir.c_str(void));
  root.rewindDirectory(void);
  File entry = root.openNextFile(void);
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
  html_BR(void);
  html_table_class_multirow(void);
  html_table_header("", 50);
  html_table_header("Name");
  html_table_header("Size");
  html_TR_TD(void);
  TXBuffer += F("<TD><a href=\"SDfilelist?chgto=");
  TXBuffer += parent_dir;
  TXBuffer += F("\">..");
  TXBuffer += F("</a>");
  html_TD(void);

  while (entry)
  {
    if (entry.isDirectory(void))
    {
      char SDcardChildDir[80];
      html_TR_TD(void);

      // take a look in the directory for entries
      String child_dir = current_dir + entry.name(void);
      child_dir.toCharArray(SDcardChildDir, child_dir.length(void) + 1);
      File child         = SD.open(SDcardChildDir);
      File dir_has_entry = child.openNextFile(void);

      // when the directory is empty, display the button to delete them
      if (!dir_has_entry)
      {
        TXBuffer += F("<a class='button link' onclick=\"return confirm('Delete this directory?')\" href=\"SDfilelist?deletedir=");
        TXBuffer += current_dir;
        TXBuffer += entry.name(void);
        TXBuffer += '/';
        TXBuffer += F("&chgto=");
        TXBuffer += current_dir;
        TXBuffer += F("\">Del</a>");
      }
      TXBuffer += F("<TD><a href=\"SDfilelist?chgto=");
      TXBuffer += current_dir;
      TXBuffer += entry.name(void);
      TXBuffer += '/';
      TXBuffer += "\">";
      TXBuffer += entry.name(void);
      TXBuffer += F("</a>");
      html_TD(void);
      TXBuffer += F("dir");
      dir_has_entry.close(void);
    }
    else
    {
      html_TR_TD(void);

      if ((entry.name(void) != String(F(FILE_CONFIG)).c_str(void)) && (entry.name(void) != String(F(FILE_SECURITY)).c_str(void)))
      {
        TXBuffer += F("<a class='button link' onclick=\"return confirm('Delete this file?')\" href=\"SDfilelist?delete=");
        TXBuffer += current_dir;
        TXBuffer += entry.name(void);
        TXBuffer += F("&chgto=");
        TXBuffer += current_dir;
        TXBuffer += F("\">Del</a>");
      }
      TXBuffer += F("<TD><a href=\"");
      TXBuffer += current_dir;
      TXBuffer += entry.name(void);
      TXBuffer += "\">";
      TXBuffer += entry.name(void);
      TXBuffer += F("</a>");
      html_TD(void);
      TXBuffer += entry.size(void);
    }
    entry.close(void);
    entry = root.openNextFile(void);
  }
  root.close(void);
  html_end_table(void);
  html_end_form(void);

  // TXBuffer += F("<BR><a class='button link' href=\"/upload\">Upload</a>");
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream(void);
}

#endif // ifdef FEATURE_SD

