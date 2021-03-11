///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "otidalplanUIDialogBase.h"
#include "otidalplanUIDialog.h"
#include "otidalplan_pi.h"

///////////////////////////////////////////////////////////////////////////

otidalplanUIDialogBase::otidalplanUIDialogBase(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxFrame(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Route Calculation")), wxHORIZONTAL);

	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer(new wxStaticBox(sbSizer4->GetStaticBox(), wxID_ANY, _("Speed")), wxVERTICAL);

	m_tSpeed = new wxTextCtrl(sbSizer3->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sbSizer3->Add(m_tSpeed, 0, wxALL, 5);

	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer(new wxStaticBox(sbSizer3->GetStaticBox(), wxID_ANY, _("Date/Time")), wxVERTICAL);

	m_staticText3 = new wxStaticText(sbSizer2->GetStaticBox(), wxID_ANY, _("First departure time (UTC)"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText3->Wrap(-1);
	sbSizer2->Add(m_staticText3, 0, wxALL, 5);

	m_textCtrl1 = new wxTextCtrl(sbSizer2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sbSizer2->Add(m_textCtrl1, 0, wxALL, 5);

	wxStaticBoxSizer* sbSizer91;
	sbSizer91 = new wxStaticBoxSizer(new wxStaticBox(sbSizer2->GetStaticBox(), wxID_ANY, _("Route")), wxVERTICAL);

	m_staticText4 = new wxStaticText(sbSizer91->GetStaticBox(), wxID_ANY, _("DR/ETA Route Name"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText4->Wrap(-1);
	sbSizer91->Add(m_staticText4, 0, wxALL, 5);

	m_tRouteName = new wxTextCtrl(sbSizer91->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sbSizer91->Add(m_tRouteName, 0, wxALL, 5);

	m_cbAddRoute = new wxCheckBox(sbSizer91->GetStaticBox(), wxID_ANY, _("Add route and display "), wxDefaultPosition, wxDefaultSize, 0);
	sbSizer91->Add(m_cbAddRoute, 0, wxALL, 5);

	m_cbGPX = new wxCheckBox(sbSizer91->GetStaticBox(), wxID_ANY, _("Save as GPX file"), wxDefaultPosition, wxDefaultSize, 0);
	sbSizer91->Add(m_cbGPX, 0, wxALL, 5);

	m_staticText2 = new wxStaticText(sbSizer91->GetStaticBox(), wxID_ANY, _("Departure Times"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText2->Wrap(-1);
	sbSizer91->Add(m_staticText2, 0, wxALL, 5);

	wxString m_choiceDepartureTimesChoices[] = { _("1"), _("2"), _("3"), _("4"), _("5"), _("6") };
	int m_choiceDepartureTimesNChoices = sizeof(m_choiceDepartureTimesChoices) / sizeof(wxString);
	m_choiceDepartureTimes = new wxChoice(sbSizer91->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choiceDepartureTimesNChoices, m_choiceDepartureTimesChoices, 0);
	m_choiceDepartureTimes->SetSelection(0);
	sbSizer91->Add(m_choiceDepartureTimes, 0, wxALL, 5);

	m_bCalcDR = new wxButton(sbSizer91->GetStaticBox(), wxID_ANY, _("Calculate DR"), wxDefaultPosition, wxDefaultSize, 0);
	sbSizer91->Add(m_bCalcDR, 0, wxALL | wxEXPAND, 5);

	m_staticline1 = new wxStaticLine(sbSizer91->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	sbSizer91->Add(m_staticline1, 0, wxEXPAND | wxALL, 5);

	m_bCalcETA = new wxButton(sbSizer91->GetStaticBox(), wxID_ANY, _("Calculate ETA"), wxDefaultPosition, wxDefaultSize, 0);
	sbSizer91->Add(m_bCalcETA, 0, wxALL | wxEXPAND, 5);


	sbSizer2->Add(sbSizer91, 1, wxEXPAND, 5);


	sbSizer3->Add(sbSizer2, 1, wxEXPAND, 5);


	sbSizer4->Add(sbSizer3, 1, wxEXPAND, 5);

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	fgSizer2->SetFlexibleDirection(wxBOTH);
	fgSizer2->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_staticText41 = new wxStaticText(sbSizer4->GetStaticBox(), wxID_ANY, _("Max Dist from leg"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText41->Wrap(-1);
	fgSizer2->Add(m_staticText41, 0, wxALL, 5);

	wxString m_cMaxDistChoices[] = { _("3"), _("5"), _("10") };
	int m_cMaxDistNChoices = sizeof(m_cMaxDistChoices) / sizeof(wxString);
	m_cMaxDist = new wxChoice(sbSizer4->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cMaxDistNChoices, m_cMaxDistChoices, 0);
	m_cMaxDist->SetSelection(0);
	fgSizer2->Add(m_cMaxDist, 0, wxALL, 5);

	m_stText = new wxStaticText(sbSizer4->GetStaticBox(), wxID_ANY, _("Leg Distance"), wxDefaultPosition, wxDefaultSize, 0);
	m_stText->Wrap(-1);
	fgSizer2->Add(m_stText, 0, wxALL, 5);

	wxString m_cLegDistChoices[] = { _("2"), _("3"), _("5"), _("10") };
	int m_cLegDistNChoices = sizeof(m_cLegDistChoices) / sizeof(wxString);
	m_cLegDist = new wxChoice(sbSizer4->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cLegDistNChoices, m_cLegDistChoices, 0);
	m_cLegDist->SetSelection(0);
	fgSizer2->Add(m_cLegDist, 0, wxALL, 5);

	m_cbAttach = new wxCheckBox(sbSizer4->GetStaticBox(), wxID_ANY, _("Show attachments"), wxDefaultPosition, wxDefaultSize, 0);
	m_cbAttach->SetValue(true);
	fgSizer2->Add(m_cbAttach, 0, wxALL, 5);


	fgSizer2->Add(0, 0, 1, wxEXPAND, 5);

	m_buttonAttach = new wxButton(sbSizer4->GetStaticBox(), wxID_ANY, _("Attach Currents"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer2->Add(m_buttonAttach, 0, wxALL, 5);

	bSizer1->Add(fgSizer2, 1, wxEXPAND, 5);

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer* sbSizer61;
	sbSizer61 = new wxStaticBoxSizer(new wxStaticBox(sbSizer4->GetStaticBox(), wxID_ANY, wxEmptyString), wxVERTICAL);

	m_cbPlannedSpeed = new wxCheckBox(sbSizer61->GetStaticBox(), wxID_ANY, _("Use Planned Speeds"), wxDefaultPosition, wxDefaultSize, 0);
	m_cbPlannedSpeed->SetValue(true);
	m_cbPlannedSpeed->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

	sbSizer61->Add(m_cbPlannedSpeed, 0, wxALL, 5);

	m_staticText6 = new wxStaticText(sbSizer61->GetStaticBox(), wxID_ANY, _("If checked the speeds used \nfor the ETA calculations will\ncome from speeds \nentered in the route table.\n\nIf unchecked the speeds\nwill be taken from the entry\non this page.\n"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText6->Wrap(-1);
	sbSizer61->Add(m_staticText6, 0, wxALL, 5);


	bSizer2->Add(sbSizer61, 1, wxEXPAND, 5);

	bSizer1->Add(bSizer2, 1, wxEXPAND, 5);

	sbSizer4->Add(bSizer1, 1, wxEXPAND, 5);


	fgSizer1->Add(sbSizer4, 1, wxALL | wxEXPAND, 5);

	wxStaticBoxSizer* sbSizer6;
	sbSizer6 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Tidal Current Data")), wxVERTICAL);

	m_dirPicker1 = new wxDirPickerCtrl(sbSizer6->GetStaticBox(), wxID_ANY, wxEmptyString, _("Select a folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE);
	sbSizer6->Add(m_dirPicker1, 0, wxALL | wxEXPAND, 5);


	fgSizer1->Add(sbSizer6, 1, wxEXPAND, 5);


	this->SetSizer(fgSizer1);
	this->Layout();
	fgSizer1->Fit(this);
	m_menubar3 = new wxMenuBar(0);
	m_menu3 = new wxMenu();
	wxMenuItem* m_mSummary;
	m_mSummary = new wxMenuItem(m_menu3, wxID_ANY, wxString(_("Summary")), wxEmptyString, wxITEM_CHECK);
	m_menu3->Append(m_mSummary);

	wxMenuItem* m_mNewRoute;
	m_mNewRoute = new wxMenuItem(m_menu3, wxID_ANY, wxString(_("Route Tables")), wxEmptyString, wxITEM_NORMAL);
	m_menu3->Append(m_mNewRoute);

	wxMenuItem* m_mDeleteAllRoutes;
	m_mDeleteAllRoutes = new wxMenuItem(m_menu3, wxID_ANY, wxString(_("Delete All Routes")), wxEmptyString, wxITEM_NORMAL);
	m_menu3->Append(m_mDeleteAllRoutes);

	m_menubar3->Append(m_menu3, _("Routes"));

	m_mHelp = new wxMenu();
	wxMenuItem* m_mInformation;
	m_mInformation = new wxMenuItem(m_mHelp, wxID_ANY, wxString(_("Guide")), wxEmptyString, wxITEM_NORMAL);
	m_mHelp->Append(m_mInformation);

	wxMenuItem* m_mAbout;
	m_mAbout = new wxMenuItem(m_mHelp, wxID_ANY, wxString(_("About")), wxEmptyString, wxITEM_NORMAL);
	m_mHelp->Append(m_mAbout);

	m_menubar3->Append(m_mHelp, _("Help"));

	this->SetMenuBar(m_menubar3);


	this->Centre(wxBOTH);

	// Connect Events
	this->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(otidalplanUIDialogBase::OnClose));
	this->Connect(wxEVT_SIZE, wxSizeEventHandler(otidalplanUIDialogBase::OnSize));
	m_bCalcDR->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(otidalplanUIDialogBase::DRCalculate), NULL, this);
	m_bCalcETA->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(otidalplanUIDialogBase::ETACalculate), NULL, this);
	m_buttonAttach->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(otidalplanUIDialogBase::OnAttachCurrents), NULL, this);
	m_dirPicker1->Connect(wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler(otidalplanUIDialogBase::OnFolderSelChanged), NULL, this);
	m_menu3->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(otidalplanUIDialogBase::OnSummary), this, m_mSummary->GetId());
	m_menu3->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(otidalplanUIDialogBase::OnShowTables), this, m_mNewRoute->GetId());
	m_menu3->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(otidalplanUIDialogBase::OnDeleteAllRoutes), this, m_mDeleteAllRoutes->GetId());
	m_mHelp->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(otidalplanUIDialogBase::OnInformation), this, m_mInformation->GetId());
	m_mHelp->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(otidalplanUIDialogBase::OnAbout), this, m_mAbout->GetId());

}

otidalplanUIDialogBase::~otidalplanUIDialogBase()
{
	// Disconnect Events
	this->Disconnect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(otidalplanUIDialogBase::OnClose));
	this->Disconnect(wxEVT_SIZE, wxSizeEventHandler(otidalplanUIDialogBase::OnSize));
	m_bCalcDR->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(otidalplanUIDialogBase::DRCalculate), NULL, this);
	m_bCalcETA->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(otidalplanUIDialogBase::ETACalculate), NULL, this);
	m_buttonAttach->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(otidalplanUIDialogBase::OnAttachCurrents), NULL, this);
	m_dirPicker1->Disconnect(wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler(otidalplanUIDialogBase::OnFolderSelChanged), NULL, this);

}

///////////////////////////////////////////////////////////////////////////

ConfigurationDialog::ConfigurationDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxFlexGridSizer* fgSizer95;
	fgSizer95 = new wxFlexGridSizer(3, 1, 0, 0);
	fgSizer95->AddGrowableCol(0);
	fgSizer95->AddGrowableCol(1);
	fgSizer95->SetFlexibleDirection(wxBOTH);
	fgSizer95->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	wxStaticBoxSizer* sbSizer29;
	sbSizer29 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Routes")), wxVERTICAL);

	m_lRoutes = new wxListBox(sbSizer29->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 | wxLB_ALWAYS_SB);
	sbSizer29->Add(m_lRoutes, 1, wxALL | wxEXPAND, 5);


	fgSizer95->Add(sbSizer29, 1, wxEXPAND, 5);

	wxFlexGridSizer* fgSizer78;
	fgSizer78 = new wxFlexGridSizer(1, 0, 0, 0);
	fgSizer78->SetFlexibleDirection(wxBOTH);
	fgSizer78->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_bDelete = new wxButton(this, wxID_ANY, _("Delete"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer78->Add(m_bDelete, 0, wxALL, 5);

	m_bSelect = new wxButton(this, wxID_ANY, _("Route Table"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer78->Add(m_bSelect, 0, wxALL, 5);

	m_bGenerate = new wxButton(this, wxID_ANY, _("Chart Route"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer78->Add(m_bGenerate, 0, wxALL, 5);

	m_bClose = new wxButton(this, wxID_ANY, _("Close"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer78->Add(m_bClose, 0, wxALL, 5);

	fgSizer95->Add(fgSizer78, 1, wxEXPAND, 5);

	this->SetSizer(fgSizer95);
	this->Layout();
	fgSizer95->Fit(this);
	this->Centre(wxBOTH);

	// Connect Events
	m_bDelete->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ConfigurationDialog::OnDelete), NULL, this);
	m_bSelect->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ConfigurationDialog::OnInformation), NULL, this);
	m_bGenerate->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ConfigurationDialog::OnGenerate), NULL, this);
	m_bClose->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ConfigurationDialog::OnClose), NULL, this);

}

ConfigurationDialog::~ConfigurationDialog()
{
	// Disconnect Events
	m_bDelete->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ConfigurationDialog::OnDelete), NULL, this);
	m_bSelect->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ConfigurationDialog::OnInformation), NULL, this);
	m_bGenerate->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ConfigurationDialog::OnGenerate), NULL, this);
	m_bClose->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ConfigurationDialog::OnClose), NULL, this);

}




void ConfigurationDialog::OnDelete(wxCommandEvent& event)
{
	wxString rn;
	int s;
	s = m_lRoutes->GetSelection();
	rn = m_lRoutes->GetString(s);
	
	for (std::list<TidalRoute>::iterator it = pPlugIn->m_potidalplanDialog->m_TidalRoutes.begin();
		it != pPlugIn->m_potidalplanDialog->m_TidalRoutes.end(); it++) {
		if ((*it).Name == rn ) {
			pPlugIn->m_potidalplanDialog->m_TidalRoutes.erase(it);
			m_lRoutes->Delete(s);

			pPlugIn->m_potidalplanDialog->SaveXML(pPlugIn->m_potidalplanDialog->m_default_configuration_path);
		}
	}
}
void ConfigurationDialog::OnInformation(wxCommandEvent& event)
{
	wxString rn;
	int s;
	s = m_lRoutes->GetSelection();

	if (s == -1){ 		
		wxMessageBox(_("Please select a route"));
		return; 
	}
	
	rn = m_lRoutes->GetString(s);
	/*
	
	if (m_lRoutes->IsEmpty()){
		wxMessageBox(_("Please select positions and generate a route"));
		return;
	}
	*/
	pPlugIn->m_potidalplanDialog->GetTable(rn);
	return; //
}


void ConfigurationDialog::OnGenerate(wxCommandEvent& event)
{
	wxString rn;
	int s;
	s = m_lRoutes->GetSelection();

	if (s == -1) {
		wxMessageBox(_("Please select a route"));
		return;
	}

	rn = m_lRoutes->GetString(s);

	pPlugIn->m_potidalplanDialog->AddChartRoute(rn);
}


void ConfigurationDialog::OnClose(wxCommandEvent& event)
{
	Hide();
}


///////////////////////////////////////////////////////////////////////////

AboutDialogBase::AboutDialogBase(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxFlexGridSizer* fgSizer90;
	fgSizer90 = new wxFlexGridSizer(0, 1, 0, 0);
	fgSizer90->SetFlexibleDirection(wxBOTH);
	fgSizer90->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	wxFlexGridSizer* fgSizer109;
	fgSizer109 = new wxFlexGridSizer(0, 2, 0, 0);
	fgSizer109->SetFlexibleDirection(wxBOTH);
	fgSizer109->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_staticText135 = new wxStaticText(this, wxID_ANY, _("otidalplan Plugin Version"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText135->Wrap(-1);
	fgSizer109->Add(m_staticText135, 0, wxALL, 5);

	m_stVersion = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	m_stVersion->Wrap(-1);
	fgSizer109->Add(m_stVersion, 0, wxALL, 5);


	fgSizer90->Add(fgSizer109, 1, wxEXPAND, 5);

	m_staticText110 = new wxStaticText(this, wxID_ANY, _("The otidalplan plugin for opencpn is intended to calculate routes based on computerized tidal data.\n\nLicense: GPLv3+\n\nSource Code:\nhttps://github.com/rasbats/otidalplan_pi\n\nAuthor:\nMike Rossiter\n\nMany thanks to all translators and testers."), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText110->Wrap(400);
	fgSizer90->Add(m_staticText110, 0, wxALL, 5);

	wxFlexGridSizer* fgSizer91;
	fgSizer91 = new wxFlexGridSizer(0, 2, 0, 0);
	fgSizer91->SetFlexibleDirection(wxBOTH);
	fgSizer91->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_bAboutAuthor = new wxButton(this, wxID_ANY, _("About Author"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer91->Add(m_bAboutAuthor, 0, wxALL, 5);

	m_bClose = new wxButton(this, wxID_ANY, _("Close"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer91->Add(m_bClose, 0, wxALL, 5);


	fgSizer90->Add(fgSizer91, 1, wxEXPAND, 5);


	this->SetSizer(fgSizer90);
	this->Layout();
	fgSizer90->Fit(this);

	this->Centre(wxBOTH);

	// Connect Events
	m_bAboutAuthor->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AboutDialogBase::OnAboutAuthor), NULL, this);
	m_bClose->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AboutDialogBase::OnClose), NULL, this);
}

AboutDialogBase::~AboutDialogBase()
{
	// Disconnect Events
	m_bAboutAuthor->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AboutDialogBase::OnAboutAuthor), NULL, this);
	m_bClose->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AboutDialogBase::OnClose), NULL, this);

}