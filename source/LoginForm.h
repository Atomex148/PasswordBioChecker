#pragma once
#include "headers.h"

class LoginForm : public wxDialog {
private:
	std::string username = "";

	wxTextCtrl* usernameTextBox;

	void OnOK(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);
	void OnTextEnter(wxCommandEvent& event);

	wxDECLARE_EVENT_TABLE();
public:
	LoginForm(wxWindow* parent);

	std::string getUsername() const { return username; }
};

