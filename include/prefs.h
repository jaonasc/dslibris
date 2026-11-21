#ifndef _PREFS_H
#define _PREFS_H
#include "expat.h"
#include "app.h"

class Prefs
{
public:
	App *app;
	long modtime;
	Prefs();
	Prefs(App *parent);
	~Prefs();
	int Read(bool booksOnly=false);
	int Write();
	bool swapshoulder;
private:
	bool recoveredBackup;
	void Init();
};

#endif
