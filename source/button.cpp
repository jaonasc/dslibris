/* DSLibris - Copyright (C) 2007-2008 Ray Haleblian.
 * Modified for DSLibris Modern. GNU GPL v2 or later; see COPYING.
 * Distributed without any warranty.
 */
#include "button.h"
#include "app.h"
#include "ui.h"

using namespace ReaderVisual;
Button::Button():ts(NULL),iconKind(NONE) {}
void Button::Init(Text* typesetter) {
    ts=typesetter;origin.x=origin.y=0;extent.x=176;extent.y=34;
    text.clear();text2.clear();iconKind=NONE;
}
void Button::Label(const char* s) {text=s?s:"";}
void Button::SetLabel(std::string& s) {text=s;}
void Button::SetLabel2(std::string s) {text2=s;}
void Button::Move(u16 x,u16 y) {origin.x=x;origin.y=y;}
void Button::Resize(u16 x,u16 y) {extent.x=x;extent.y=y;}
void Button::Draw(u16* fb,bool highlight) {
    MenuScope scope(ts,fb,ts->app->readingTheme);
    Palette p=palette(ts->app->readingTheme);
    Surface s={fb,192,256,256};
    int x=origin.x,y=origin.y,w=extent.x,h=extent.y;
    Pixel bg=highlight?blend(p.surface,p.accent,35):p.surface;
    rounded(s,x,y,w,h,6,highlight?p.accent:p.border);
    rounded(s,x+1,y+1,w-2,h-2,5,bg);
    if(highlight) rounded(s,x+1,y+7,3,h-14,1,p.accent);
    int offset=iconKind==NONE?9:29;
    if(iconKind!=NONE) icon(s,x+9,y+(h-15)/2,iconKind,p.accent);
    int size=h<32?10:12;
    UiText(ts,x+offset,y+(text2.empty()?(h+size)/2-1:14),text.c_str(),w-offset-7,size,p.ink);
    if(!text2.empty()) UiText(ts,x+offset,y+h-6,text2.c_str(),w-offset-7,9,p.muted);
}
bool Button::EnclosesPoint(u16 x,u16 y) {
    return x>=origin.x && y>=origin.y && x<origin.x+extent.x && y<origin.y+extent.y;
}
