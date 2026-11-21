"""Configure a temporary DeSmuME libretro checkout for homebrew FAT testing."""
from pathlib import Path
import sys
p=Path(sys.argv[1])/'desmume/src/frontend/libretro/libretro.cpp'
s=p.read_text()
if 'DSLIBRIS_TEST_FAT' not in s:
    s=s.replace('#include "NDSSystem.h"','#include "NDSSystem.h"\n#include "slot2.h"')
    s=s.replace('   if (NDS_LoadROM(game->path) < 0)', '''   const char* fat=getenv("DSLIBRIS_TEST_FAT");
   if(fat) {
      CFlash_Mode=ADDON_CFLASH_MODE_Path;
      CFlash_Path=fat;
      slot2_Change(NDS_SLOT2_CFLASH);
   }
   if (NDS_LoadROM(game->path) < 0)''')
    p.write_text(s)
make=p.parent/'Makefile.libretro'
s=make.read_text().replace('LIBS := -lpthread -lGL -lpcap','LIBS := -lpthread')
s=s.replace('CXXFLAGS += -DHAVE_OPENGL -std=gnu++11','CXXFLAGS += -std=gnu++11')
s=s.replace('DESMUME_OPENGL = 1','DESMUME_OPENGL = 0').replace('DESMUME_OPENGL_CORE = 1','DESMUME_OPENGL_CORE = 0')
s=s.replace('-O3 -DNDEBUG','-O1 -DNDEBUG')
make.write_text(s)
wifi=Path(sys.argv[1])/'desmume/src/wifi.cpp'
s=wifi.read_text().replace('#include <pcap/pcap.h>','#define NO_PCAP\ntypedef void* pcap_pkthdr;')
wifi.write_text(s)
