#include "app.h"
#include "ui.h"
#include <cstdio>
#include <cstring>
using namespace ReaderVisual;

void App::HandleEventInBrowser() {
    uint32 keys=keysDown();
    if(keys&(KEY_A|key.down)) AttemptBookOpen();
    else if(keys&KEY_Y) {CycleBrightness();prefs->Write();}
    else if(keys&KEY_SELECT) {mode=APP_MODE_PREFS;prefsPage=0;prefsSelected=PREFS_BUTTON_THEME;PrefsDraw();}
    else if(keys&(key.left|key.l)) {
        int b=GetBookIndex(bookselected);
        if(b>=0 && b+1<bookcount) {
            bookselected=books[++b];
            browserstart=(b/APP_BROWSER_BUTTON_COUNT)*APP_BROWSER_BUTTON_COUNT;
            browser_draw();
        }
    } else if(keys&(key.right|key.r)) {
        int b=GetBookIndex(bookselected);
        if(b>0) {
            bookselected=books[--b];browserstart=(b/APP_BROWSER_BUTTON_COUNT)*APP_BROWSER_BUTTON_COUNT;
            browser_draw();
        }
    } else if(keys&KEY_TOUCH) {
        touchPosition touch;touchRead(&touch);
        int x=orientation?191-touch.py:touch.py;
        int y=orientation?touch.px:255-touch.px;
        if(buttonnext.EnclosesPoint(x,y)) {browser_nextpage();browser_draw();}
        else if(buttonprev.EnclosesPoint(x,y)) {browser_prevpage();browser_draw();}
        else if(buttonprefs.EnclosesPoint(x,y)) {
            mode=APP_MODE_PREFS;prefsPage=0;prefsSelected=PREFS_BUTTON_THEME;PrefsDraw();
        } else for(int i=browserstart;i<bookcount && i<browserstart+APP_BROWSER_BUTTON_COUNT;++i) {
            if(buttons[i]->EnclosesPoint(x,y)) {bookselected=books[i];AttemptBookOpen();break;}
        }
    }
}
void App::browser_init() {
    for(int i=0;i<bookcount;++i) {
        Button* b=new Button();b->Init(ts);b->Move(8,60+(i%APP_BROWSER_BUTTON_COUNT)*40);
        b->Resize(176,38);b->SetIcon(BOOK);
        b->Label(strlen(books[i]->GetTitle())?books[i]->GetTitle():books[i]->GetFileName());
        if(books[i]->GetAuthor()) b->SetLabel2(*books[i]->GetAuthor());
        buttons.push_back(b);
    }
    buttonprev.Init(ts);buttonprev.Move(8,226);buttonprev.Resize(48,28);buttonprev.Label("Ant.");
    buttonnext.Init(ts);buttonnext.Move(136,226);buttonnext.Resize(48,28);buttonnext.Label("Prox.");
    buttonprefs.Init(ts);buttonprefs.Move(60,226);buttonprefs.Resize(72,28);buttonprefs.Label("Ajustes");
    if(!bookselected && !books.empty()) bookselected=books[0];
    int index=GetBookIndex(bookselected);
    browserstart=index<0?0:(index/APP_BROWSER_BUTTON_COUNT)*APP_BROWSER_BUTTON_COUNT;
}
void App::browser_nextpage() {
    if(browserstart+APP_BROWSER_BUTTON_COUNT<bookcount) {
        browserstart+=APP_BROWSER_BUTTON_COUNT;bookselected=books[browserstart];
    }
}
void App::browser_prevpage() {
    if(browserstart>=APP_BROWSER_BUTTON_COUNT) {
        browserstart-=APP_BROWSER_BUTTON_COUNT;bookselected=books[browserstart];
    }
}
void App::browser_draw() {
    MenuScope scope(ts,ts->screenright,readingTheme);
    char count[48];snprintf(count,sizeof(count),"%d livros  /  sua biblioteca",bookcount);
    UiHeader(ts,"Biblioteca",count,readingTheme);
    for(int i=browserstart;i<bookcount && i<browserstart+APP_BROWSER_BUTTON_COUNT;++i)
        buttons[i]->Draw(ts->screenright,books[i]==bookselected);
    if(!bookcount) {
        UiText(ts,16,108,"Sua biblioteca comeca aqui.",164,11,ts->inkColor);
        UiText(ts,16,130,"Adicione EPUBs a pasta Book.",164,10,ts->inkColor);
    }
    if(browserstart>=APP_BROWSER_BUTTON_COUNT) buttonprev.Draw(ts->screenright,false);
    if(browserstart+APP_BROWSER_BUTTON_COUNT<bookcount) buttonnext.Draw(ts->screenright,false);
    buttonprefs.Label("Ajustes");buttonprefs.Draw(ts->screenright,false);
    DrawCompanion();
}
void App::browser_redraw() {browser_draw();}
void App::AttemptBookOpen() {
    if(!OpenBook()) mode=APP_MODE_BOOK;
    else {browser_draw();PrintStatus(bookcount?"Nao foi possivel abrir o livro.":"Adicione EPUBs a pasta Book.");}
}
