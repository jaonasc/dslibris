#include "ui.h"
#include "main.h"
#include "app.h"

using namespace ReaderVisual;
MenuScope::MenuScope(Text* text,u16* target,int theme):t(text) {
    screen=t->screen;paper=t->paperColor;ink=t->inkColor;
    size=t->GetPixelSize();style=t->GetStyle();t->GetPen(&x,&y);
    t->SetScreen(target);t->SetStyle(TEXT_STYLE_BROWSER);t->SetPixelSize(12);
    Palette p=palette(theme);t->paperColor=p.paper;t->inkColor=p.ink;
}
MenuScope::~MenuScope() {
    t->paperColor=paper;t->inkColor=ink;t->SetStyle(style);
    t->SetPixelSize(size);t->SetScreen(screen);t->SetPen(x,y);
}
void UiText(Text* t,int x,int baseline,const char* text,int width,int size,u16 ink) {
    if(width<=0 || !text) return;
    t->SetPixelSize(size);t->inkColor=ink;t->SetPen(x,baseline);
    // Width helper returns a byte count ending on a complete UTF-8 character.
    int count=t->GetCharCountInsideWidth(text,TEXT_STYLE_BROWSER,width>255?255:width);
    std::string label(text,count);
    t->PrintString(label.c_str(),TEXT_STYLE_BROWSER);
}
void UiHeader(Text* t,const char* title,const char* subtitle,int theme) {
    Palette p=palette(theme);
    Surface s={t->screen,192,256,256};
    rect(s,0,0,192,256,p.paper);
    rounded(s,10,10,24,3,1,p.accent);
    UiText(t,10,34,title,174,18,p.ink);
    UiText(t,10,51,subtitle,174,10,p.muted);
}
const char* ThemeName(int theme) {
    switch(theme) {
    case LIGHT:return "Claro";
    case DARK:return "Escuro";
    case BLACK:return "Preto";
    default:return "Papel amarelado";
    }
}
void App::ApplyTheme() {
    Palette p=palette(readingTheme);
    ts->paperColor=p.paper;ts->inkColor=p.ink;
    ts->SetInvert(readingTheme==DARK || readingTheme==BLACK);
}
void App::DrawCompanion() {
    MenuScope scope(ts,ts->screenleft,readingTheme);
    Palette p=palette(readingTheme);
    Surface s={ts->screenleft,192,256,256};
    rect(s,0,0,192,256,p.paper);
    rounded(s,14,18,40,40,10,p.surface);
    icon(s,28,30,BOOK,p.accent);
    UiText(ts,14,91,"dslibris",164,26,p.ink);
    UiText(ts,15,111,"M O D E R N",162,10,p.accent);
    rounded(s,14,129,164,1,0,p.border);
    UiText(ts,14,158,mode==APP_MODE_BROWSER?"Sua proxima boa leitura.":"Leitura do seu jeito.",164,12,p.ink);
    UiText(ts,14,181,"X  /  alternar tema no livro",164,10,p.muted);
    UiText(ts,14,199,"Y  /  ajustar brilho",164,10,p.muted);
    UiText(ts,14,217,"START  /  biblioteca",164,10,p.muted);
    UiText(ts,14,245,ThemeName(readingTheme),164,10,p.accent);
}
