// Only hardware/font rendering is substituted. Book, Page, EPUB, Expat,
// parser state and preference serialization are the production implementations.
#include "app.h"
#include "reader_visual.h"
#include <cstring>
App* app;
char msg[256];
Button::Button():ts(NULL),iconKind(0) {}
App::App() {
    ts=new Text();ts->app=this;prefs=&myprefs;prefs->app=this;
    brightness=1;orientation=0;readingTheme=ReaderVisual::PAPER;
    pageAnimation=true;reopen=false;cache=false;console=false;paraspacing=1;paraindent=0;
    bookdir="Book";fontdir="Font";bookcount=0;bookcurrent=bookselected=NULL;
}
App::~App() {for(auto b:books) delete b;delete ts;}
void App::Log(const char*) {}
void App::Log(const char*,const char*) {}
void App::Log(const std::string) {}
void App::PrintStatus(const char*) {}
void App::parse_error(XML_Parser) {}
#ifndef HOST_REAL_TEXT
Text::Text() {
    pixelsize=12;linespacing=0;display.width=192;display.height=256;
    margin.left=margin.right=12;margin.top=10;margin.bottom=16;invert=false;
    filenames[0]="LiberationSerif-Regular.ttf";filenames[1]="LiberationSerif-Bold.ttf";filenames[2]="LiberationSerif-Italic.ttf";
}
Text::~Text() {}
u8 Text::GetHeight() {return 16;}
u8 Text::GetAdvance(u32) {return 7;}
u8 Text::GetAdvance(u32,u8) {return 7;}
u8 Text::GetCharCode(const char* s,u32* code) {
    unsigned int c=0;int bytes=ReaderVisual::utf8Next(s,c);*code=c;return bytes;
}
void Text::SetInvert(bool state) {invert=state;}
bool Text::GetInvert() {return invert;}
u8 Text::GetPixelSize() {return pixelsize;}
void Text::SetFontFile(const char* f,u8 style) {filenames[style]=f;}
std::string Text::GetFontFile(u8 style) {return filenames[style];}
#endif
