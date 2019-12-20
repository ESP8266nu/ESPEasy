

// ********************************************************************************
// Web Interface login page
// ********************************************************************************
void handle_login(void) {
  checkRAM(F("handle_login"));

  if (!clientIPallowed(void)) { return; }
  TXBuffer.startStream(void);
  sendHeadandTail_stdtemplate(_HEAD);

  String webrequest = WebServer.arg(F("password"));
  TXBuffer += F("<form method='post'>");
  html_table_class_normal(void);
  TXBuffer += F("<TR><TD>Password<TD>");
  TXBuffer += F("<input class='wide' type='password' name='password' value='");
  TXBuffer += webrequest;
  TXBuffer += "'>";
  html_TR_TD(void);
  html_TD(void);
  addSubmitButton(void);
  html_TR_TD(void);
  html_end_table(void);
  html_end_form(void);

  if (webrequest.length(void) != 0)
  {
    char command[80];
    command[0] = 0;
    webrequest.toCharArray(command, 80);

    // compare with stored password and set timer if there's a match
    if ((strcasecmp(command, SecuritySettings.Password) == 0) || (SecuritySettings.Password[0] == 0))
    {
      WebLoggedIn      = true;
      WebLoggedInTimer = 0;
      TXBuffer         = F("<script>window.location = '.'</script>");
    }
    else
    {
      TXBuffer += F("Invalid password!");

      if (Settings.UseRules)
      {
        String event = F("Login#Failed");
        // TD-er: Do not add to the eventQueue, but execute right now.
        rulesProcessing(event);
      }
    }
  }

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream(void);
  printWebString = "";
  printToWeb     = false;
}
