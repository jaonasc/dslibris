#include "app.h"
#include "ui.h"
#include <cstdio>
using namespace ReaderVisual;
namespace {
const int order[2][4]={{PREFS_BUTTON_THEME,PREFS_BUTTON_FONTSIZE,PREFS_BUTTON_PARASPACING,PREFS_BUTTON_ANIMATION},
                       {PREFS_BUTTON_FONT,PREFS_BUTTON_FONT_BOLD,PREFS_BUTTON_FONT_ITALIC,PREFS_BUTTON_REOPEN}};
}
void App::PrefsInit() {
    Button* all[PREFS_BUTTON_COUNT]={&prefsButtonParaspacing,&prefsButtonFontSize,&prefsButtonFont,
        &prefsButtonFontItalic,&prefsButtonFontBold,&prefsButtonTheme,&prefsButtonAnimation,&prefsButtonReopen};
    for(int i=0;i<PREFS_BUTTON_COUNT;++i) {prefsButtons[i]=all[i];all[i]->Init(ts);all[i]->Resize(176,38);}
    prefsSelected=PREFS_BUTTON_THEME;prefsPage=0;
}
void App::PrefsRefreshButtonFont() {
    prefsButtonFont.Label("Fonte do texto");prefsButtonFont.SetLabel2(ts->GetFontFile(TEXT_STYLE_REGULAR));prefsButtonFont.SetIcon(TYPE);
}
void App::PrefsRefreshButtonFontBold() {
    prefsButtonFontBold.Label("Fonte em negrito");prefsButtonFontBold.SetLabel2(ts->GetFontFile(TEXT_STYLE_BOLD));prefsButtonFontBold.SetIcon(TYPE);
}
void App::PrefsRefreshButtonFontItalic() {
    prefsButtonFontItalic.Label("Fonte em italico");prefsButtonFontItalic.SetLabel2(ts->GetFontFile(TEXT_STYLE_ITALIC));prefsButtonFontItalic.SetIcon(TYPE);
}
void App::PrefsRefreshButtonFontSize() {
    char value[50];snprintf(value,sizeof(value),"<  %d px  >",ts->pixelsize);
    prefsButtonFontSize.Label("Tamanho do texto");prefsButtonFontSize.SetLabel2(value);prefsButtonFontSize.SetIcon(TYPE);
}
void App::PrefsRefreshButtonParaspacing() {
    char value[50];snprintf(value,sizeof(value),"<  %d  >",paraspacing);
    prefsButtonParaspacing.Label("Entre paragrafos");prefsButtonParaspacing.SetLabel2(value);prefsButtonParaspacing.SetIcon(SPACING);
}
void App::PrefsDraw() {PrefsDraw(true);}
void App::PrefsDraw(bool) {
    MenuScope scope(ts,ts->screenright,readingTheme);
    PrefsRefreshButtonFont();PrefsRefreshButtonFontBold();PrefsRefreshButtonFontItalic();
    PrefsRefreshButtonFontSize();PrefsRefreshButtonParaspacing();
    prefsButtonTheme.Label("Tema de leitura");prefsButtonTheme.SetLabel2(ThemeName(readingTheme));prefsButtonTheme.SetIcon(SUN);
    prefsButtonAnimation.Label("Virada de folha");prefsButtonAnimation.SetLabel2(pageAnimation?"Animada  /  toque para mudar":"Instantanea  /  toque para mudar");prefsButtonAnimation.SetIcon(SHEET);
    prefsButtonReopen.Label("Retomar leitura");prefsButtonReopen.SetLabel2(reopen?"Abrir ultimo livro ao iniciar":"Mostrar biblioteca ao iniciar");prefsButtonReopen.SetIcon(BOOK);
    UiHeader(ts,"Ajustes",prefsPage?"02 / Fontes e inicializacao":"01 / Conforto de leitura",readingTheme);
    for(int row=0;row<4;++row) {
        int i=order[prefsPage][row];prefsButtons[i]->Move(8,60+row*40);
        prefsButtons[i]->Draw(ts->screenright,i==prefsSelected);
    }
    buttonprev.Label("Ant.");buttonnext.Label("Prox.");buttonprefs.Label("Voltar");
    if(prefsPage) buttonprev.Draw(ts->screenright,false);else buttonnext.Draw(ts->screenright,false);
    buttonprefs.Draw(ts->screenright,false);
    DrawCompanion();
}
void App::HandleEventInPrefs() {
    int keys=keysDown(),row=0;
    while(row<3 && order[prefsPage][row]!=prefsSelected) ++row;
    if(keys&(KEY_START|KEY_SELECT|KEY_B)) {mode=APP_MODE_BROWSER;browser_draw();return;}
    if(keys&KEY_Y) {CycleBrightness();prefs->Write();return;}
    if(keys&KEY_TOUCH) {
        touchPosition touch;touchRead(&touch);
        int x=orientation?191-touch.py:touch.py,y=orientation?touch.px:255-touch.px;
        if(buttonprefs.EnclosesPoint(x,y)) {mode=APP_MODE_BROWSER;browser_draw();return;}
        if((!prefsPage && buttonnext.EnclosesPoint(x,y)) || (prefsPage && buttonprev.EnclosesPoint(x,y))) {
            prefsPage=1-prefsPage;prefsSelected=order[prefsPage][0];PrefsDraw();return;
        }
        for(int r=0;r<4;++r) if(prefsButtons[order[prefsPage][r]]->EnclosesPoint(x,y)) {
            prefsSelected=order[prefsPage][r];
            if(prefsSelected==PREFS_BUTTON_FONTSIZE) {if(x<96) PrefsDecreasePixelSize();else PrefsIncreasePixelSize();}
            else if(prefsSelected==PREFS_BUTTON_PARASPACING) {if(x<96) PrefsDecreaseParaspacing();else PrefsIncreaseParaspacing();}
            else PrefsButton();
            return;
        }
    } else if(keys&(key.right|key.r)) {
        if(row>0) prefsSelected=order[prefsPage][row-1];
        else {prefsPage=1-prefsPage;prefsSelected=order[prefsPage][3];}
        PrefsDraw();
    } else if(keys&(key.left|key.l)) {
        if(row<3) prefsSelected=order[prefsPage][row+1];
        else {prefsPage=1-prefsPage;prefsSelected=order[prefsPage][0];}
        PrefsDraw();
    } else if(keys&KEY_A) PrefsButton();
    else if(keys&(key.up|key.down)) {
        bool increase=(keys&key.down)!=0;
        if(prefsSelected==PREFS_BUTTON_FONTSIZE) {if(increase) PrefsIncreasePixelSize();else PrefsDecreasePixelSize();}
        else if(prefsSelected==PREFS_BUTTON_PARASPACING) {if(increase) PrefsIncreaseParaspacing();else PrefsDecreaseParaspacing();}
        else PrefsButton();
    }
}
void App::PrefsIncreasePixelSize() {
    if(ts->pixelsize<24) {++ts->pixelsize;prefs->Write();PrefsDraw();}
}
void App::PrefsDecreasePixelSize() {
    if(ts->pixelsize>8) {--ts->pixelsize;prefs->Write();PrefsDraw();}
}
void App::PrefsIncreaseParaspacing() {
    if(paraspacing<12) {++paraspacing;prefs->Write();PrefsDraw();}
}
void App::PrefsDecreaseParaspacing() {
    if(paraspacing>0) {--paraspacing;prefs->Write();PrefsDraw();}
}
void App::PrefsButton() {
    if(prefsSelected==PREFS_BUTTON_THEME) {readingTheme=(readingTheme+1)%THEME_COUNT;ApplyTheme();prefs->Write();PrefsDraw();return;}
    if(prefsSelected==PREFS_BUTTON_ANIMATION) {pageAnimation=!pageAnimation;prefs->Write();PrefsDraw();return;}
    if(prefsSelected==PREFS_BUTTON_REOPEN) {reopen=!reopen;prefs->Write();PrefsDraw();return;}
    if(prefsSelected==PREFS_BUTTON_FONTSIZE) {PrefsIncreasePixelSize();return;}
    if(prefsSelected==PREFS_BUTTON_PARASPACING) {PrefsIncreaseParaspacing();return;}
    if(prefsSelected==PREFS_BUTTON_FONT) mode=APP_MODE_PREFS_FONT;
    else if(prefsSelected==PREFS_BUTTON_FONT_BOLD) mode=APP_MODE_PREFS_FONT_BOLD;
    else if(prefsSelected==PREFS_BUTTON_FONT_ITALIC) mode=APP_MODE_PREFS_FONT_ITALIC;
    else return;
    FontInit();FontDraw();
}
