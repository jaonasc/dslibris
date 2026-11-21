#include "reader_storage.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <sys/stat.h>
int main() {
    remove("prefs.xml");remove("missing.tmp");
    {std::ofstream backup("prefs.xml.bak");backup<<"last good preferences";}
    assert(!ReaderStorage::commit("missing.tmp","prefs.xml"));
    struct stat info;
    assert(stat("prefs.xml.bak",&info)==0 && info.st_size>0);
    {std::ofstream current("prefs.xml");current<<"current preferences";}
    assert(!ReaderStorage::commit("missing.tmp","prefs.xml"));
    assert(stat("prefs.xml",&info)==0 && info.st_size>0);
    {std::ofstream backup("prefs.xml.bak");backup<<"known good backup";}
    {std::ofstream broken("prefs.xml");broken<<"<incomplete";}
    assert(!ReaderStorage::commit("missing.tmp","prefs.xml",true));
    std::ifstream recovered("prefs.xml.bak");std::string content;
    std::getline(recovered,content);assert(content=="known good backup");
    std::puts("PASS: failed preference commit preserves recovered backup and current file");
}
