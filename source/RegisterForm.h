#pragma once
#include "headers.h"

class RegisterForm : public wxDialog {
private:
	std::string username = "";
	std::string password = "";

	wxTextCtrl* usernameTextBox;
	wxTextCtrl* passwordTextBox;

	void OnOK(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);
	void OnTextEnter(wxCommandEvent& event);

	wxDECLARE_EVENT_TABLE();
public:
	RegisterForm(wxWindow* parent);

	std::string getUsername() const { return username; }
	std::string getPassword() const { return password; }
};

