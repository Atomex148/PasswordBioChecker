#include "MainForm.h"
#include "RegisterForm.h"
#include "LoginForm.h"

wxBEGIN_EVENT_TABLE(MainForm, wxFrame)
    EVT_CHAR_HOOK(MainForm::onKeyDown)
    EVT_CHAR(MainForm::onKeyUp)
    EVT_TEXT_ENTER(wxID_ANY, MainForm::onEnter)
    EVT_BUTTON(wxID_OK, MainForm::onEnter)
    EVT_TIMER(3000, MainForm::OnBlinkTimer)
wxEND_EVENT_TABLE()

MainForm::MainForm()
    : wxFrame(nullptr, wxID_ANY, "ZPZ_RGR",
        wxDefaultPosition, wxSize(WIDGET_WIDTH, WIDGET_HEIGHT),
        wxDEFAULT_FRAME_STYLE & ~wxRESIZE_BORDER & ~wxMAXIMIZE_BOX),
    dataDir(std::filesystem::current_path()),
    DBdir(dataDir.string().append("/db.json")),
    blinkTimer(this, 3000)
{
    loadDB();   

    auiManager.SetManagedWindow(this);
    buildUI();
    auiManager.Update();

    origBgColor = mainPanel->GetBackgroundColour();
}

void MainForm::loadDB() {
    if (std::filesystem::exists(DBdir)) {
        std::ifstream in(DBdir);
        try {
            in >> db;
        }
        catch (std::exception& ex) {
            wxMessageBox("Error while reading from DB: " + wxString(ex.what()), "Error");
            this->Close();
        }
    }
    else {
        db = nlohmann::json::object();
        db["users"] = nlohmann::json::object();
        saveDB();
    }
}

void MainForm::saveDB() {
    std::ofstream out(DBdir);
    if (out) {
        out << db.dump(2);
    }
    else {
        wxMessageBox("Error while saving DB", "Error");
    }
}

void MainForm::buildUI() {
    mainPanel = new wxPanel(this);

    usernameLabel = new wxStaticText();
    usernameLabel->SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);
    usernameLabel->Create(mainPanel, wxID_ANY, "Username:", wxPoint(20, WIDGET_HEIGHT - 100), wxDefaultSize, wxTRANSPARENT_WINDOW);

    wxMenuBar* menuBar = new wxMenuBar();
    optionsMenu = new wxMenu();

    attemptsMenu = new wxMenu();
    attemptsMenu->AppendRadioItem(1001, "5 Attempts");
    attemptsMenu->AppendRadioItem(1002, "10 Attempts");
    attemptsMenu->AppendRadioItem(1003, "15 Attempts");
    attemptsMenu->AppendRadioItem(1004, "20 Attempts");
    optionsMenu->Append(2000, "Attempts", attemptsMenu);
    Bind(wxEVT_MENU, [this](wxCommandEvent& e) {
        switch (e.GetId()) {
            case 1001: currAttempts = 5; break;
            case 1002: currAttempts = 10; break;
            case 1003: currAttempts = 15; break;
            case 1004: currAttempts = 20; break;
        }
        db["users"][currUser]["current_attempts"] = currAttempts;
        saveDB();
    }, 1001, 1004);

    optionsMenu->Append(2001, "Change alpha", "Change strictness of the biocheck", false);
    Bind(wxEVT_MENU,
        [this](wxCommandEvent& e) {
            wxTextEntryDialog dlg(
                this,
                "Enter the strictness (0.01 - 0.99, lower is less strict):",
                "Change alpha",
                wxString::Format("%.2f", userAlpha)
            );

            if (dlg.ShowModal() == wxID_OK) {
                wxString val = dlg.GetValue();
                double alpha;
                if (!val.ToDouble(&alpha) || alpha < 0.01 || alpha > 0.99) {
                    wxMessageBox("Please enter a number between 0.01 and 0.99",
                        "Invalid input", wxOK | wxICON_ERROR);
                    return;
                }

                userAlpha = alpha;
                db["users"][currUser]["user_alpha"] = userAlpha;
                saveDB();
                wxMessageBox(wxString::Format("Alpha changed to %.3f", userAlpha),
                    "Strictness has been changed");
            }
        }, 
        2001
    );
    menuBar->Append(optionsMenu, "&Options");

    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, "About", "About this application");
    menuBar->Append(helpMenu, "&Help");
    Bind(wxEVT_MENU, [](wxCommandEvent&) { wxMessageBox("Author: Artem Punko\nGroup: FB-22\n(c) 2025", "About"); }, wxID_ABOUT);

    SetMenuBar(menuBar);

    textBox = new wxTextCtrl(mainPanel, wxID_ANY, "", wxDefaultPosition, wxSize(200, 20), wxTE_PROCESS_ENTER);
    buttonEnter = new wxButton(mainPanel, wxID_OK, "Enter");
    textboxDisable();

    buttonTrain = new wxButton(mainPanel, wxID_ANY, "Training", wxPoint(10, 10), wxSize(80, 25));
    buttonVerification = new wxButton(mainPanel, wxID_ANY, "Verification", wxPoint(100, 10), wxSize(80, 25));
    buttonRegister = new wxButton(mainPanel, wxID_ANY, "Register", wxPoint(WIDGET_WIDTH - 105, 10), wxSize(80, 25));
    buttonLogout = new wxButton(mainPanel, wxID_ANY, "Logout", wxPoint(WIDGET_WIDTH - 105, WIDGET_HEIGHT - 100), wxSize(80, 25));
    buttonLogin = new wxButton(mainPanel, wxID_ANY, "Login", wxPoint(WIDGET_WIDTH - 105, WIDGET_HEIGHT - 130), wxSize(80, 25));
    loggedoutState();

    textBox->Bind(wxEVT_KEY_DOWN, &MainForm::onKeyDown, this);
    textBox->Bind(wxEVT_KEY_UP, &MainForm::onKeyUp, this);
    textBox->Bind(wxEVT_TEXT_ENTER, &MainForm::onEnter, this);

    Bind(wxEVT_BUTTON, &MainForm::onEnter, this, buttonEnter->GetId());
    Bind(wxEVT_BUTTON, &MainForm::startTraining, this, buttonTrain->GetId());
    Bind(wxEVT_BUTTON, &MainForm::startTrialVerification, this, buttonVerification->GetId());
    Bind(wxEVT_BUTTON, &MainForm::registerUser, this, buttonRegister->GetId());
    Bind(wxEVT_BUTTON, &MainForm::logout, this, buttonLogout->GetId());
    Bind(wxEVT_BUTTON, &MainForm::login, this, buttonLogin->GetId());

    wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
    row->Add(textBox, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    row->Add(buttonEnter, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->AddStretchSpacer(2);
    mainSizer->Add(row, 0, wxALIGN_CENTER_HORIZONTAL);
    mainSizer->AddStretchSpacer(3);

    mainPanel->SetSizer(mainSizer);
    mainPanel->Layout();

    auiManager.AddPane(mainPanel, wxAuiPaneInfo().CenterPane());
}

void MainForm::loggedState() {
    buttonTrain->Enable();
    buttonVerification->Enable();
    buttonLogout->Enable();
    optionsMenu->Enable(2000, true);
    optionsMenu->Enable(2001, true);
    usernameLabel->SetLabelText("Username: " + currUser);

    buttonRegister->Disable();
    buttonLogin->Disable();
}

void MainForm::loggedoutState() {
    buttonTrain->Disable();
    buttonVerification->Disable();
    buttonLogout->Disable();
    optionsMenu->Enable(2000, false);
    optionsMenu->Enable(2001, false);
    usernameLabel->SetLabelText("Username: ");

    buttonRegister->Enable();
    buttonLogin->Enable();
}

void MainForm::textboxEnable() {
    textBox->Enable();
    textBox->Clear();
    buttonEnter->Enable();

    textBox->SetFocus();
}

void MainForm::textboxDisable() {
    textBox->Disable();
    textBox->Clear();
    buttonEnter->Disable();
}

void MainForm::startTraining(wxCommandEvent& event) {
    if (currUser.empty()) { wxMessageBox("Register or login first", "Error"); return; }

    currentMode = Mode::Training;
    attempts = 0;
    holdTimes.clear();
    intervalTimes.clear();

    currHolds.clear();
    currIntervals.clear();
    pressTime.clear();
    lastRelease = {};

    textboxEnable();
    wxMessageBox(wxString::Format("Training started. Be ready to enter your password %i times", currAttempts), "Info");
}

void MainForm::startTrialVerification(wxCommandEvent& event) {
    if (currUser.empty()) { wxMessageBox("Login first", "Error"); return; }

    currentMode = Mode::TrialVerification;
    currHolds.clear();
    currIntervals.clear();
    pressTime.clear();
    lastRelease = {};

    textboxEnable();
    wxMessageBox("Trial verification started. Be ready to enter your password", "Info");
}

void MainForm::onKeyDown(wxKeyEvent& event) {
    if (currentMode == Mode::Idle) { event.Skip(); return; }

    int kc = event.GetKeyCode();
    auto now = std::chrono::high_resolution_clock::now();
    pressTime[kc] = now;

    if (lastRelease.time_since_epoch().count() > 0) {
        currIntervals.push_back(std::chrono::duration<double>(now - lastRelease).count());
    }
    event.Skip();
}

void MainForm::onKeyUp(wxKeyEvent& event) {
    if (currentMode == Mode::Idle) { event.Skip(); return; }

    int kc = event.GetKeyCode();
    auto now = std::chrono::high_resolution_clock::now();
    if (auto it = pressTime.find(kc); it != pressTime.end()) {
        currHolds.push_back(std::chrono::duration<double>(now - it->second).count());
        pressTime.erase(it);
    }
    lastRelease = now;
    event.Skip();
}

void MainForm::onEnter(wxCommandEvent& event) {
    if (currentMode == Mode::Idle) {
        return;
    }

    wxString typed = textBox->GetValue();
    textBox->Clear();
    textboxDisable();

    switch (currentMode) {
        case Mode::Training:
            if (typed.ToStdString() != currPassword) {
                wxMessageBox("Incorrect password. Please try again!", "Error");
                currHolds.clear();
                currIntervals.clear();
                pressTime.clear();
                lastRelease = {};
                textboxEnable();
                return;
            }

            processAttempt();

            if (attempts >= currAttempts) {
                saveBioprint();
                wxMessageBox("Training complete!", "Info");
                currentMode = Mode::Idle;
            }
            else {
                wxMessageBox(wxString::Format("Attempt %d/%d saved. Next attempt:", attempts, currAttempts), "Training");
                currHolds.clear();
                currIntervals.clear();
                pressTime.clear();
                lastRelease = {};
                textboxEnable();
            }
            return;
        case Mode::Verification:
            currentMode = Mode::Idle;

            if (typed.ToStdString() != currPassword) {
                BackgroundPulse(PulseType::Error);
                wxMessageBox("Incorrect password.", "Error");
                currUser.clear();
                return;
            }

            if (!verifyBio()) {
                BackgroundPulse(PulseType::Error);
                wxMessageBox("Biometric mismatch. Access denied.", "Error");
                currUser.clear();
                return;
            }

            BackgroundPulse(PulseType::Success);
            wxMessageBox("Welcome, " + wxString(currUser), "Access granted");
            loggedState();
            return;
        case Mode::TrialVerification:
            if (typed.ToStdString() != currPassword) {
                BackgroundPulse(PulseType::Error);
                wxMessageBox("Incorrect password. Verification failed!", "Error");
            }
            else {
                if (!verifyBio()) {
                    BackgroundPulse(PulseType::Error);
                    wxMessageBox("You failed verification!", "Error");
                }
                else {
                    BackgroundPulse(PulseType::Success);
                    wxMessageBox("You succeeded verification!", "Success");
                }
            }
            currentMode = Mode::Idle;
        default: return;
    }  
}

void MainForm::registerUser(wxCommandEvent& event) {
    RegisterForm form(this);

    if (form.ShowModal() == wxID_OK) {
        currUser = form.getUsername();
        currPassword = form.getPassword();
        currAttempts = 5;
        userAlpha = 0.05;

        if (db["users"].contains(currUser)) {
            wxMessageBox("User already exists!", "Error");
            currUser.clear();
            currPassword.clear();
            return;
        }

        db["users"][currUser] = {
            {"password", currPassword},
            {"samples", {
                {"holds", std::vector<std::vector<double>>()},
                {"intervals", std::vector<std::vector<double>>()}
            }},
            {"current_attempts", currAttempts},
            {"user_alpha", userAlpha}
        };

        saveDB();

        wxMessageBox("Registration completed, " + currUser + "!", "Congratulations!");
        loggedState();
    }
}

void MainForm::logout(wxCommandEvent& event) {
    currUser.clear();
    currPassword.clear();
    attempts = 0;
    holdTimes.clear();
    intervalTimes.clear();
    currHolds.clear();
    currIntervals.clear();

    loggedoutState();
}

void MainForm::login(wxCommandEvent& event) {
    LoginForm form(this);

    if (form.ShowModal() == wxID_OK) {
        std::string username = form.getUsername();
        if (!db["users"].contains(username)) {
            wxMessageBox("This user does not exist", "Error");
            return;
        }

        currUser = username;
        currPassword = db["users"][currUser]["password"];

        if (db["users"][currUser].contains("current_attempts"))
            currAttempts = db["users"][currUser]["current_attempts"];
        else
            currAttempts = 5;

        int idToCheck = 1001;
        switch (currAttempts) {
        case 10:
            idToCheck = 1002;
            break;
        case 15:
            idToCheck = 1003;
            break;
        case 20:
            idToCheck = 1004;
        default:
            break; 
        }

        attemptsMenu->Check(1001, idToCheck == 1001);
        attemptsMenu->Check(1002, idToCheck == 1002);
        attemptsMenu->Check(1003, idToCheck == 1003);
        attemptsMenu->Check(1004, idToCheck == 1004);

        if (db["users"][currUser].contains("user_alpha"))
            userAlpha = db["users"][currUser]["user_alpha"];
        else
            userAlpha = 0.05;

        wxMessageBox("Now please enter your password in the field below to verify biometric profile", "You almost done!");
        currentMode = Mode::Verification;

        currHolds.clear();
        currIntervals.clear();
        pressTime.clear();
        lastRelease = {};

        textboxEnable();
    }
}

bool MainForm::verifyBio() {
    if (!db["users"].contains(currUser)) return false;

    const auto& user = db["users"][currUser];
    const auto& samples = user["samples"];

    if (!samples.contains("holds") || samples["holds"].empty()) {
        wxMessageBox("You logged without biometrical verification because it is empty. Please, complete training", "ALERT!!!");
        return true;
    }

    std::vector<double> intervals;
    for (const auto& sample : samples["intervals"]) {
        for (const auto& value : sample) {
            intervals.push_back(value.get<double>());
        }
    }

    std::vector<double> holds;
    for (const auto& sample : samples["holds"]) {
        for (const auto& value : sample) {
            holds.push_back(value.get<double>());
        }
    }
   
    if (!checkSample(intervals, currIntervals) || !checkSample(holds, currHolds)) return false;

    return true;
}

bool MainForm::checkSample(const std::vector<double>& sample, const std::vector<double>& ethalon) {
    auto filteredEthalon = filterErrors(ethalon);

    if (sample.size() < 2 || filteredEthalon.size() < 2) {
        wxMessageBox("Not enough data points to perform statistical tests. Skipping check.", "Warning");
        return true;
    }

    double s1 = calculateVariance(sample);
    double s2 = calculateVariance(filteredEthalon);

    double Fp = (s1 < s2 ? s2 / s1 : s1 / s2);
    double Ft = 0;
    double pF = 1.0 - userAlpha;
    if (pF <= 0.0 || pF >= 1.0) pF = 0.95;

    try {
        boost::math::fisher_f_distribution<> fDist(sample.size() - 1, filteredEthalon.size() - 1);
        Ft = boost::math::quantile(fDist, pF);
    }
    catch (const std::domain_error& ex) {
        wxMessageBox("F-test skipped: " + wxString(ex.what()), "F-Test Error", wxICON_ERROR);
        return true;
    }

    wxMessageBox(
        wxString::Format("F_p = %f.5\nF_T = %f.5\nVerdict is %s", Fp, Ft, (Fp > Ft) ? "NEGATIVE" : "POSITIVE"),
        "F-Test"
    );
    if (Fp > Ft) return false;

    double My = std::accumulate(filteredEthalon.begin(), filteredEthalon.end(), 0.0) / filteredEthalon.size();
    double Mx = std::accumulate(sample.begin(), sample.end(), 0.0) / sample.size();

    double S2y = 0;
    for (double v : filteredEthalon) S2y += std::pow((v - My), 2);
    S2y /= (filteredEthalon.size() - 1);

    double S2x = 0;
    for (double v : sample) S2x += std::pow((v - Mx), 2);
    S2x /= (sample.size() - 1);

    double pooledVar = ((filteredEthalon.size() - 1) * S2y + (sample.size() - 1) * S2x)
        / (filteredEthalon.size() + sample.size() - 2);
    double Sp = std::sqrt(pooledVar);
    double tStat = (Mx - My) / (Sp * std::sqrt(1.0 / filteredEthalon.size() + 1.0 / sample.size()));

    std::size_t dfT = filteredEthalon.size() + sample.size() - 2;
    double tCrit = 0;
    double pT = 1.0 - userAlpha;
    if (pT <= 0.0 || pT >= 1.0) pT = 0.975;

    try {
        boost::math::students_t tDist(dfT);
        tCrit = boost::math::quantile(tDist, pT);
    }
    catch (const std::domain_error& ex) {
        wxMessageBox("T-test skipped: " + wxString(ex.what()), "T-Test Error", wxICON_ERROR);
        return true;
    }

    wxMessageBox(
        wxString::Format("t_stat = %f.5\ntCritical = %f.5\nVerdict is %s", std::abs(tStat), tCrit,
            (std::abs(tStat) > tCrit) ? "NEGATIVE" : "POSITIVE"),
        "T-Test"
    );

    if (std::abs(tStat) > tCrit) return false;

    return true;
}

void MainForm::processAttempt() {
    auto fh = filterErrors(currHolds);
    auto fi = filterErrors(currIntervals);

    holdTimes.push_back(fh);
    intervalTimes.push_back(fi);
    attempts++;

    currHolds.clear();
    currIntervals.clear();
}

std::vector<double> MainForm::filterErrors(const std::vector<double>& data) {
    std::vector<double> filtered(data);

    if (filtered.size() < 3)
        return filtered;

    bool removed;
    do {
        removed = false;

        if (filtered.size() < 3)
            break;

        for (int i = 0; i < static_cast<int>(filtered.size()); ++i) {
            if (filtered.size() - 1 < 2)
                continue;

            double sum = std::accumulate(filtered.begin(), filtered.end(), 0.0) - filtered[i];
            double mI = sum / (filtered.size() - 1);

            double sI2 = 0.0;
            for (int j = 0; j < static_cast<int>(filtered.size()); ++j) {
                if (j == i) continue;
                sI2 += std::pow(filtered[j] - mI, 2);
            }

            if (filtered.size() - 2 <= 0)
                continue;
            sI2 /= (filtered.size() - 2);

            double sI = std::sqrt(sI2);
            if (sI == 0.0)
                continue;

            boost::math::students_t studCoef(filtered.size() - 2);
            double tT = boost::math::quantile(studCoef, 0.975);

            double tP = std::abs((filtered[i] - mI) / sI);
            if (tP > tT) {
                filtered.erase(filtered.begin() + i);
                removed = true;
                break;
            }
        }
    } while (removed);

    return filtered;
}

void MainForm::saveBioprint() {
    auto& userData = db["users"][currUser];
    userData["password"] = currPassword;
    userData["samples"] = {
        { "holds", holdTimes },
        { "intervals", intervalTimes }
    };
    userData["current_attempts"] = currAttempts;
    userData["user_alpha"] = currAttempts;

    saveDB();
}

void MainForm::BackgroundPulse(PulseType type) {
    currentPulseType = type;
    blinkStart = std::chrono::steady_clock::now();
    blinkTimer.Start(blinkIntervalMs);
}

void MainForm::OnBlinkTimer(wxTimerEvent&) {
    using namespace std::chrono;
    auto now = steady_clock::now();
    auto delta = duration_cast<milliseconds>(now - blinkStart).count();

    if (delta >= blinkDurationMs) {
        blinkTimer.Stop();
        mainPanel->SetBackgroundColour(origBgColor);
        mainPanel->Refresh();
        return;
    }

    double t = double(delta) / blinkDurationMs;
    double intensity = (t < 0.5) ? (t * 2.0) : ((1.0 - t) * 2.0);

    int r0 = origBgColor.Red();
    int g0 = origBgColor.Green();
    int b0 = origBgColor.Blue();

    int rt, gt, bt;

    if (currentPulseType == PulseType::Success) {
        rt = 80;
        gt = 180;
        bt = 80;
    }
    else {
        rt = 180;
        gt = 80;
        bt = 80;
    }

    int r = int(r0 * (1 - intensity) + rt * intensity);
    int g = int(g0 * (1 - intensity) + gt * intensity);
    int b = int(b0 * (1 - intensity) + bt * intensity);

    mainPanel->SetBackgroundColour(wxColour(r, g, b));
    mainPanel->Refresh();
}

double MainForm::calculateVariance(const std::vector<double>& data) {
    if (data.empty() || data.size() == 1) return 0.0;

    double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();

    double sumSquaredDiffs = 0.0;
    for (const auto& x : data) {
        sumSquaredDiffs += std::pow((x - mean), 2);
    }
    return sumSquaredDiffs / (data.size() - 1);
}

MainForm::~MainForm() {
    saveDB();
    auiManager.UnInit();
}