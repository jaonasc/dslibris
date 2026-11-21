#include <stdio.h>
#include <vector>
#include "sys/stat.h"
#include "sys/time.h"
#include "nds.h"
#include "main.h"
#include "app.h"
#include "prefs.h"
#include "book.h"
#include "reader_storage.h"

#define PARSEBUFSIZE 1024*64

Prefs::Prefs() {
	Init();
}
Prefs::Prefs(App *parent) { Init(); app = parent; }
Prefs::~Prefs() {}

//! \return 0: success, 255: file open failure, 254: no bytes read, 253: parse failure.
namespace {
bool parsePrefs(FILE* file,XML_Parser parser) {
    char buffer[8192];
    while(true) {
        size_t count=fread(buffer,1,sizeof(buffer),file);
        if(ferror(file)) return false;
        if(XML_Parse(parser,buffer,count,count==0)==XML_STATUS_ERROR) return false;
        if(!count) return true;
    }
}
}
int Prefs::Read(bool booksOnly) {
    const char* paths[]={PREFSPATH,PREFSPATH ".bak","dslibris.xml"};
    int error=255;
    for(const char* path:paths) {
        FILE* file=fopen(path,"r");if(!file) continue;
        XML_Parser parser=XML_ParserCreate(NULL);
        if(!parser) {fclose(file);return 254;}
        // Validate before applying anything: interrupted XML cannot partially
        // overwrite the defaults or a recovered backup.
        if(!parsePrefs(file,parser)) {XML_ParserFree(parser);fclose(file);error=253;continue;}
        rewind(file);XML_ParserReset(parser,NULL);
        parsedata_t data;app->parse_init(&data);data.prefs=this;data.preferencesBookPass=booksOnly;
        XML_SetUserData(parser,&data);
        XML_SetStartElementHandler(parser,prefs_start_hndl);
        XML_SetEndElementHandler(parser,prefs_end_hndl);
        bool ok=parsePrefs(file,parser);
        XML_ParserFree(parser);fclose(file);
        if(ok) {recoveredBackup=std::string(path)==PREFSPATH ".bak";return 0;}
        error=253;
    }
    return error;
}

//! \return Error code, 0: success.
int Prefs::Write()
{
	int err = 0;
	FILE* fp = fopen(PREFSPATH ".tmp","w");
	if(!fp) return 255;
	
	fprintf(fp, "<dslibris>\n");
	if(swapshoulder)
		fprintf(fp, "<option swapshoulder=\"%d\" />\n",swapshoulder);		
	fprintf(fp, "\t<screen brightness=\"%d\" invert=\"%d\" flip=\"%d\" />\n",
		app->brightness,
		app->ts->GetInvert(),
		app->orientation);
	fprintf(fp,"\t<reading theme=\"%d\" animation=\"%d\" />\n",app->readingTheme,app->pageAnimation);
	fprintf(fp,	"\t<margin top=\"%d\" left=\"%d\" bottom=\"%d\" right=\"%d\" />\n",	
			app->ts->margin.top, app->ts->margin.left,
			app->ts->margin.bottom, app->ts->margin.right);
 	fprintf(fp, "\t<font size=\"%d\" normal=\"%s\" bold=\"%s\" italic=\"%s\" />\n",
		app->ts->GetPixelSize(),
		ReaderStorage::xmlAttribute(app->ts->GetFontFile(TEXT_STYLE_REGULAR)).c_str(),
		ReaderStorage::xmlAttribute(app->ts->GetFontFile(TEXT_STYLE_BOLD)).c_str(),
		ReaderStorage::xmlAttribute(app->ts->GetFontFile(TEXT_STYLE_ITALIC)).c_str());
 	fprintf(fp, "\t<paragraph indent=\"%d\" spacing=\"%d\" />\n",
			app->paraindent,
			app->paraspacing);
	/* TODO save pagination data with current book to cache it to disk.
	   store timestamp too in order to invalidate caches.
	vector<u16> pageindices;
	for(u16 i=0;i<app->pagecount;i++) {}
	*/
    fprintf(fp, "\t<books path=\"%s\" reopen=\"%d\">\n",
		ReaderStorage::xmlAttribute(app->bookdir).c_str(),
    		app->reopen);
    
    for (u8 i = 0; i < app->bookcount; i++) {
        Book* book = app->books[i];
        fprintf(fp, "\t\t<book file=\"%s\" page=\"%d\"",
                ReaderStorage::xmlAttribute(book->GetFileName()).c_str(), book->GetPosition() + 1);
		if(app->bookcurrent == app->books[i]) fprintf(fp," current=\"1\"");
		fprintf(fp,">\n");
		std::list<u16>* bookmarks = book->GetBookmarks();
        for (std::list<u16>::iterator j = bookmarks->begin(); j != bookmarks->end(); j++) {
            fprintf(fp, "\t\t\t<bookmark page=\"%d\" word=\"%d\" />\n",
                    *j + 1,0);
        }

        fprintf(fp, "\t\t</book>\n");
    }

    fprintf(fp, "\t</books>\n");
	
	fprintf(fp, "</dslibris>\n");
	fprintf(fp, "\n");
    bool ok=!ferror(fp);
    if(fclose(fp)) ok=false;
    if(!ok) {remove(PREFSPATH ".tmp");return 254;}
    if(!ReaderStorage::commit(PREFSPATH ".tmp",PREFSPATH,recoveredBackup)) return 253;
    recoveredBackup=false;
    return err;
}

void Prefs::Init(){
	recoveredBackup=false;
	modtime = 0;  // fill this in with gettimeofday()
	swapshoulder = FALSE;
}
