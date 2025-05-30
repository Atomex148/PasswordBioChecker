#include "LoginForm.h"

wxBEGIN_EVENT_TABLE(LoginForm, wxDialog)
    EVT_BUTTON(wxID_OK, LoginForm::OnOK)
    EVT_BUTTON(wxID_CANCEL, LoginForm::OnCancel)
    EVT_TEXT_ENTER(wxID_ANY, LoginForm::OnTextEnter)
wxEND_EVENT_TABLE()

LoginForm::LoginForm(wxWindow* parent) :
    wxDialog(parent, wxID_ANY, "Login", wxDefaultPosition, wxSize(300, 150))
{
    wxStaticText* usernameLabel = new wxStaticText(this, wxID_ANY, "Username:");
    usernameTextBox = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    usernameTextBox->SetMaxLength(16);

    wxButton* buttonOK = new wxButton(this, wxID_OK, "OK");
    wxButton* buttonCancel = new wxButton(this, wxID_CANCEL, "Cancel");

    wxBoxSizer* formSizer = new wxBoxSizer(wxVERTICAL);
    formSizer->Add(usernameLabel, 0, wxLEFT | wxTOP, 10);
    formSizer->Add(usernameTextBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(buttonOK, 0, wxALL, 5);
    buttonSizer->Add(buttonCancel, 0, wxALL, 5);

    formSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
    SetSizerAndFit(formSizer);
}

void LoginForm::OnOK(wxCommandEvent&) {
    if (usernameTextBox->IsEmpty()) {
        wxMessageBox("Please, enter your login", "Error");
        return;
    }
    else if (usernameTextBox->GetValue().Length() < 4) {
        wxMessageBox("The login must be between 4 and 16 characters long", "Error");
        return;
    }

    username = usernameTextBox->GetValue();
    EndModal(wxID_OK);
}

void LoginForm::OnCancel(wxCommandEvent&) {
    EndModal(wxID_CANCEL);
}

void LoginForm::OnTextEnter(wxCommandEvent& event) {
    OnOK(event);
}