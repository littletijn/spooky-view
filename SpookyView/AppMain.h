#ifndef APPMAIN_H
#define APPMAIN_H

class AppMain
{
public:
	int Run();
protected:
	void LoadFunctionAdresses();
	void SendAlreadyRunningNotify();
	void GetIsWindows8();
};

#endif