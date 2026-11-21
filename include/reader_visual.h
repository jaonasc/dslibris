#ifndef READER_VISUAL_H
#define READER_VISUAL_H

// Portable RGB15 primitives, shared by the DS renderer and host checks.
namespace ReaderVisual {
inline int utf8Next(const char* text,unsigned int& code) {
    const unsigned char* p=(const unsigned char*)text;
    code=0xfffd;
    if(p[0]<0x80) {code=p[0];return p[0]?1:0;}
    int count=p[0]>=0xc2 && p[0]<=0xdf?2:p[0]>=0xe0 && p[0]<=0xef?3:p[0]>=0xf0 && p[0]<=0xf4?4:0;
    if(!count) return 1;
    unsigned int value=p[0]&((1<<(7-count))-1);
    for(int i=1;i<count;++i) {
        if(!p[i] || (p[i]&0xc0)!=0x80) return 1;
        value=(value<<6)|(p[i]&0x3f);
    }
    if((count==2 && value<0x80) || (count==3 && value<0x800) || (count==4 && value<0x10000)
       || value>0x10ffff || (value>=0xd800 && value<=0xdfff)) return 1;
    code=value;return count;
}
typedef unsigned short Pixel;
enum Theme { LIGHT, DARK, BLACK, PAPER, THEME_COUNT };
inline Pixel rgb(int r, int g, int b) { return 0x8000 | r | (g<<5) | (b<<10); }
struct Palette { Pixel paper, ink, surface, accent, muted, border; };
inline Palette palette(int theme) {
    switch (theme) {
    case LIGHT: return {rgb(30,31,31),rgb(3,5,6),rgb(27,29,29),rgb(1,14,13),rgb(10,13,13),rgb(23,26,26)};
    case DARK: return {rgb(3,4,5),rgb(28,29,28),rgb(5,7,8),rgb(11,25,21),rgb(19,21,22),rgb(9,11,12)};
    case BLACK: return {rgb(0,0,0),rgb(29,29,27),rgb(3,3,3),rgb(12,25,21),rgb(19,20,19),rgb(8,8,8)};
    default: return {rgb(31,29,23),rgb(6,4,2),rgb(29,26,19),rgb(7,13,10),rgb(15,12,8),rgb(25,22,16)};
    }
}
inline Pixel blend(Pixel background, Pixel foreground, int alpha) {
    if (alpha<=0) return background;
    if (alpha>=255) return foreground;
    int r=((background&31)*(255-alpha)+(foreground&31)*alpha+127)/255;
    int g=(((background>>5)&31)*(255-alpha)+((foreground>>5)&31)*alpha+127)/255;
    int b=(((background>>10)&31)*(255-alpha)+((foreground>>10)&31)*alpha+127)/255;
    return rgb(r,g,b);
}
struct Surface { Pixel* pixels; int width, height, stride; };
inline void dot(Surface s,int x,int y,Pixel color) {
    if (x>=0 && y>=0 && x<s.width && y<s.height) s.pixels[y*s.stride+x]=color;
}
inline void rect(Surface s,int x,int y,int w,int h,Pixel color) {
    int endx=x+w<s.width?x+w:s.width, endy=y+h<s.height?y+h:s.height;
    for(int j=y<0?0:y;j<endy;++j)
        for(int i=x<0?0:x;i<endx;++i) s.pixels[j*s.stride+i]=color;
}
inline void rounded(Surface s,int x,int y,int w,int h,int radius,Pixel color) {
    if (w<=0 || h<=0) return;
    int r=radius;
    if(r>w/2) r=w/2;
    if(r>h/2) r=h/2;
    for(int j=0;j<h;++j) {
        int dy=j<r?r-1-j:(j>=h-r?j-(h-r):0), inset=0;
        while(inset<r && (r-inset)*(r-inset)+dy*dy>r*r) ++inset;
        rect(s,x+inset,y+j,w-2*inset,1,color);
    }
}
enum Icon { NONE, BOOK, SETTINGS, PREVIOUS, NEXT, SUN, TYPE, SPACING, SHEET, CHECK };
inline void icon(Surface s,int x,int y,int kind,Pixel color) {
    switch(kind) {
    case BOOK:
        rect(s,x,y,12,15,color); rect(s,x+2,y+2,8,10,palette(PAPER).paper);
        rect(s,x+3,y+4,6,1,color); rect(s,x+3,y+7,5,1,color); break;
    case SETTINGS:
        for(int i=0;i<3;++i) {rect(s,x,y+i*5+2,13,1,color);rect(s,x+2+i*3,y+i*5,3,5,color);} break;
    case PREVIOUS: case NEXT:
        for(int i=0;i<6;++i) {int a=kind==NEXT?i:5-i;dot(s,x+a,y+2+i,color);dot(s,x+a,y+12-i,color);} break;
    case SUN:
        rounded(s,x+3,y+3,8,8,4,color);
        rect(s,x+6,y,2,2,color);rect(s,x+6,y+12,2,2,color);
        rect(s,x,y+6,2,2,color);rect(s,x+12,y+6,2,2,color);break;
    case TYPE:
        rect(s,x+2,y+1,10,2,color);rect(s,x+6,y+2,2,12,color);rect(s,x+3,y+12,8,2,color);break;
    case SPACING:
        for(int i=0;i<3;++i) rect(s,x,y+2+i*5,13-i*2,2,color);
        break;
    case SHEET:
        rect(s,x,y,11,14,color);rect(s,x+2,y+2,7,10,palette(PAPER).paper);
        rect(s,x+7,y+11,6,3,color);break;
    case CHECK:
        for(int i=0;i<4;++i) dot(s,x+i,y+6+i,color);
        for(int i=0;i<8;++i) dot(s,x+3+i,y+9-i,color);
        break;
    default: break;
    }
}

// progress is measured in logical pixels, 0..width. Old sheet slides off,
// exposing the next page with a narrow fold and a soft shadow at its edge.
inline void turnFrame(Surface out,const Pixel* old,const Pixel* next,int progress,int direction) {
    int p=progress<0?0:(progress>out.width?out.width:progress);
    int edge=direction>0?out.width-p:p;
    for(int y=0;y<out.height;++y) for(int x=0;x<out.width;++x) {
        bool covered=direction>0?x<edge:x>=edge;
        int source=direction>0?x+p:x-p;
        Pixel value=covered?old[y*out.stride+source]:next[y*out.stride+x];
        if(p>0 && p<out.width) {
            int d=direction>0?x-edge:edge-1-x;
            if(d>=0 && d<9) value=blend(value,rgb(0,0,0),(9-d)*9);
            if(d<0 && d>=-5) value=blend(value,rgb(31,30,27),(-d)*19);
        }
        out.pixels[y*out.stride+x]=value;
    }
}
// In-place variant used on DS: only the new spread needs temporary storage.
// Copy order preserves overlapping pixels. Shadow is drawn on the exposed
// page, so it never becomes part of the moving sheet on a subsequent frame.
inline void advanceTurn(Surface out,const Pixel* next,int previous,int progress,int direction) {
    int p=progress<0?0:(progress>out.width?out.width:progress);
    int delta=p-previous;
    if(delta<0 || delta>out.width) return;
    int edge=direction>0?out.width-p:p;
    for(int y=0;y<out.height;++y) {
        Pixel* row=out.pixels+y*out.stride;
        if(direction>0) {
            for(int x=0;x<edge;++x) row[x]=row[x+delta];
            for(int x=edge;x<out.width;++x) row[x]=next[y*out.stride+x];
        } else {
            for(int x=out.width-1;x>=edge;--x) row[x]=row[x-delta];
            for(int x=0;x<edge;++x) row[x]=next[y*out.stride+x];
        }
        if(p>0 && p<out.width) for(int d=0;d<10;++d) {
            int x=direction>0?edge+d:edge-1-d;
            if(x>=0 && x<out.width) row[x]=blend(row[x],d<2?rgb(31,30,27):rgb(0,0,0),d<2?95:(10-d)*10);
        }
    }
}
}
#endif
