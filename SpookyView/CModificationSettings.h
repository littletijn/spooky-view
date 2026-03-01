#ifndef CMODIFICATIONSETTINGS_H
#define CMODIFICATIONSETTINGS_H

struct CModificationSettings
{
	BOOL isGlobal = 0;
	BOOL enabled = 0;
	BOOL alwaysOnTop = 0;
	BOOL separateBackgroundValue = 0;
	BYTE foreground = 255;
	BYTE background = 255;
};

#endif