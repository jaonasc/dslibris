/* DSLibris - Copyright (C) 2007-2008 Ray Haleblian.
 * Modified for DSLibris Modern. GNU GPL v2 or later; see COPYING.
 * Distributed without any warranty.
 */
#include "epub.h"
#include "main.h"
#include "parse.h"
#include "expat.h"
#include "unzip.h"
#include <cstring>
#include <vector>
#include <algorithm>
#include <cctype>

namespace {
struct EpubState:epub_data_t {
    bool failed;
    EpubState() {
        type=PARSE_CONTAINER;book=NULL;metadataonly=false;failed=false;
        ctx.push_back(new std::string("TOP"));
    }
    ~EpubState() {
        for(auto p:ctx) delete p;
        for(auto p:manifest) delete p;
        for(auto p:spine) delete p;
    }
};
void containerStart(void* data,const char* name,const char** attributes) {
    EpubState* d=(EpubState*)data;
    if(!strcmp(name,"rootfile")) for(int i=0;attributes[i];i+=2)
        if(!strcmp(attributes[i],"full-path")) d->rootfile=attributes[i+1];
}
void metadataStart(void* data,const char* name,const char** attributes) {
    EpubState* d=(EpubState*)data;
    if(d->failed) return;
    if(d->ctx.size()>=64) {d->failed=true;return;}
    const std::string parent=d->ctx.empty()?"":*d->ctx.back();
    std::string element=name;
    if((parent=="manifest" || parent=="opf:manifest") && (element=="item" || element=="opf:item")) {
        epub_item* item=new epub_item();d->manifest.push_back(item);
        for(int i=0;attributes[i];i+=2) {
            if(!strcmp(attributes[i],"id")) item->id=attributes[i+1];
            else if(!strcmp(attributes[i],"href")) item->href=attributes[i+1];
        }
    } else if((parent=="spine" || parent=="opf:spine") && (element=="itemref" || element=="opf:itemref")) {
        epub_itemref* ref=new epub_itemref();d->spine.push_back(ref);
        for(int i=0;attributes[i];i+=2) if(!strcmp(attributes[i],"idref")) ref->idref=attributes[i+1];
    }
    d->ctx.push_back(new std::string(element));
}
void metadataEnd(void* data,const char*) {
    EpubState* d=(EpubState*)data;
    if(d->failed || d->ctx.size()<=1) return;
    delete d->ctx.back();d->ctx.pop_back();
}
void metadataText(void* data,const char* text,int size) {
    EpubState* d=(EpubState*)data;
    if(d->failed || d->ctx.empty()) return;
    if(*d->ctx.back()=="dc:title") d->title.append(text,size);
    else if(*d->ctx.back()=="dc:creator") d->creator.append(text,size);
}
int parseCurrent(unzFile zip,EpubState& state) {
    XML_Parser parser=XML_ParserCreate(NULL);
    if(!parser) return -1;
    parsedata_t content;
    if(state.type==PARSE_CONTENT) {
        parse_init(&content);content.book=state.book;
        XML_SetUserData(parser,&content);
        XML_SetElementHandler(parser,start_hndl,end_hndl);
        XML_SetCharacterDataHandler(parser,char_hndl);
        XML_SetDefaultHandler(parser,default_hndl);
        XML_SetProcessingInstructionHandler(parser,proc_hndl);
    } else {
        XML_SetUserData(parser,&state);
        if(state.type==PARSE_CONTAINER) XML_SetStartElementHandler(parser,containerStart);
        else {
            XML_SetElementHandler(parser,metadataStart,metadataEnd);
            XML_SetCharacterDataHandler(parser,metadataText);
        }
    }
    std::vector<char> buffer(BUFSIZE);
    int result=0;
    while(true) {
        int bytes=unzReadCurrentFile(zip,&buffer[0],buffer.size());
        if(bytes<0) {result=bytes;break;}
        if(XML_Parse(parser,&buffer[0],bytes,bytes==0)==XML_STATUS_ERROR) {result=-2;break;}
        if(state.failed || (state.type==PARSE_CONTENT && content.status>=2)) {result=-3;break;}
        if(!bytes) break;
    }
    XML_ParserFree(parser);
    return result;
}
int readEntry(unzFile zip,const std::string& name,EpubState& state) {
    int result=unzLocateFile(zip,name.c_str(),0);
    if(result) return result;
    result=unzOpenCurrentFile(zip);if(result) return result;
    result=parseCurrent(zip,state);
    int closed=unzCloseCurrentFile(zip);
    return result?result:closed;
}
struct ZipScope {
    unzFile file;
    explicit ZipScope(unzFile f):file(f) {}
    ~ZipScope() {if(file) unzClose(file);}
};
}
int epub(Book* book,std::string filename,bool metadataOnly) {
    ZipScope zip(unzOpen(filename.c_str()));
    if(!zip.file) return -1;
    EpubState state;state.book=book;
    int error=readEntry(zip.file,"META-INF/container.xml",state);
    if(error || state.rootfile.empty()) return error?error:-2;
    std::string rootfile=state.rootfile;
    size_t slash=rootfile.find_last_of('/');
    std::string folder=slash==std::string::npos?"":rootfile.substr(0,slash+1);
    state.type=PARSE_ROOTFILE;
    error=readEntry(zip.file,rootfile,state);if(error) return error;
    if(!state.title.empty()) book->SetTitle(state.title.c_str());
    if(!state.creator.empty()) book->SetAuthor(state.creator);
    if(metadataOnly) return 0;
    book->Close();
    std::vector<std::string> sections;
    if(!state.spine.empty()) {
        for(auto ref:state.spine) {
            bool found=false;
            for(auto item:state.manifest) if(item->id==ref->idref) {sections.push_back(item->href);found=true;break;}
            if(!found) return -3;
        }
    } else for(auto item:state.manifest) sections.push_back(item->href);
    state.type=PARSE_CONTENT;
    for(std::string href:sections) {
        size_t fragment=href.find('#');if(fragment!=std::string::npos) href.resize(fragment);
        size_t dot=href.find_last_of('.');if(dot==std::string::npos) continue;
        std::string extension=href.substr(dot);
        std::transform(extension.begin(),extension.end(),extension.begin(),[](unsigned char c){return (char)tolower(c);});
        if(extension!=".xhtml" && extension!=".html" && extension!=".htm") continue;
        error=readEntry(zip.file,folder+href,state);
        if(error) {book->Close();return error;}
    }
    return book->GetPageCount()?0:-4;
}
