#include "reader_visual.h"
#include <cassert>
#include <cstdio>
#include <vector>

int main() {
    using namespace ReaderVisual;
    assert(blend(rgb(31,29,23), rgb(4,3,2), 0) == rgb(31,29,23));
    assert(blend(rgb(31,29,23), rgb(4,3,2), 255) == rgb(4,3,2));
    for (int i=0; i<THEME_COUNT; ++i) {
        Palette p = palette(i);
        assert(p.paper != p.ink);
        int paper = (p.paper&31)+((p.paper>>5)&31)+((p.paper>>10)&31);
        int ink = (p.ink&31)+((p.ink>>5)&31)+((p.ink>>10)&31);
        assert(paper-ink > 50 || ink-paper > 50);
    }
    assert(palette(999).paper == palette(PAPER).paper);
    unsigned int code=0;
    assert(utf8Next("\xc3\xa7",code)==2 && code==0xe7);
    assert(utf8Next("\xf0\x9f\x93\x96",code)==4 && code==0x1f4d6);
    assert(utf8Next("\xe2",code)==1 && code==0xfffd);
    assert(utf8Next("\x80",code)==1 && code==0xfffd);
    std::vector<Pixel> storage(256*256+2, 0x1234);
    Surface s = {&storage[1],192,256,256};
    rounded(s,-3,-3,210,280,8,rgb(1,2,3));
    assert(storage.front()==0x1234 && storage.back()==0x1234);
    assert(storage[1]==rgb(1,2,3));
    assert(storage[1+191+255*256]==rgb(1,2,3));
    assert(storage[1+192]==0x1234); // stride padding untouched
    std::vector<Pixel> old(256*256, rgb(25,20,10)), next(256*256, rgb(2,3,4));
    for (int direction=-1; direction<=1; direction+=2) {
        Surface out = {&storage[1],192,256,256};
        turnFrame(out,&old[0],&next[0],0,direction);
        assert(out.pixels[0]==old[0] && out.pixels[191]==old[191]);
        turnFrame(out,&old[0],&next[0],192,direction);
        assert(out.pixels[0]==next[0] && out.pixels[191]==next[191]);
        turnFrame(out,&old[0],&next[0],96,direction);
        assert(out.pixels[direction>0?191:0]==next[0]);
        assert(out.pixels[direction>0?0:191]==old[0]);
        assert(storage.front()==0x1234 && storage.back()==0x1234);
        assert(out.pixels[192]==0x1234);
        // Native animation advances in place, preserving the old sheet until
        // it has left the display. A wrong copy direction corrupts this pattern.
        for(int y=0;y<256;++y) for(int x=0;x<192;++x) out.pixels[y*256+x]=rgb(x%31,y%31,0);
        advanceTurn(out,&next[0],0,48,direction);
        assert(out.pixels[direction>0?0:191]==rgb(direction>0?48%31:143%31,0,0));
        advanceTurn(out,&next[0],48,96,direction);
        advanceTurn(out,&next[0],96,192,direction);
        for(int y=0;y<256;++y) for(int x=0;x<192;++x) assert(out.pixels[y*256+x]==next[y*256+x]);
    }
    std::puts("PASS: themes, alpha, clipping, guards, animation directions/endpoints/stride");
}
