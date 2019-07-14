#ifndef COMMAND_RTC_H
#define COMMAND_RTC_H


String Command_RTC_Clear(struct EventStruct *event, const char* Line)
{
	initRTC();
	return return_command_success();
}

String Command_RTC_resetFlashWriteCounter(struct EventStruct *event, const char* Line)
{
	RTC.flashDayCounter = 0;
	return return_command_success();
}

String Command_saveUserVarToRTC(struct EventStruct *event, const char* Line)
{
	saveUserVarToRTC();
	return return_command_success();
}

String Command_readUserVarFromRTC(struct EventStruct *event, const char* Line)
{
	readUserVarFromRTC();
	return return_command_success();
}

#endif // COMMAND_RTC_H
