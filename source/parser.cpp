#include <sys/param.h>
#include "app.h"
#include "book.h"
#include "text.h"
#include "main.h"
#include "parse.h"
#include "reader_visual.h"
#include "reader_storage.h"
extern App* app;
extern char msg[];
bool iswhitespace(u8 c)
{
	switch (c)
	{
	case ' ':
	case '\t':
	case '\n':
		return true;
		break;
	default:
		return false;
		break;
	}
}

void parse_init(parsedata_t *data) {app->parse_init(data);}

u8 GetParserFace(parsedata_t *pdata)
{
	if (pdata->italic)
		return TEXT_STYLE_ITALIC;
	else if (pdata->bold)
		return TEXT_STYLE_BOLD;
	else
		return TEXT_STYLE_REGULAR;
}

void WriteBufferToCache(parsedata_t *pdata)
{
	// Only cache if we are laying out text.
	//	if (pdata->cachefile && pdata->status) {
	//		fwrite(pdata->buf, 1, pdata->buflen, pdata->cachefile);
	//}
}

void prefs_start_hndl(	void *data,
						const XML_Char *name,
						const XML_Char **attr)
{
	parsedata_t* p = (parsedata_t*)data;
	int position = 0; //! Page position in book.
	char filename[MAXPATHLEN];
	bool current = FALSE;
	int i;
    if(p->preferencesBookPass && strcmp(name,"book") && strcmp(name,"bookmark")) return;

    if (!strcmp(name,"library"))
	{
		for(i=0;attr[i];i+=2)
			if(!strcmp(attr[i],"modtime"))
			   app->prefs->modtime = atoi(attr[i+1]);
	}
	else if (!strcmp(name,"library"))
	{
		for(i=0;attr[i];i+=2) {
			if(!strcmp(attr[i],"folder"))
				app->bookdir = std::string(attr[i+1]);
		}
	}
	else if (!strcmp(name,"screen"))
	{
		for(i=0;attr[i];i+=2)
		{
			if(!strcmp(attr[i],"brightness"))
			{
				app->brightness = ReaderStorage::boundedInt(attr[i+1],0,3);
				app->brightness = app->brightness % 4;
			}
			else if(!strcmp(attr[i],"invert"))
			{
				app->ts->SetInvert(atoi(attr[i+1]));
				app->readingTheme=atoi(attr[i+1])?ReaderVisual::BLACK:ReaderVisual::PAPER;
			}
			else if(!strcmp(attr[i],"flip"))
			{
				app->orientation = ReaderStorage::boundedInt(attr[i+1],0,1);
			}
		}
	}
	else if (!strcmp(name,"reading"))
	{
		for(i=0;attr[i];i+=2) {
			if(!strcmp(attr[i],"theme")) {
				int theme=atoi(attr[i+1]);
				app->readingTheme=theme>=0 && theme<ReaderVisual::THEME_COUNT?theme:ReaderVisual::PAPER;
			} else if(!strcmp(attr[i],"animation")) app->pageAnimation=atoi(attr[i+1])!=0;
		}
	}
	else if (!strcmp(name,"paragraph"))
	{
		for(i=0;attr[i];i+=2)
		{
			if(!strcmp(attr[i],"spacing")) app->paraspacing = ReaderStorage::boundedInt(attr[i+1],0,12);
			if(!strcmp(attr[i],"indent")) app->paraindent = ReaderStorage::boundedInt(attr[i+1],0,12);
		}
	}
	else if (!strcmp(name,"font"))
	{
		for(i=0;attr[i];i+=2)
		{
			if(!strcmp(attr[i],"size"))
				app->ts->pixelsize = ReaderStorage::boundedInt(attr[i+1],8,24);
			else if(!strcmp(attr[i],"normal"))
				app->ts->SetFontFile((char *)attr[i+1], TEXT_STYLE_REGULAR);
			else if(!strcmp(attr[i],"bold"))
				app->ts->SetFontFile((char *)attr[i+1], TEXT_STYLE_BOLD);
			else if(!strcmp(attr[i],"italic"))
				app->ts->SetFontFile((char *)attr[i+1], TEXT_STYLE_ITALIC);
			else if (!strcmp(attr[i], "path")) {
				if (strlen(attr[i+1]))
					app->fontdir = string(attr[i+1]);
			}
		}
	}
	else if (!strcmp(name, "books"))
	{
		for (i = 0; attr[i]; i+=2) {
			if (!strcmp(attr[i], "reopen"))
				// For prefs where reopen was a string,
				// reopen will get turned off.
				app->reopen = atoi(attr[i+1]);
			else if (!strcmp(attr[i], "path")) {
				if (strlen(attr[i+1]))
					app->bookdir = string(attr[i+1]);
			}
        }
	}
	else if (!strcmp(name, "book"))
	{
		strcpy(filename,"");
		current = FALSE;
		position = 0;
		for (i = 0; attr[i]; i+=2) {
			if (!strcmp(attr[i], "file"))
				snprintf(filename,sizeof(filename),"%s",attr[i+1]);
			if (!strcmp(attr[i], "page"))
				position = ReaderStorage::boundedInt(attr[i+1],1,65535);
			if (!strcmp(attr[i], "current"))
			{
				// Should warn if multiple books are current...
				// the last current book will win.
				if(atoi(attr[i+1])) current = TRUE;
			}
        }

		// Find the book index for this library entry
		// and set context for later bookmarks.
		vector<Book*>::iterator it;
		for(it = app->books.begin(); it < app->books.end(); it++)
		{
			const char *bookname = (*it)->GetFileName();
			if(!strcmp(bookname, filename))
			{
				// bookmark tags will refer to this.
				p->book = *it;

				char msg[128];
				snprintf(msg,sizeof(msg),"info : matched extant book '%s'.\n",bookname);
				app->Log(msg);

				if (current)
				{
					// Set this book as current.
					app->bookcurrent = *it;
					app->bookselected = *it;
				}
				if (position)
					// Set current page in this book.
					(*it)->SetPosition(position - 1);

				break;
			}
		}
	}
	else if (!strcmp(name, "bookmark"))
	{
		for (i = 0; attr[i]; i+=2) {
			if (!strcmp(attr[i], "page"))
				position = ReaderStorage::boundedInt(attr[i+1],1,65535);
		}

		if (p->book)
		{
			p->book->GetBookmarks()->push_back(position - 1);
		}
	}
	else if (!strcmp(name,"margin"))
	{
		for (i=0;attr[i];i+=2)
		{
			if (!strcmp(attr[i],"left")) app->ts->margin.left = ReaderStorage::boundedInt(attr[i+1],0,32);
			if (!strcmp(attr[i],"right")) app->ts->margin.right = ReaderStorage::boundedInt(attr[i+1],0,32);
			if (!strcmp(attr[i],"top")) app->ts->margin.top = ReaderStorage::boundedInt(attr[i+1],0,32);
			if (!strcmp(attr[i],"bottom")) app->ts->margin.bottom = ReaderStorage::boundedInt(attr[i+1],0,32);
		}
	}
	else if (!strcmp(name,"option"))
	{
		for (i=0;attr[i];i+=2)
		{
			if (!strcmp(attr[i],"swapshoulder"))
				p->prefs->swapshoulder = atoi(attr[i+1]);
		}
	}
}

void prefs_end_hndl(void *data, const char *name)
{
	//! Exit element callback for the prefs file.
	parsedata_t *p = (parsedata_t*)data;
	if (!strcmp(name,"book")) p->book = NULL;
}

int unknown_hndl(void *encodingHandlerData,
                 const XML_Char *name,
                 XML_Encoding *info)
{
	// noops!
	strcpy(msg, "warn : encoding handler encountered, did nothing.");
	app->Log(msg);
	return 0;
}

void default_hndl(void *data, const XML_Char *s, int len)
{
	//! Fallback callback. NYUK!

#ifdef DEBUG
	char msg[256];
	strncpy(msg,(const char*)s, len > 255 ? 255 : len);
	app->Log("info : ");
	app->Log(msg);
	app->Log("\n");
#endif

	if(len<=0 || ((parsedata_t*)data)->status>=2) return;
    std::string token(s,len);s=token.c_str();
    int advancespace = app->ts->GetAdvance(' ');
	parsedata_t *p = (parsedata_t*)data;
	if (s[0] == '&')
	{
		/** if it's decimal, convert the UTF-16 to UTF-8. */
		int code=0;
		sscanf(s,"&#%d;",&code);
		if (code)
		{
			if (code>=128 && code<=2047)
			{
				parse_append(p,192 + (code/64));
				parse_append(p,128 + (code%64));
			}
			else if (code>=2048 && code<=65535)
			{
				parse_append(p,224 + (code/4096));
				parse_append(p,128 + ((code/64)%64));
				parse_append(p,128 + (code%64));
			}
			// TODO - support 4-byte codes

			p->pen.x += app->ts->GetAdvance(code);
			return;
		}

		/** otherwise, handle only common HTML named entities. */
		if (!strncmp(s,"&nbsp;",5))
		{
			parse_append(p,' ');
			p->pen.x += advancespace;
			return;
		}
		if (!strcmp(s,"&quot;"))
		{
			parse_append(p,'"');
			p->pen.x += advancespace;
			return;
		}
		if (!strcmp(s,"&amp;"))
		{
			parse_append(p,'&');
			p->pen.x += advancespace;
			return;
		}
		if (!strcmp(s,"&lt;"))
		{
			parse_append(p,'<');
			p->pen.x += advancespace;
			return;
		}
		if (!strcmp(s,"&lt;"))
		{
			parse_append(p,'>');
			p->pen.x += advancespace;
			return;
		}
	}
}

static char title[32];

void title_start_hndl(void *userdata, const char *el, const char **attr)
{
	//! Expat callback, when entering an element.
	//! For finding book title only.

	if(!strcmp(el,"title"))
	{
		app->parse_push((parsedata_t *)userdata,TAG_TITLE);
		strcpy(title,"");
	}
}

void title_char_hndl(void *userdata, const char *txt, int txtlen)
{
	//! Expat callback, when in char data for element.
	//! For finding book title only.
	if(strlen(title) > 27) return;
	if (!app->parse_in((parsedata_t*)userdata,TAG_TITLE)) return;

	for(int t=0;t<txtlen;t++)
	{
		if(iswhitespace(txt[t]))
		{
			if(strlen(title)) strncat(title," ",2);
		}
		else strncat(title,txt+t,1);

		if (strlen(title) > 27)
		{
			strcat(title+27, "...");
			break;
		}
	}
}

void title_end_hndl(void *userdata, const char *el)
{
	//! Expat callback, when exiting an element.
	//! For finding book title only.
	parsedata_t *data = (parsedata_t*)userdata;
	if(!strcmp(el,"title")) data->book->SetTitle(title);
	if(!strcmp(el,"head")) data->status = 1; // done.
	app->parse_pop(data);
}

// Expat callbacks for parsing full text follow. //

void linefeed(parsedata_t *p) {
	Text *ts = app->ts;
	parse_append(p,'\n');
	p->pen.x = MARGINLEFT;
	p->pen.y += ts->GetHeight() + ts->linespacing;
	p->linebegan = false;
}

bool blankline(parsedata_t *p) {
	// Was the preceding text a blank line?
	if (p->buflen < 3) return true;
	return (p->buf[p->buflen-1] == '\n') && (p->buf[p->buflen-2] == '\n');
}

void start_hndl(void *data, const char *el, const char **attr)
{
    if(((parsedata_t*)data)->status>=2) return;
	//! Expat callback, when starting an element.

	parsedata_t *p = (parsedata_t*)data;

	if (!strcmp(el,"html")) app->parse_push(p,TAG_HTML);
	else if (!strcmp(el,"body")) app->parse_push(p,TAG_BODY);
	else if (!strcmp(el,"div")) app->parse_push(p,TAG_DIV);
	else if (!strcmp(el,"dt")) app->parse_push(p,TAG_DT);
	else if (!strcmp(el,"h1")) {
		app->parse_push(p,TAG_H1);
		bool lf = !blankline(p);
		parse_append(p,TEXT_BOLD_ON);
		p->pos++;
		p->bold = true;
		if (lf) linefeed(p);
	}
	else if (!strcmp(el,"h2")) {
		app->parse_push(p,TAG_H2);
		bool lf = !blankline(p);
		parse_append(p,TEXT_BOLD_ON);
		p->pos++;
		p->bold = true;
		if (lf) linefeed(p);
	}
	else if (!strcmp(el,"h3")) {
		app->parse_push(p,TAG_H3);
		linefeed(p);
	}
	else if (!strcmp(el,"h4")) {
		app->parse_push(p,TAG_H4);
		if (!blankline(p)) linefeed(p);
	}
	else if (!strcmp(el,"h5")) {
		app->parse_push(p,TAG_H5);
		if (!blankline(p)) linefeed(p);
	}
	else if (!strcmp(el,"h6")) {
		app->parse_push(p,TAG_H6);
		if (!blankline(p)) linefeed(p);
	}
	else if (!strcmp(el,"head")) app->parse_push(p,TAG_HEAD);
	else if (!strcmp(el,"ol")) app->parse_push(p,TAG_OL);
	else if (!strcmp(el,"p")) {
		app->parse_push(p,TAG_P);
		if (!blankline(p)) {
			// for(int i=0;i<app->paraspacing;i++)
			// {
			// 	linefeed(p);
			// }
			// for(int i=0;i<app->paraindent;i++)
			// {
			// 	parse_append(p,' ');
			// 	p->pen.x += ts->GetAdvance(' ');
			// }
			linefeed(p);
		}
	}
	else if (!strcmp(el,"pre")) app->parse_push(p,TAG_PRE);
	else if (!strcmp(el,"script")) app->parse_push(p,TAG_SCRIPT);
	else if (!strcmp(el,"style")) app->parse_push(p,TAG_STYLE);
	else if (!strcmp(el,"title")) app->parse_push(p,TAG_TITLE);
	else if (!strcmp(el,"td")) app->parse_push(p,TAG_TD);
	else if (!strcmp(el,"ul")) app->parse_push(p,TAG_UL);
	else if (!strcmp(el,"strong") || !strcmp(el, "b")) {
		app->parse_push(p,TAG_STRONG);
		parse_append(p,TEXT_BOLD_ON);
		p->pos++;
		p->bold = true;
	}
	else if (!strcmp(el,"em") || !strcmp(el, "i")) {
		app->parse_push(p,TAG_EM);
		parse_append(p,TEXT_ITALIC_ON);
		p->italic = true;
	}
	else app->parse_push(p,TAG_UNKNOWN);
}

void char_hndl(void *data, const XML_Char *txt, int txtlen)
{
    if(((parsedata_t*)data)->status>=2) return;
	//! reflow text on the fly, into page data structure.

	parsedata_t *p = (parsedata_t *)data;
	//if (p->pagecount > 3) return;
	if (app->parse_in(p,TAG_TITLE)) return;
	if (app->parse_in(p,TAG_SCRIPT)) return;
	if (app->parse_in(p,TAG_STYLE)) return;

	Text *ts = app->ts;
	int lineheight = ts->GetHeight();
	int linespacing = ts->linespacing;
	int spaceadvance = ts->GetAdvance((u16)' ');

	if (p->buflen == 0)
	{
		/** starting a new page. **/
		p->pen.x = ts->margin.left;
		p->pen.y = ts->margin.top + lineheight;
		p->linebegan = false;
	}

	u8 advance=0;
	int i=0, j=0;
	while (i<txtlen)
	{
		if (txt[i] == '\r')
		{
			i++;
			continue;
		}

		if (iswhitespace(txt[i]))
		{
			if(app->parse_in(p,TAG_PRE))
			{
				parse_append(p,txt[i]);
				if(txt[i] == '\n')
				{
					p->pen.x = ts->margin.left;
					p->pen.y += (lineheight + linespacing);
				}
				else {
					p->pen.x += spaceadvance;
				}
			}
			else if(p->linebegan && p->buflen
				&& !iswhitespace(p->buf[p->buflen-1]))
			{
				parse_append(p,' ');
				p->pen.x += spaceadvance;
			}
			i++;
		}
		else
		{
			p->linebegan = true;
			advance = 0;
			u8 bytes = 1;
			for (j=i;(j<txtlen) && (!iswhitespace(txt[j]));j+=bytes)
			{
				/** set type until the end of the next word.
				    account for UTF-8 characters when advancing. **/
				u32 code = txt[j];
				bytes = 1;
				if (code >> 7) {
					// FIXME the performance bottleneck
					bytes = ts->GetCharCode((char*)&(txt[j]),&code);
				}

				advance += ts->GetAdvance(code);
				if(advance > ts->display.width - ts->margin.right - ts->margin.left)
				{
					// here's a line-long word, need to break it now.
					break;
				}
			}
		}

		if ((p->pen.x + advance) > (ts->display.width - ts->margin.right))
		{
			// we overran the margin, insert a break.
			parse_append(p,'\n');
			p->pen.x = ts->margin.left;
			p->pen.y += (lineheight + linespacing);
			p->linebegan = false;
		}

		if (p->pen.y > (ts->display.height - ts->margin.bottom))
		{
			// reached bottom of screen.
			if(p->screen == 1)
			{
				// page full.
				// put chars into current page.
				Page *page = p->book->AppendPage();
				page->SetBuffer(p->buf, p->buflen);
				page->start = p->pos;
				p->pos += p->buflen;
				page->end = p->pos;
				p->pagecount++;

				// make a new page.
				p->buflen = 0;
				if (p->italic) parse_append(p,TEXT_ITALIC_ON);
				if (p->bold) parse_append(p,TEXT_BOLD_ON);
				p->screen = 0;
			}
			else
				// move to right screen.
				p->screen = 1;

			p->pen.x = ts->margin.left;
			p->pen.y = ts->margin.top + lineheight;
		}

		/** append this word to the page.
			chars stay UTF-8 until they are rendered. **/

		for (;i<j;i++)
		{
			if (iswhitespace(txt[i]))
			{
				if (p->linebegan)
				{
					parse_append(p,' ');
				}
			}
			else
			{
				p->linebegan = true;
				parse_append(p,txt[i]);
			}
		}
		p->pen.x += advance;
		advance = 0;
	}
}  /* End char_hndl */

void end_hndl(void *data, const char *el)
{
    if(((parsedata_t*)data)->status>=2) return;
	parsedata_t *p = (parsedata_t *)data;
	Text *ts = app->ts;

	if (!strcmp(el,"body")) {
		// Save off our last page.
		Page *page = p->book->AppendPage();
		page->SetBuffer(p->buf,p->buflen);
		if (app->cache)
			WriteBufferToCache(p);
		p->buflen = 0;
		// Retain styles across the page.
		if (p->italic) parse_append(p,TEXT_ITALIC_ON);
		if (p->bold) parse_append(p,TEXT_BOLD_ON);
		app->parse_pop(p);
		return;
	}

	if (!strcmp(el,"br")) {
		linefeed(p);
	} else if (!strcmp(el,"p")) {
		linefeed(p);
		linefeed(p);
	} else if(!strcmp(el,"div")) {
	} else if (!strcmp(el, "strong") || !strcmp(el, "b")) {
		parse_append(p,TEXT_BOLD_OFF);
		p->bold = false;
	} else if (!strcmp(el, "em") || !strcmp(el, "i")) {
		parse_append(p,TEXT_ITALIC_OFF);
		p->italic = false;
	} else if(!strcmp(el,"h1")) {
		parse_append(p,TEXT_BOLD_OFF);
		p->bold = false;
		linefeed(p);
		linefeed(p);
	} else if (!strcmp(el,"h2")) {
		parse_append(p,TEXT_BOLD_OFF);
		p->bold = false;
		linefeed(p);
	} else if (
		   !strcmp(el,"h3")
		|| !strcmp(el,"h4")
		|| !strcmp(el,"h5")
		|| !strcmp(el,"h6")
		|| !strcmp(el,"hr")
		|| !strcmp(el,"pre")
	) {
		linefeed(p);
		linefeed(p);
	} else if (
		   !strcmp(el, "li")
		|| !strcmp(el, "ul")
		|| !strcmp(el, "ol")
	) {
		linefeed(p);
	}

	app->parse_pop(p);

	if (p->pen.y > (ts->display.height - ts->margin.bottom))
	{
		if (p->screen == 1)
		{
			// End of right screen; end of page.
			// Copy in buffered char data into a new page.
			Page *page = p->book->AppendPage();
			page->SetBuffer(p->buf, p->buflen);
			if (app->cache)
				WriteBufferToCache(p);
			p->buflen = 0;
			if (p->italic) parse_append(p,TEXT_ITALIC_ON);
			if (p->bold )parse_append(p,TEXT_BOLD_ON);
			p->screen = 0;
		}
		else
			// End of left screen; same page, next screen.
			p->screen = 1;
		p->pen.x = ts->margin.left;
		p->pen.y = ts->margin.top + ts->GetHeight();
		p->linebegan = false;
	}

}

void proc_hndl(void *data, const char *target, const char *pidata)
{
	app->Log("called proc_hndl().\n");
}
