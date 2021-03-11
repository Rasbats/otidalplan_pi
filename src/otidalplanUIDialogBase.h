///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __otidalplanUIDIALOGBASE_H__
#define __otidalplanUIDIALOGBASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/listctrl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/clrpicker.h>
#include <wx/choice.h>
#include <wx/colordlg.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/splitter.h>
#include <wx/listbox.h>
#include <wx/radiobox.h>
#include <wx/filepicker.h>
#include <wx/statline.h>

class ConfigurationDialog;
class otidalplan_pi;

///////////////////////////////////////////////////////////////////////////////
/// Class otidalplanUIDialogBase
///////////////////////////////////////////////////////////////////////////////
class otidalplanUIDialogBase : public wxFrame
{
private:

protected:
	wxTextCtrl* m_tSpeed;
	wxStaticText* m_staticText3;
	wxStaticText* m_staticText4;
	wxStaticText* m_staticText2;
	wxChoice* m_choiceDepartureTimes;
	wxButton* m_bCalcDR;
	wxStaticLine* m_staticline1;
	wxButton* m_bCalcETA;
	wxStaticText* m_staticText41;
	wxStaticText* m_stText;
	wxButton* m_buttonAttach;
	wxMenu* m_menu3;
	wxMenu* m_mHelp;
	wxStaticText* m_staticText6;

	// Virtual event handlers, overide them in your derived class
	virtual void OnClose(wxCloseEvent& event) { event.Skip(); }
	virtual void OnSize(wxSizeEvent& event) { event.Skip(); }
	virtual void DRCalculate(wxCommandEvent& event) { event.Skip(); }
	virtual void ETACalculate(wxCommandEvent& event) { event.Skip(); }
	virtual void OnAttachCurrents(wxCommandEvent& event) { event.Skip(); }
	virtual void OnFolderSelChanged(wxFileDirPickerEvent& event) { event.Skip(); }
	virtual void OnSummary(wxCommandEvent& event) { event.Skip(); }
	virtual void OnShowTables(wxCommandEvent& event) { event.Skip(); }
	virtual void OnDeleteAllRoutes(wxCommandEvent& event) { event.Skip(); }
	virtual void OnInformation(wxCommandEvent& event) { event.Skip(); }
	virtual void OnAbout(wxCommandEvent& event) { event.Skip(); }

public:
	wxTextCtrl* m_textCtrl1;
	wxTextCtrl* m_tRouteName;
	wxCheckBox* m_cbAddRoute;
	wxCheckBox* m_cbGPX;
	wxChoice* m_cMaxDist;
	wxChoice* m_cLegDist;
	wxCheckBox* m_cbAttach;
	wxCheckBox* m_cbPlannedSpeed;
	wxDirPickerCtrl* m_dirPicker1;
	wxMenuBar* m_menubar3;

	otidalplanUIDialogBase(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1, -1), long style = wxCAPTION | wxCLOSE_BOX | wxFRAME_FLOAT_ON_PARENT | wxRESIZE_BORDER | wxSYSTEM_MENU | wxTAB_TRAVERSAL);

	~otidalplanUIDialogBase();

};


///////////////////////////////////////////////////////////////////////////////
/// Class ConfigurationDialog
///////////////////////////////////////////////////////////////////////////////
class ConfigurationDialog : public wxDialog
{
private:
	
	

protected:

	wxButton* m_bDelete;
	wxButton* m_bSelect;
	wxButton* m_bGenerate;
	wxButton* m_bClose;

	// Virtual event handlers, overide them in your derived class
	void OnDelete(wxCommandEvent& event);
	void OnInformation(wxCommandEvent& event);
	void OnGenerate(wxCommandEvent& event);
	void OnClose(wxCommandEvent& event);

public:
	otidalplan_pi *pPlugIn;
	wxListBox* m_lRoutes;

	ConfigurationDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Tidal Routes"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
	~ConfigurationDialog();

};

///////////////////////////////////////////////////////////////////////////////
/// Class AboutDialogBase
///////////////////////////////////////////////////////////////////////////////
class AboutDialogBase : public wxDialog
{
private:

protected:
	wxStaticText* m_staticText135;
	wxStaticText* m_stVersion;
	wxStaticText* m_staticText110;
	wxButton* m_bAboutAuthor;
	wxButton* m_bClose;

	// Virtual event handlers, overide them in your derived class
	virtual void OnAboutAuthor(wxCommandEvent& event) { event.Skip(); }
	virtual void OnClose(wxCommandEvent& event) { event.Skip(); }


public:

	AboutDialogBase(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("About Weather Routing"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
	~AboutDialogBase();

};


#endif //__otidalplanUIDIALOGBASE_H__
