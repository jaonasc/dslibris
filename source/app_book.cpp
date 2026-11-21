#include <errno.h>
#include <stdlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/dir.h>
#include <sys/stat.h>

#include <expat.h>

#include <fat.h>
#include <nds/bios.h>

#include "types.h"
#include "main.h"
#include "parse.h"
#include "app.h"
#include "book.h"
#include "button.h"
#include "text.h"
#include "reader_visual.h"
#include <new>

//! Book-related methods for App class.

void App::HandleEventInBook()
{
	u16 pagecurrent = bookcurrent->GetPosition();
	u16 pagecount = bookcurrent->GetPageCount();

	if (key.downrepeat & (KEY_A|key.r|key.down))
	{
		// page forward.
		if (pagecurrent < pagecount-1)
		{
			TurnPage(1);
		}
		prefs->Write();
	}

	else if (key.downrepeat & (KEY_B|key.l|key.up))
	{
		// page back.
		if(pagecurrent > 0)
		{
			TurnPage(-1);
		}
		prefs->Write();
	}

	uint32 keys = keysDown();

	if(keys){

		if (keys & KEY_X)
		{
			// toggle inverted text.
			readingTheme=(readingTheme+1)%ReaderVisual::THEME_COUNT;
            ApplyTheme();
            prefs->Write();
			bookcurrent->GetPage()->Draw(ts);
		}

		else if (keys & KEY_Y)
		{
			// go to next level in brightness. 
			CycleBrightness();
			prefs->Write();
		}

		else if (keys & KEY_START)
		{
			// return to browser.
			bookcurrent->Close();
			bookcurrent = NULL;
			if(mode == APP_MODE_BOOK)
			{
				if(orientation) lcdSwap();
				mode = APP_MODE_BROWSER;
			}
			ts->PrintSplash(ts->screenleft);
			prefs->Write();
			browser_draw();
		}

		else if (keys & KEY_TOUCH)
		{
			// Turn page on touch.
			touchPosition touch;
			touchRead(&touch);
			if (orientation ? touch.py > 95 : touch.py < 96)
			{
				if (pagecurrent > 0)
				{
					TurnPage(-1);
				}
			}
			else
			{
				if (pagecurrent < pagecount-1)
				{
					TurnPage(1);
				}
			}
			prefs->Write();
		}

		else if (keys & KEY_SELECT)
		{
			// Toggle bookmark.
			std::list<u16>* bookmarks = bookcurrent->GetBookmarks();
	
			bool found = false;
			for (std::list<u16>::iterator i = bookmarks->begin(); i != bookmarks->end(); i++) {
				if (*i == pagecurrent)
				{
					bookmarks->erase(i);
					found = true;
					break;
				}
			}
	
			if (!found)
			{
				bookmarks->push_back(pagecurrent);
				bookmarks->sort();
			}
	
			bookcurrent->GetPage()->Draw(ts);
		}
		else if (keys & (key.right | key.left))
		{
			// Navigate bookmarks.
			std::list<u16>* bookmarks = bookcurrent->GetBookmarks();
	
			if (!bookmarks->empty())
			{
				//Find the bookmark just after the current page
				if (keys & key.left)
				{
					std::list<u16>::iterator i;
					for (i = bookmarks->begin(); i != bookmarks->end(); i++) {
						if (*i > bookcurrent->GetPosition())
							break;
					}
			
					if (i == bookmarks->end())
						i = bookmarks->begin();
			
					bookcurrent->SetPosition(*i);
				}
				else // KEY_OTHER by process of elimination
				{
					std::list<u16>::reverse_iterator i;
					for (i = bookmarks->rbegin(); i != bookmarks->rend(); i++) {
						if (*i < bookcurrent->GetPosition())
							break;
					}
			
					if (i == bookmarks->rend())
						i = bookmarks->rbegin();
			
					bookcurrent->SetPosition(*i);
				}
		
				bookcurrent->GetPage()->Draw(ts);
			}
		}

	}

	if(keysUp()) prefs->Write();
}

int App::GetBookIndex(Book *b)
{
	if (!b) return -1;
	vector<Book*>::iterator it;
	int i=0;
	for(it=books.begin(); it<books.end();it++,i++)
	{
		if(*it == b) return i;
	}
	return -1;
}

u8 App::OpenBook(void)
{
	//! Attempt to open book indicated by bookselected.

	if(!bookselected) return 254;	
	PrintStatus("opening book...");
	swiWaitForVBlank();

	const char *filename = bookselected->GetFileName();
	const char *c; 	// will point to the file's extension.
	for (c=filename;c!=filename+strlen(filename) && *c!='.';c++);
	
	if(bookcurrent) bookcurrent->Close();
	bookcurrent=NULL;
	if (int err = bookselected->Open())
	{
		char msg[64];
		sprintf(msg, "could not open book (%d)",err);
		PrintStatus(msg);
		return 255;
	}
	PrintStatus("book opened");
	bookcurrent = bookselected;
	if(mode == APP_MODE_BROWSER) {
		if(orientation) lcdSwap();
		mode = APP_MODE_BOOK;
	}
	if(bookcurrent->GetPosition() >= bookcurrent->GetPageCount())
		bookcurrent->SetPosition(0);
	bookcurrent->GetPage()->Draw(ts);
	prefs->Write();
	ts->PrintStats();
	return 0;
}

void App::parse_error(XML_Parser p)
{
	char msg[128];
	sprintf(msg,"%d:%d: %s\n",
		(int)XML_GetCurrentLineNumber(p),
		(int)XML_GetCurrentColumnNumber(p),
		XML_ErrorString(XML_GetErrorCode(p)));
	Log(msg);
	PrintStatus(msg);
}

void App::TurnPage(int direction) {
    if(!bookcurrent || !bookcurrent->GetPageCount()) return;
    int target=bookcurrent->GetPosition()+(direction>0?1:-1);
    if(target<0 || target>=bookcurrent->GetPageCount()) return;
    bookcurrent->SetPosition(target);
    if(!pageAnimation) {bookcurrent->GetPage()->Draw(ts);return;}
    const int pixels=256*256;
    u16* nextLeft=new(std::nothrow) u16[pixels];
    u16* nextRight=new(std::nothrow) u16[pixels];
    if(!nextLeft || !nextRight) {
        delete[] nextLeft;delete[] nextRight;
        bookcurrent->GetPage()->Draw(ts);return;
    }
    u16* left=ts->screenleft;u16* right=ts->screenright;
    ts->screenleft=nextLeft;ts->screenright=nextRight;
    bookcurrent->GetPage()->Draw(ts);
    ts->screenleft=left;ts->screenright=right;ts->SetScreen(right);
    ReaderVisual::Surface a={left,192,256,256}, b={right,192,256,256};
    int previous=0;
    for(int frame=1;frame<=12;++frame) {
        // Ease out over approximately 200 ms at 60 Hz, excluding render time.
        int remaining=12-frame;
        int progress=192-(192*remaining*remaining)/144;
        swiWaitForVBlank();
        ReaderVisual::advanceTurn(a,nextLeft,previous,progress,direction);
        ReaderVisual::advanceTurn(b,nextRight,previous,progress,direction);
        previous=progress;
    }
    delete[] nextLeft;delete[] nextRight;
}
