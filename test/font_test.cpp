#include "app.h"
#include <cassert>
#include <iostream>
extern App* app;
int main() {
    App application;app=&application;
    app->ts->SetFontFile("missing-custom-font.ttf",TEXT_STYLE_REGULAR);
    app->ts->SetFontFile("also-missing.ttf",TEXT_STYLE_ITALIC);
    assert(app->ts->Init()==0);
    assert(app->ts->GetHeight()>0);
    assert(!app->ts->SelectFont("invalid.ttf",TEXT_STYLE_REGULAR));
    assert(app->ts->GetHeight()>0);
    assert(app->ts->SelectFont("LiberationSans-Regular.ttf",TEXT_STYLE_REGULAR));
    assert(app->ts->GetHeight()>0);
    std::cout<<"PASS: missing-font fallback, invalid replacement, valid replacement, resource destruction"<<std::endl;
}
