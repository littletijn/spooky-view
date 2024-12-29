#ifndef CMODIFICATIONSETTINGS_H
#define CMODIFICATIONSETTINGS_H

struct CModificationSettings
{
	BOOL alwaysOnTop = 0;
	//Transparency
	BOOL enabled = 0;
	BOOL separateBackgroundValue = 0;
	BYTE foreground = 255;
	BYTE background = 255;
	//Maximized transparency
	BOOL maxEnabled = 0;
	BOOL maxSeparateBackgroundValue = 0;
	BYTE maxForeground = 255;
	BYTE maxBackground = 255;
};

#endif