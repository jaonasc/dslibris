#include "app.h"
#include "epub.h"
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include "log.h"
extern App* app;
static int parse(const std::string& content) {
    Book b;parsedata_t state;app->parse_init(&state);state.book=&b;
    XML_Parser parser=XML_ParserCreate(NULL);
    XML_SetUserData(parser,&state);XML_SetElementHandler(parser,start_hndl,end_hndl);
    XML_SetCharacterDataHandler(parser,char_hndl);
    XML_Parse(parser,content.data(),content.size(),1);XML_ParserFree(parser);
    return state.status;
}
int main(int argc,char** argv) {
    App application;app=&application;
    std::string which=argc>1?argv[1]:"all";
    if(which=="lifecycle") {
        for(int repeat=0;repeat<200;++repeat) {
            Book book;
            for(int i=0;i<12;++i) {u8 data[1024]={0};book.AppendPage()->SetBuffer(data,sizeof(data));}
            book.Close();assert(book.GetPageCount()==0);
        }
    } else if(which=="depth") {
        std::string content="<html><body>";
        for(int i=0;i<80;++i) content+="<div>";
        content+="Text";for(int i=0;i<80;++i) content+="</div>";
        content+="</body></html>";
        assert(parse(content)>=2);
    } else if(which=="markers") {
        std::string content="<html><body>";
        for(int i=0;i<10000;++i) content+="<strong></strong>";
        content+="</body></html>";assert(parse(content)>=2);
    } else if(which=="invalid-epub") {
        Book book;assert(epub(&book,"bad.epub",false)!=0);assert(book.GetPageCount()==0);
    } else if(which=="empty-epub") {
        Book book;assert(epub(&book,"empty.epub",false)!=0);assert(book.GetPageCount()==0);
    } else if(which=="repeat-epub") {
        Book book;
        for(int i=0;i<40;++i) {assert(epub(&book,"good.epub",false)==0);assert(book.GetPageCount()>0);book.Close();}
    } else if(which=="xml") {
        Book* book=new Book();book->SetFileName("Title & \"quotes\".epub");
        app->books.push_back(book);app->bookcount=1;app->bookdir="Book & Text";
        assert(app->prefs->Write()==0);
        assert(app->prefs->Read()==0);
    } else if(which=="xml-clamp") {
        std::ofstream f("dslibris-modern.xml");
        f<<"<dslibris><font size=\"-999\"/><margin top=\"-200\" left=\"99999\" right=\"-9\" bottom=\"999\"/><paragraph spacing=\"999\" indent=\"-9\"/><screen flip=\"55\"/></dslibris>";f.close();
        assert(app->prefs->Read()==0);
        assert(app->ts->pixelsize>=8 && app->ts->pixelsize<=24);
        assert(app->ts->margin.left>=0 && app->ts->margin.left<=32);
        assert(app->ts->margin.top>=0 && app->ts->margin.top<=32);
        assert(app->ts->margin.right>=0 && app->ts->margin.right<=32);
        assert(app->ts->margin.bottom>=0 && app->ts->margin.bottom<=32);
        assert(app->paraspacing<=12 && app->paraindent<=12 && app->orientation<=1);
    } else if(which=="xml-recovery") {
        app->readingTheme=1;assert(app->prefs->Write()==0);
        app->readingTheme=2;assert(app->prefs->Write()==0);
        std::ofstream f("dslibris-modern.xml");f<<"<dslibris><broken";f.close();
        app->readingTheme=3;assert(app->prefs->Read()==0);assert(app->readingTheme==1);
    } else if(which=="logging") {
        assert(mkdir("dslibris.log",0700)==0);
        Log("Unavailable log must not crash the reader");
    }
    std::cout<<"PASS: "<<which<<std::endl;
}
