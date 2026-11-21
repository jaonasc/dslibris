#ifndef READER_STORAGE_H
#define READER_STORAGE_H
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <sys/stat.h>
namespace ReaderStorage {
inline int boundedInt(const char* text,int low,int high) {
    char* end=NULL;long value=strtol(text,&end,10);
    if(end==text) value=low;
    return value<low?low:value>high?high:(int)value;
}
inline std::string xmlAttribute(const std::string& text) {
    std::string result;
    for(char c:text) switch(c) {
        case '&':result+="&amp;";break;
        case '"':result+="&quot;";break;
        case '\'':result+="&apos;";break;
        case '<':result+="&lt;";break;
        case '>':result+="&gt;";break;
        default:result+=c;break;
    }
    return result;
}
inline bool commit(const char* temporary,const char* target,bool preserveBackup=false) {
    std::string backup=std::string(target)+".bak";
    struct stat info;
    bool exists=stat(target,&info)==0;
    if(!exists && errno!=ENOENT) return false;
    if(exists && !S_ISREG(info.st_mode)) return false;
    bool rotated=false;
    if(exists && !preserveBackup) {
        if(remove(backup.c_str()) && errno!=ENOENT) return false;
        if(rename(target,backup.c_str())) return false;
        rotated=true;
    } else if(exists && remove(target)) return false;
    if(rename(temporary,target)==0) return true;
    if(rotated) rename(backup.c_str(),target);
    return false;
}
}
#endif
