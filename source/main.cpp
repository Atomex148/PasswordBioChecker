#include "MainForm.h"

class MyApp : public wxApp {
public:
    bool OnInit() override {
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        MainForm* frame = new MainForm();
        frame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);
