#include "RegisterForm.h"

wxBEGIN_EVENT_TABLE(RegisterForm, wxDialog)
    EVT_BUTTON(wxID_OK, RegisterForm::OnOK)
    EVT_BUTTON(wxID_CANCEL, RegisterForm::OnCancel)
    EVT_TEXT_ENTER(wxID_ANY, RegisterForm::OnTextEnter)
wxEND_EVENT_TABLE()

RegisterForm::RegisterForm(wxWindow* parent) :
	wxDialog(parent, wxID_ANY, "Register", wxDefaultPosition, wxSize(300, 200))
{
    wxStaticText* usernameLabel = new wxStaticText(this, wxID_ANY, "Username (4-16 symbols):");
    usernameTextBox = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    usernameTextBox->SetMaxLength(16);

    wxStaticText* passwordLabel = new wxStaticText(this, wxID_ANY, "Password (4-16 symbols):");
    passwordTextBox = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    passwordTextBox->SetMaxLength(16);

    wxButton* buttonOK = new wxButton(this, wxID_OK, "OK");
    wxButton* buttonCancel = new wxButton(this, wxID_CANCEL, "Cancel");

    wxBoxSizer* formSizer = new wxBoxSizer(wxVERTICAL);
    formSizer->Add(usernameLabel, 0, wxLEFT | wxTOP, 10);
    formSizer->Add(usernameTextBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    formSizer->Add(passwordLabel, 0, wxLEFT, 10);
    formSizer->Add(passwordTextBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(buttonOK, 0, wxALL, 5);
    buttonSizer->Add(buttonCancel, 0, wxALL, 5);

    formSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
    SetSizerAndFit(formSizer);
}

void RegisterForm::OnOK(wxCommandEvent&) {
    if (usernameTextBox->IsEmpty() || passwordTextBox->IsEmpty()) {
        wxMessageBox("Please, enter your login and password", "Error");
        return;
    } 
    else if (usernameTextBox->GetValue().Length() < 4 || passwordTextBox->GetValue().Length() < 4) {
        wxMessageBox("The login and password must be between 4 and 16 characters long", "Error");
        return;
    }

    username = usernameTextBox->GetValue();
    password = passwordTextBox->GetValue();
    EndModal(wxID_OK);
}

void RegisterForm::OnCancel(wxCommandEvent&) {
    EndModal(wxID_CANCEL);
}

void RegisterForm::OnTextEnter(wxCommandEvent& event) {
    OnOK(event);
}