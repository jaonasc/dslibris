#include "app.h"
#include "ui.h"
#include <dirent.h>
#include <cstring>
#include <algorithm>
using namespace ReaderVisual;
namespace {const unsigned int BPP=4;}
void App::FontInit() {
    FontDeinit();fontPage=fontSelected=0;
    DIR* dp=opendir(fontdir.c_str());if(!dp) return;
    dirent* entry;
    while((entry=readdir(dp))) {
        const char* name=entry->d_name;const char* ext=strrchr(name,'.');
        if(entry->d_type==DT_DIR || !ext) continue;
        if(strcasecmp(ext,".ttf") && strcasecmp(ext,".otf") && strcasecmp(ext,".ttc")) continue;
        Button* b=new Button();b->Init(ts);b->Resize(176,38);
        b->Move(8,60+(fontButtons.size()%BPP)*40);b->Label(name);b->SetIcon(TYPE);
        fontButtons.push_back(b);
    }
    closedir(dp);
}
void App::HandleEventInFont() {
    int keys=keysDown();
    if(keys&(KEY_START|KEY_SELECT|KEY_B)) {FontDeinit();mode=APP_MODE_PREFS;PrefsDraw();}
    else if(keys&(key.right|key.r)) {
        if(fontSelected>0) {--fontSelected;fontPage=fontSelected/BPP;FontDraw();}
    } else if(keys&(key.left|key.l)) {
        if(fontSelected+1<fontButtons.size()) {++fontSelected;fontPage=fontSelected/BPP;FontDraw();}
    } else if(keys&KEY_A) FontButton();
    else if(keys&KEY_TOUCH) {
        touchPosition touch;touchRead(&touch);
        int x=orientation?191-touch.py:touch.py,y=orientation?touch.px:255-touch.px;
        if(buttonprefs.EnclosesPoint(x,y)) {FontDeinit();mode=APP_MODE_PREFS;PrefsDraw();}
        else if(buttonnext.EnclosesPoint(x,y)) {FontNextPage();FontDraw();}
        else if(buttonprev.EnclosesPoint(x,y)) {FontPreviousPage();FontDraw();}
        else for(unsigned int i=fontPage*BPP;i<fontButtons.size() && i<(fontPage+1)*BPP;++i) {
            if(fontButtons[i]->EnclosesPoint(x,y)) {fontSelected=i;FontButton();break;}
        }
    }
}
void App::FontDeinit() {
    for(unsigned int i=0;i<fontButtons.size();++i) delete fontButtons[i];
    fontButtons.clear();
}
void App::FontDraw() {FontDraw(true);}
void App::FontDraw(bool) {
    MenuScope scope(ts,ts->screenright,readingTheme);
    UiHeader(ts,"Escolher fonte","A / selecionar   B / voltar",readingTheme);
    for(unsigned int i=fontPage*BPP;i<fontButtons.size() && i<(fontPage+1)*BPP;++i)
        fontButtons[i]->Draw(ts->screenright,i==fontSelected);
    if(fontButtons.empty()) UiText(ts,12,110,"Nenhuma fonte em Font.",168,11,ts->inkColor);
    buttonprefs.Label("Voltar");buttonprefs.Draw(ts->screenright,false);
    if(fontPage) buttonprev.Draw(ts->screenright,false);
    if((fontPage+1)*BPP<fontButtons.size()) buttonnext.Draw(ts->screenright,false);
}
void App::FontNextPage() {
    if((fontPage+1)*BPP<fontButtons.size()) {++fontPage;fontSelected=fontPage*BPP;}
}
void App::FontPreviousPage() {if(fontPage) {--fontPage;fontSelected=fontPage*BPP;}}
void App::FontButton() {
    if(fontSelected>=fontButtons.size()) return;
    int style=mode==APP_MODE_PREFS_FONT?TEXT_STYLE_REGULAR:mode==APP_MODE_PREFS_FONT_BOLD?TEXT_STYLE_BOLD:TEXT_STYLE_ITALIC;
    // CreateFace already resolves files relative to Font.
    if(!ts->SelectFont(fontButtons[fontSelected]->GetLabel(),style)) {
        PrintStatus("Fonte invalida. Escolha outra.");return;
    }
    ApplyTheme();FontDeinit();mode=APP_MODE_PREFS;
    prefs->Write();PrefsDraw();
}
