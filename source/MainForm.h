#pragma once
#include "headers.h"

class MainForm : public wxFrame {
public:
	MainForm();
	~MainForm() override;

private:
	enum class Mode { Idle, Training, Verification, TrialVerification };
	enum class PulseType { Success, Error };

	void buildUI();
	void loggedState();
	void loggedoutState();
	void textboxEnable();
	void textboxDisable();

	void loadDB();
	void saveDB();
	void saveBioprint();

	void onKeyDown(wxKeyEvent& event);
	void onKeyUp(wxKeyEvent& event);
	void onEnter(wxCommandEvent& event);
	void OnBlinkTimer(wxTimerEvent& event);

	void startTraining(wxCommandEvent& event);
	void startTrialVerification(wxCommandEvent& event);
	void registerUser(wxCommandEvent& event);
	void login(wxCommandEvent& event);
	void logout(wxCommandEvent& event);
	void processAttempt();

	bool verifyBio();
	bool checkSample(const std::vector<double>& sample, const std::vector<double>& ethalon);
	std::vector<double> filterErrors(const std::vector<double>& data);
	double calculateVariance(const std::vector<double>& data);

	void BackgroundPulse(PulseType type);

	static constexpr unsigned short WIDGET_WIDTH = 480;
	static constexpr unsigned short WIDGET_HEIGHT = 320;
	static constexpr int blinkIntervalMs = 30;
	static constexpr int blinkDurationMs = 1000;

	wxAuiManager auiManager;
	wxTimer blinkTimer{ this, 3000 };
	wxColour origBgColor;
	std::chrono::steady_clock::time_point blinkStart;

	nlohmann::json db;
	std::filesystem::path dataDir;
	std::filesystem::path DBdir;

	std::string currUser;
	std::string currPassword;
	unsigned char currAttempts;
	double userAlpha;

	Mode currentMode{ Mode::Idle };
	PulseType currentPulseType{ PulseType::Success };
	int attempts{ 0 };

	std::map<int, std::chrono::high_resolution_clock::time_point> pressTime;
	std::vector<double> currHolds;
	std::vector<double> currIntervals;
	std::chrono::high_resolution_clock::time_point lastRelease;
	std::vector<std::vector<double>> holdTimes;
	std::vector<std::vector<double>> intervalTimes;

	wxPanel* mainPanel{ nullptr };
	wxMenu* optionsMenu{ nullptr };
	wxMenu* attemptsMenu{ nullptr };
	wxTextCtrl* textBox{ nullptr };
	wxButton* buttonEnter{ nullptr };
	wxButton* buttonTrain{ nullptr };
	wxButton* buttonVerification{ nullptr };
	wxButton* buttonRegister{ nullptr };
	wxButton* buttonLogout{ nullptr };
	wxButton* buttonLogin{ nullptr };
	wxStaticText* usernameLabel{ nullptr };

	wxDECLARE_EVENT_TABLE();
};


