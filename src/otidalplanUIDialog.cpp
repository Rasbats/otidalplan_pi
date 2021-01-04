/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  otidalplan Object
 * Author:   Mike Rossiter
 *
 ***************************************************************************
 *   Copyright (C) 2016 by Mike Rossiter  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 */
#include <wx/intl.h>
#include "wx/wx.h"

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/debug.h>
#include <wx/graphics.h>
#include <wx/stdpaths.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "otidalplan_pi.h"
#include "icons.h"
#include <wx/arrimpl.cpp>

#ifdef __WXMSW__
#include <windows.h>
#endif
#include <memory.h> 

#include <wx/colordlg.h>
#include <wx/event.h>
#include "AboutDialog.h"


class TidalRoute;
class ConfigurationDialog;
class RouteProp;
class AboutDialog;

using namespace std;


#define HourFraction 0.01666666666
#define FAIL(X) do { error = X; goto failed; } while(0)

//date/time in the desired time zone format
static wxString TToString( const wxDateTime date_time, const int time_zone )
{
    wxDateTime t( date_time );
    t.MakeFromTimezone( wxDateTime::UTC );
    if( t.IsDST() ) t.Subtract( wxTimeSpan( 1, 0, 0, 0 ) );
    switch( time_zone ) {
        case 0: return t.Format( _T(" %a %d-%b-%Y  %H:%M LOC"), wxDateTime::Local );
        case 1:
        default: return t.Format( _T(" %a %d-%b-%Y %H:%M  UTC"), wxDateTime::UTC );
    }
}


static double deg2rad(double degrees)
{
	return M_PI * degrees / 180.0;
}

static double rad2deg(double radians)
{
	return 180.0 * radians / M_PI;
}

static void CTSWithCurrent(double BG, double &VBG, double C, double VC, double &BC, double VBC)
{
	if (VC == 0) { // short-cut if no current
		BC = BG, VBG = VBC;
		return;
	}
	
	// Thanks to Geoff Sargent at "tidalstreams.net"

	double B5 = VC / VBC;
	double C1 = deg2rad(BG);
	double C2 = deg2rad(C);

	double C6 = asin(B5 * sin(C1 - C2));
	double B6 = rad2deg(C6);
	if ((BG + B6) > 360){
		BC = BG + B6 - 360;
	}
	else {
		BC = BG + B6;
	}
	VBG = (VBC * cos(C6)) + (VC * cos(C1 - C2));

}

static void CMGWithCurrent(double &BG, double &VBG, double C, double VC, double BC, double VBC)
{
	if (VC == 0) { // short-cut if no current
		BG = BC, VBG = VBC;
		return;
	}

	// Thanks to Geoff Sargent at "tidalstreams.net"
	// BUT this function has not been tested !!!

	double B5 = VC / VBC;
	double C1 = deg2rad(BC);
	double C2 = deg2rad(C);

	double B3 = VC;
	double B4 = VBC;

	double C5 = sqr(B3) + sqr(B4) + 2 * B3*B4*cos(C1 - C2);
	double D5 = C5;
	double E5 = sqrt(C5);
	double E6 = B3*sin(C2 - C1) / E5;
	double E7 = asin(E6);
	double E8 = rad2deg(E7);
	if ((BC + E8) > 360){
		BG = BC + E8 - 360;
	}
	else {
		BG = BC + E8;
	}
	VBG = E5;
}

#if !wxCHECK_VERSION(2,9,4) /* to work with wx 2.8 */
#define SetBitmap SetBitmapLabel
#endif

GetRouteDialog::GetRouteDialog(wxWindow * parent, wxWindowID id, const wxString & title,
	const wxPoint & position, const wxSize & size , long style)
	: wxDialog(parent, id, title, position, size, style)
{

	wxString dimensions = wxT(""), s;
	wxPoint p;
	wxSize  sz;

	sz.SetWidth(size.GetWidth() - 20);
	sz.SetHeight(size.GetHeight() - 70);

	p.x = 6; p.y = 2;

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	dialogText = new wxListView(this, wxID_ANY, p, sz, wxLC_NO_HEADER | wxLC_REPORT, wxDefaultValidator, wxT(""));
	wxFont *pVLFont = wxTheFontList->FindOrCreateFont(12, wxFONTFAMILY_SWISS, wxNORMAL, wxFONTWEIGHT_NORMAL,
		FALSE, wxString(_T("Arial")));
	dialogText->SetFont(*pVLFont);

	bSizer1->Add(dialogText, 0, wxALL, 5);

	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton(this, wxID_OK);
	m_sdbSizer1->AddButton(m_sdbSizer1OK);
	m_sdbSizer1Cancel = new wxButton(this, wxID_CANCEL);
	m_sdbSizer1->AddButton(m_sdbSizer1Cancel);
	m_sdbSizer1->Realize();

	bSizer1->Add(m_sdbSizer1, 1, wxALIGN_CENTER, 5);

	m_sdbSizer1OK->SetDefault();

	this->SetSizer(bSizer1);
	this->Layout();
	bSizer1->Fit(this);

}

GetRouteDialog::~GetRouteDialog()
{
}


otidalplanUIDialog::otidalplanUIDialog(wxWindow *parent, otidalplan_pi *ppi)
	: otidalplanUIDialogBase(parent), m_ConfigurationDialog(this, wxID_ANY, _("Tidal Routes"),
	wxDefaultPosition, wxSize(-1, -1), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	pParent = parent;
    pPlugIn = ppi;

    wxFileConfig *pConf = GetOCPNConfigObject();

    if(pConf) {
        pConf->SetPath ( _T ( "/Settings/otidalplan" ) );

		pConf->Read(_T("otidalplanFolder"), &m_FolderSelected);

    }

	m_default_configuration_path = ppi->StandardPath()
		+ _T("otidalplan_config.xml");
	
	if (!OpenXML(m_default_configuration_path, false)) {
		// create directory for plugin files if it doesn't already exist 
		wxFileName fn(m_default_configuration_path);
		wxFileName fn2 = fn.GetPath();
		if (!fn.DirExists()) {
			fn2.Mkdir();
			fn.Mkdir();
		}
	}
	
	if (m_FolderSelected == wxEmptyString) {
		wxString g_SData_Locn = *GetpSharedDataLocation();
		// Establish location of Tide and Current data
		pTC_Dir = new wxString(_T("tcdata"));
		pTC_Dir->Prepend(g_SData_Locn);
		m_FolderSelected = *pTC_Dir;

		m_dirPicker1->SetPath(m_FolderSelected);
		//m_dirPicker1->GetTextCtrlValue();
	}
	else {
		m_dirPicker1->SetPath(m_FolderSelected);
	}

	LoadTCMFile();

	m_ConfigurationDialog.pPlugIn = ppi;   

    this->Connect( wxEVT_MOVE, wxMoveEventHandler( otidalplanUIDialog::OnMove ) );

	m_tSpeed->SetValue(_T("5"));
	m_dtNow = wxDateTime::Now(); 

	dummyTC.clear();

	wxString initStartDate = m_dtNow.Format(_T("%Y-%m-%d  %H:%M"));
	m_textCtrl1->SetValue(initStartDate);

	b_showTidalArrow = false;
	b_showCurrentIndicator = false;

	DimeWindow( this );

    Fit();
    SetMinSize( GetBestSize() );
	
}

otidalplanUIDialog::~otidalplanUIDialog()
{
    wxFileConfig *pConf = GetOCPNConfigObject();;

    if(pConf) {

        pConf->SetPath ( _T ( "/Settings/otidalplan" ) );

		wxString myF = m_dirPicker1->GetPath();
		pConf->Write(_T("otidalplanFolder"), myF);
		
    }
	SaveXML(m_default_configuration_path);
	
}

void otidalplanUIDialog::SetCursorLatLon( double lat, double lon )
{
    m_cursor_lon = lon;
    m_cursor_lat = lat;
}

void otidalplanUIDialog::SetViewPort( PlugIn_ViewPort *vp )
{
    if(m_vp == vp)  return;

    m_vp = new PlugIn_ViewPort(*vp);
}

void otidalplanUIDialog::OnClose( wxCloseEvent& event )
{
    pPlugIn->OnotidalplanDialogClose();
}

void otidalplanUIDialog::OnShowTables(wxCommandEvent& event)
{
	b_showTidalArrow = false;
	GetParent()->Refresh();
	m_ConfigurationDialog.Show();
}

void otidalplanUIDialog::OnDeleteAllRoutes(wxCommandEvent& event)
{
	if (m_TidalRoutes.empty()){
		wxMessageBox(_("No routes have been calculated"));
		return;
	}
	wxMessageDialog mdlg(this, _("Delete all routes?\n"),
		_("Delete All Routes"), wxYES | wxNO | wxICON_WARNING);
	if (mdlg.ShowModal() == wxID_YES) {
		m_TidalRoutes.clear();
		m_ConfigurationDialog.m_lRoutes->Clear();
	}

	GetParent()->Refresh();

}
void otidalplanUIDialog::OnMove( wxMoveEvent& event )
{
    //    Record the dialog position
    wxPoint p = GetPosition();
    pPlugIn->SetotidalplanDialogX( p.x );
    pPlugIn->SetotidalplanDialogY( p.y );

    event.Skip();
}

void otidalplanUIDialog::OnSize( wxSizeEvent& event )
{
    //    Record the dialog size
    wxSize p = event.GetSize();
    pPlugIn->SetotidalplanDialogSizeX( p.x );
    pPlugIn->SetotidalplanDialogSizeY( p.y );

    event.Skip();
}

void otidalplanUIDialog::OpenFile(bool newestFile)
{

	m_FolderSelected = pPlugIn->GetFolderSelected();
	m_IntervalSelected = pPlugIn->GetIntervalSelected();

	LoadTCMFile();
}

void otidalplanUIDialog::OnFolderSelChanged(wxFileDirPickerEvent& event)
{
	m_FolderSelected = m_dirPicker1->GetPath();
	//wxMessageBox(m_FolderSelected);
	LoadTCMFile();

	RequestRefresh(pParent);
}

wxString otidalplanUIDialog::MakeDateTimeLabel(wxDateTime myDateTime)
{			
		wxDateTime dt = myDateTime;

		wxString s2 = dt.Format ( _T( "%Y-%m-%d"));
		wxString s = dt.Format(_T("%H:%M")); 
		wxString dateLabel = s2 + _T(" ") + s;	

		m_textCtrl1->SetValue(dateLabel);				

		return dateLabel;	
}

void otidalplanUIDialog::OnInformation(wxCommandEvent& event)
{
	
	wxString infolocation = *GetpSharedDataLocation()
		+ _T("plugins/otidalplan_pi/data/") + _("TidalPlanInformation.html");
	wxLaunchDefaultBrowser(_T("file:///") + infolocation);
	
}

void otidalplanUIDialog::OnAbout(wxCommandEvent& event)
{
	AboutDialog dlg(GetParent());
	dlg.ShowModal();
}

void otidalplanUIDialog::OnSummary(wxCommandEvent& event)
{
	TableRoutes* tableroutes = new TableRoutes(this, 7000, _T(" Route Summary"), wxPoint(200, 200), wxSize(650, 200), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	
	wxString RouteName;
	wxString From;
	wxString Towards;
	wxString StartTime;
	wxString EndTime;
	wxString Duration;
	wxString Distance;
	wxString Type;

	if (m_TidalRoutes.empty()){

		wxMessageBox(_("No routes found. Please make a route"));
		return;
	}		

	int in = 0;
	
	for (std::list<TidalRoute>::iterator it = m_TidalRoutes.begin();
		it != m_TidalRoutes.end(); it++) {
		

		RouteName = (*it).Name;
		From = (*it).Start;
		Towards = (*it).End;
		StartTime = (*it).StartTime;
		EndTime = (*it).EndTime;
		Duration = (*it).Time;
		Distance = (*it).Distance;
		Type = (*it).Type;

		tableroutes->m_wpList->InsertItem(in, _T(""), -1);
		tableroutes->m_wpList->SetItem(in, 0, RouteName);
		tableroutes->m_wpList->SetItem(in, 1, From);
		tableroutes->m_wpList->SetItem(in, 2, Towards);
		tableroutes->m_wpList->SetItem(in, 3, StartTime);
		tableroutes->m_wpList->SetItem(in, 4, EndTime);
		tableroutes->m_wpList->SetItem(in, 5, Duration);
		tableroutes->m_wpList->SetItem(in, 6, Distance);
		tableroutes->m_wpList->SetItem(in, 7, Type);

		in++;	
		
	}

	tableroutes->Show();

	GetParent()->Refresh();
		
}


void otidalplanUIDialog::OnShowRouteTable(){
	
	wxString name;

	for (std::list<TidalRoute>::iterator it = m_TidalRoutes.begin();
		it != m_TidalRoutes.end(); it++) {

		if (!m_TidalRoutes.empty()){
			name = (*it).Name;
			break;
		}
		else {
			wxMessageBox(_("Please select or generate a route"));
			return;
		}
	}

	RouteProp* routetable = new RouteProp(this, 7000, _T("Tidal Routes"), wxPoint(200, 200), wxSize(650, -1), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

	int in = 0;

	wxString lat;
	wxString lon;
	wxString etd;
	wxString cts;
	wxString smg;
	wxString dis;
	wxString brg;
	wxString set;
	wxString rat;

	routetable->m_PlanSpeedCtl->SetValue(pPlugIn->m_potidalplanDialog->m_tSpeed->GetValue());

	for (std::list<TidalRoute>::iterator it = m_TidalRoutes.begin();
		it != m_TidalRoutes.end(); it++) {
		name = (*it).Name;
		if (m_tRouteName->GetValue() == name){

			routetable->m_RouteNameCtl->SetValue(name);
			routetable->m_RouteStartCtl->SetValue((*it).Start);
			routetable->m_RouteDestCtl->SetValue((*it).End);

			routetable->m_TotalDistCtl->SetValue((*it).Distance);
			routetable->m_TimeEnrouteCtl->SetValue((*it).Time);
			routetable->m_StartTimeCtl->SetValue((*it).StartTime);			
			routetable->m_TypeRouteCtl->SetValue((*it).Type);

			for (std::list<Position>::iterator itp = (*it).m_positionslist.begin();
				itp != (*it).m_positionslist.end(); itp++) {

				name = (*itp).name;
				lat = (*itp).lat;
				lon = (*itp).lon;
				etd = (*itp).time;
				cts = (*itp).CTS;
				smg = (*itp).SMG;
				dis = (*itp).distTo;
				brg = (*itp).brgTo;
				set = (*itp).set;
				rat = (*itp).rate;

				routetable->m_wpList->InsertItem(in, _T(""), -1);
				routetable->m_wpList->SetItem(in, 1, name);
				routetable->m_wpList->SetItem(in, 2, dis);
				routetable->m_wpList->SetItem(in, 4, lat);
				routetable->m_wpList->SetItem(in, 5, lon);
				routetable->m_wpList->SetItem(in, 6, etd);
				routetable->m_wpList->SetItem(in, 8, cts);
				routetable->m_wpList->SetItem(in, 9, set);
				routetable->m_wpList->SetItem(in, 10, rat);

				in++;
			}
		}
	}

	routetable->Show();

}

void otidalplanUIDialog::GetTable(wxString myRoute){

	wxString name;

	if (m_TidalRoutes.empty()){
		wxMessageBox(_("Please select or generate a route"));
		return;
	}	

	RouteProp* routetable = new RouteProp(this, 7000, _T("Tidal Route Table"), wxPoint(200, 200), wxSize(650, 800), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

	int in = 0;

	wxString lat;
	wxString lon;
	wxString etd;
	wxString cts;
	wxString smg;
	wxString dis;
	wxString brg;
	wxString set;
	wxString rat;

	routetable->m_PlanSpeedCtl->SetValue(pPlugIn->m_potidalplanDialog->m_tSpeed->GetValue());

	for (std::list<TidalRoute>::iterator it = m_TidalRoutes.begin();
		it != m_TidalRoutes.end(); it++) {
		name = (*it).Name;
		if (myRoute == name){

			routetable->m_RouteNameCtl->SetValue(name);
			routetable->m_RouteStartCtl->SetValue((*it).Start);
			routetable->m_RouteDestCtl->SetValue((*it).End);

			routetable->m_TotalDistCtl->SetValue((*it).Distance);
			routetable->m_TimeEnrouteCtl->SetValue((*it).Time);
			routetable->m_StartTimeCtl->SetValue((*it).StartTime);
			routetable->m_TypeRouteCtl->SetValue((*it).Type);

			for (std::list<Position>::iterator itp = (*it).m_positionslist.begin();
				itp != (*it).m_positionslist.end(); itp++) {

				name = (*itp).name;
				lat = (*itp).lat;
				lon = (*itp).lon;
				etd = (*itp).time;
				cts = (*itp).CTS;
				smg = (*itp).SMG;
				dis = (*itp).distTo;
				brg = (*itp).brgTo;
				set = (*itp).set;
				rat = (*itp).rate;

				routetable->m_wpList->InsertItem(in, _T(""), -1);
				routetable->m_wpList->SetItem(in, 1, name);
				routetable->m_wpList->SetItem(in, 2, dis);	
				routetable->m_wpList->SetItem(in, 3, brg);
				routetable->m_wpList->SetItem(in, 4, lat);
				routetable->m_wpList->SetItem(in, 5, lon);
				routetable->m_wpList->SetItem(in, 6, etd);
				routetable->m_wpList->SetItem(in, 7, smg);
				routetable->m_wpList->SetItem(in, 8, cts);
				routetable->m_wpList->SetItem(in, 9, set);
				routetable->m_wpList->SetItem(in, 10, rat);

				in++;
			}
		}
	}

	routetable->Show();

}


void otidalplanUIDialog::AddChartRoute(wxString myRoute) {

	b_showCurrentIndicator = false;

	PlugIn_Route* newRoute = new PlugIn_Route; // for adding a route on OpenCPN chart display
	PlugIn_Waypoint*  wayPoint = new PlugIn_Waypoint;
	
	double lati, loni, value, value1;

	for (std::list<TidalRoute>::iterator it = m_TidalRoutes.begin();
		it != m_TidalRoutes.end(); it++) {

		if ((*it).Name == myRoute) {
			newRoute->m_GUID = (*it).m_GUID;
			newRoute->m_NameString = (*it).Name;
			newRoute->m_StartString = (*it).Start;
			newRoute->m_EndString = (*it).End;

			

			for (std::list<Position>::iterator itp = (*it).m_positionslist.begin();
				itp != (*it).m_positionslist.end(); itp++) {

				PlugIn_Waypoint*  wayPoint = new PlugIn_Waypoint;
				
				wayPoint->m_MarkName = (*itp).name;
				if (!(*itp).lat.ToDouble(&value)) { /* error! */ }
				lati = value;
				if (!(*itp).lon.ToDouble(&value1)) { /* error! */ }
				loni = value1;
				wayPoint->m_lat = lati;
				wayPoint->m_lon = loni;
				wayPoint->m_MarkDescription = (*itp).time;
				wayPoint->m_GUID = (*itp).guid;
				wayPoint->m_IsVisible = (*itp).show_name;				
				wayPoint->m_IconName = (*itp).icon_name;				

				newRoute->pWaypointList->Append(wayPoint);
			}
 
			AddPlugInRoute(newRoute, true);

			if ((*it).Type == wxT("ETA")) {
				wxMessageBox(_("ETA Route has been charted!"));
			}
			else if ((*it).Type == wxT("DR")) {
				wxMessageBox(_("DR Route has been charted!"));
			}
			GetParent()->Refresh();
			break;

		}

	}

}


void otidalplanUIDialog::AddTidalRoute(TidalRoute tr)
{
	m_TidalRoutes.push_back(tr);
	wxString it = tr.Name;
	m_ConfigurationDialog.m_lRoutes->Append(it);

}

TotalTideArrow otidalplanUIDialog::tcCalculate(wxDateTime tcdt, int tcInt) // for working out the tc on a leg at a specified time
{
	
	bool bnew_val;
	float tcvalue, dir;

	m_ptcmgr->GetTideOrCurrent15(tcdt, tcInt, tcvalue, dir, bnew_val);
	tcForLeg.m_dir = dir;
	tcForLeg.m_force = tcvalue;
			
	return tcForLeg;
}

void otidalplanUIDialog::OnAttachCurrents(wxCommandEvent& event) {

	DummyTimedDR(event, false, 1);
}


void otidalplanUIDialog::DummyTimedDR(wxCommandEvent& event, bool write_file, int Pattern) {


	TotalTideArrow dummyTideArrow;

	dummyTC.clear();
	tc thisTC;

	wxString thisRoute = SelectRoute(true);

	if (thisRoute == wxEmptyString) {
		wxMessageBox("No route has been selected");
		return;
	}

	bool error_occured = false;
	double lat1, lon1;

	int num_hours = 1;
	int n = 0;

	lat1 = 0.0;
	lon1 = 0.0;

	wxString s;

	//Validate input ranges
	if (!error_occured) {
		if (std::abs(lat1) > 90) { error_occured = true; }
		if (std::abs(lon1) > 180) { error_occured = true; }
		if (error_occured) wxMessageBox(_("error in input range validation"));
	}

	if (dbg) cout << "DummyDR Calculation\n";
	double speed = 0;
	double maxDist;

	wxString s_LegDist = m_cLegDist->GetStringSelection();
	s_LegDist.ToDouble(&speed);

	wxString s_MaxDist = m_cMaxDist->GetStringSelection();
	s_MaxDist.ToDouble(&maxDist);

	double lati, loni;
	double latN[2000], lonN[2000];
	double latF, lonF;

	Position my_point;

	double value, value1;

	for (std::vector<Position>::iterator it = my_positions.begin(); it != my_positions.end(); it++) {

		if (!(*it).lat.ToDouble(&value)) { /* error! */ }
		lati = value;
		if (!(*it).lon.ToDouble(&value1)) { /* error! */ }
		loni = value1;

		waypointName[n] = (*it).wpt_num;

		latN[n] = lati;
		lonN[n] = loni;

		n++;
	}

	my_positions.clear();
	n--;

	int routepoints = n + 1;

	double myDist, myBrng, myDistForBrng;
	myBrng = 0.1;
	myDist = 0.1;

	double myLast, route_dist;

	route_dist = 0;
	myLast = 0;
	myDistForBrng = 0;
	double total_dist = 0;

	latF = latN[0];
	lonF = lonN[0];

	lati = latN[0];
	loni = lonN[0];

	double spd, dir;
	spd = 0;
	dir = 0;

	double iDist = 0;
	double tdist = 0; // For accumulating the total distance by adding the distance for each leg

	double iTime = 0;

	long longMinutes = 0;


	int epNumber = 0;
	int legNumber = 0;

	wxString epName;

	wxDateTime dt, iPointDT, gdt;

	wxString ddt, sdt;
	wxTimeSpan HourSpan, MinuteSpan, threeMinuteSpan;
	HourSpan = wxTimeSpan::Hours(1);
	MinuteSpan = wxTimeSpan::Minutes(30);
	threeMinuteSpan = wxTimeSpan::Minutes(3);
	wxTimeSpan pointMinuteSpan = wxTimeSpan::Minutes(3);

	bool madeETA = false;

	wxDateTime dtStart, dtEnd, interimDateTimeETA;
	int tcRefNum, last_tcRefNum;
	tcRefNum = 0;
	last_tcRefNum = 0;

	sdt = m_textCtrl1->GetValue(); // date/time route starts
	dt.ParseDateTime(sdt);

	sdt = dt.Format(_T(" %a %d-%b-%Y  %H:%M"));


	dtStart = dt;

	//
	// 
	//

	int wpn = 0;
	double VBG; // velocity of boat over ground
	VBG = speed;

	double latD, lonD; //latlon for the projected tide positions
	double legDistance, waypointDistance;
	double fractpart, intpart;
	int numEP;
	double timeToWaypoint, timeToRun;

	//wxString sSpeed = wxString::Format("%f", speed);
	//wxMessageBox(sSpeed);

	for (wpn; wpn < n; wpn++) {

		latF = latN[wpn]; 
		lonF = lonN[wpn];

		if (wpn == 0) {

			DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], latN[wpn], lonN[wpn], &myBrng, &waypointDistance); // how far to the next waypoint?
			timeToWaypoint = waypointDistance / VBG;

			PositionBearingDistanceMercator_Plugin(latN[wpn], lonN[wpn], myBrng, speed / 2, &latD, &lonD);
			tcRefNum = FindTCurrentStation(latD, lonD, maxDist);

			if (tcRefNum != 0) {
				dummyTideArrow = FindDummyTCurrent(tcRefNum);  // use at each waypoint

				thisTC.tcLat = dummyTideArrow.lat;
				thisTC.tcLon = dummyTideArrow.lon;
				thisTC.lat = latD;
				thisTC.lon = lonD;
				thisTC.tcRef = tcRefNum;
				thisTC.legNum = legNumber;
				thisTC.routeName = thisRoute;

				dummyTC.push_back(thisTC);
			}
			//
			// end of tidal current calc at the waypoint
			//


			if (timeToWaypoint < 1) {

				timeToRun = 1 - timeToWaypoint;		// getting to the next waypoint expend usedTime	
			}
			else {

				// work out number of waypoints
				// must be more than one as we have worked this out already
				DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], lati, loni, &myBrng, &waypointDistance); // how far to the next waypoint?

				timeToWaypoint = waypointDistance / VBG;
				fractpart = modf(timeToWaypoint, &intpart);
				numEP = intpart;

				if (numEP == 0) {
					//wxMessageBox("help");
					timeToRun = 1 - timeToWaypoint;

				}
				else {

					for (int z = 1; z <= numEP; z++) {

						PositionBearingDistanceMercator_Plugin(latF, lonF, myBrng, VBG, &lati, &loni);  // first waypoint of the leg
						// work out tc
						PositionBearingDistanceMercator_Plugin(lati, loni, myBrng, speed / 2, &latD, &lonD);
						tcRefNum = FindTCurrentStation(latD, lonD, maxDist);

						if (tcRefNum != 0) {
							dummyTideArrow = FindDummyTCurrent(tcRefNum); // update as we move along first leg

							thisTC.tcLat = dummyTideArrow.lat;
							thisTC.tcLon = dummyTideArrow.lon;
							thisTC.lat = latD;
							thisTC.lon = lonD;
							thisTC.tcRef = tcRefNum;
							thisTC.legNum = legNumber;
							thisTC.routeName = thisRoute;

							dummyTC.push_back(thisTC);
						}


						DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], lati, loni, &myBrng, &waypointDistance); // how far to the next waypoint?
						timeToWaypoint = waypointDistance / VBG;

						if (timeToWaypoint < 1) {
							
							z = numEP + 1; // to stop the next EP being made 

							//wxString sSpeed = wxString::Format("%f", ttwpt);
							//wxMessageBox(sSpeed);

							timeToRun = 1 - timeToWaypoint;
						}

						latF = lati;
						lonF = loni;
					}
				}
			}

		}
		else { // *************** After waypoint zero ******************************

			DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], latN[wpn], lonN[wpn], &myBrng, &waypointDistance); // how far to the next waypoint?
	
			PositionBearingDistanceMercator_Plugin(latN[wpn], lonN[wpn], myBrng, speed / 2, &latD, &lonD);
			tcRefNum = FindTCurrentStation(latD, lonD, maxDist);

			if (tcRefNum != 0) {
				dummyTideArrow = FindDummyTCurrent(tcRefNum);  // use at each waypoint

				thisTC.tcLat = dummyTideArrow.lat;
				thisTC.tcLon = dummyTideArrow.lon;
				thisTC.lat = latD;
				thisTC.lon = lonD;
				thisTC.tcRef = tcRefNum;
				thisTC.legNum = legNumber;
				thisTC.routeName = thisRoute;

				dummyTC.push_back(thisTC);
			}
			//
			// end of tidal current calc at the waypoint
			//
			timeToWaypoint = waypointDistance / VBG;

			if (timeToWaypoint < timeToRun) {
				timeToRun = timeToRun - timeToWaypoint;
			}
			else {
				// space for an EP
				// we have the position for the first EP on the new leg ... latloni

				waypointDistance = timeToRun / VBG;
				PositionBearingDistanceMercator_Plugin(latN[wpn], lonN[wpn], myBrng, waypointDistance, &lati, &loni);  // first waypoint of the new leg

				PositionBearingDistanceMercator_Plugin(latN[wpn], lonN[wpn], myBrng, speed / 2, &latD, &lonD);
				tcRefNum = FindTCurrentStation(latD, lonD, maxDist);

				if (tcRefNum != 0) {
					dummyTideArrow = FindDummyTCurrent(tcRefNum);  // use at each waypoint

					thisTC.tcLat = dummyTideArrow.lat;
					thisTC.tcLon = dummyTideArrow.lon;
					thisTC.lat = latD;
					thisTC.lon = lonD;
					thisTC.tcRef = tcRefNum;
					thisTC.legNum = legNumber;
					thisTC.routeName = thisRoute;

					dummyTC.push_back(thisTC);
				}


				DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], lati, loni, &myBrng, &waypointDistance); // how far to the next waypoint?

				latF = lati;
				lonF = loni;

				timeToWaypoint = waypointDistance / VBG;
				fractpart = modf(timeToWaypoint, &intpart);
				numEP = intpart;


				if (numEP == 0) {
					//wxMessageBox("help");
					timeToRun = 1 - timeToWaypoint;

				} else{

					for (int z = 1; z <= numEP; z++) {

						PositionBearingDistanceMercator_Plugin(latF, lonF, myBrng, VBG, &lati, &loni);  

						PositionBearingDistanceMercator_Plugin(lati, loni, myBrng, speed / 2, &latD, &lonD);
						tcRefNum = FindTCurrentStation(latD, lonD, maxDist);

						if (tcRefNum != 0) {
							dummyTideArrow = FindDummyTCurrent(tcRefNum); // update as we move along the leg

							thisTC.tcLat = dummyTideArrow.lat;
							thisTC.tcLon = dummyTideArrow.lon;
							thisTC.lat = latD;
							thisTC.lon = lonD;
							thisTC.tcRef = tcRefNum;
							thisTC.legNum = legNumber;
							thisTC.routeName = thisRoute;

							dummyTC.push_back(thisTC);
						}


						DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], lati, loni, &myBrng, &waypointDistance); // how far to the next waypoint?

						timeToWaypoint = waypointDistance / VBG;

						if (timeToWaypoint < 1) {
							z = numEP + 1; // to stop the next EP being made 

							//wxString sSpeed = wxString::Format("%f", ttwpt);
							//wxMessageBox(sSpeed);

							timeToRun = 1 - timeToWaypoint;
						}

						latF = lati;
						lonF = loni;

						//PositionBearingDistanceMercator_Plugin(lati, loni, myBrng, VBG, &latF, &lonF); // position of the next EP
					}
				}
			} // finished the wpn				
		}  // finished all the waypoints of a route

	}

	//wxString sShow1 = wxString::Format("%i", dummyTC.size());
	//wxMessageBox(sShow1);



	GetParent()->Refresh();

	//end of if no error occured

	if (error_occured == true) {
		wxLogMessage(_("Error in calculation. Please check input!"));
		wxMessageBox(_("Error in calculation. Please check input!"));
	}

	b_showCurrentIndicator = m_cbAttach->GetValue();


}


void otidalplanUIDialog::DummyDR(wxCommandEvent& event, bool write_file, int Pattern) {


	TotalTideArrow dummyTideArrow;

	dummyTC.clear();
	tc thisTC;

	wxString thisRoute = SelectRoute(true);

	if (thisRoute == wxEmptyString) {
		wxMessageBox("No route has been selected");
		return;
	}

	PlugIn_Route* newRoute = new PlugIn_Route; // for immediate use as a route on OpenCPN chart display


	newRoute->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));
	newRoute->m_NameString = newRoute->m_GUID;


	bool error_occured = false;
	double lat1, lon1;

	int num_hours = 1;
	int n = 0;

	lat1 = 0.0;
	lon1 = 0.0;

	wxString s;

	//Validate input ranges
	if (!error_occured) {
		if (std::abs(lat1) > 90) { error_occured = true; }
		if (std::abs(lon1) > 180) { error_occured = true; }
		if (error_occured) wxMessageBox(_("error in input range validation"));
	}

	switch (Pattern) {
	case 1:
	{
		if (dbg) cout << "DummyDR Calculation\n";
		double speed = 0;
		double maxDist;

		wxString s_LegDist = m_cLegDist->GetStringSelection();
		s_LegDist.ToDouble(&speed);

		wxString s_MaxDist = m_cMaxDist->GetStringSelection();
		s_MaxDist.ToDouble(&maxDist);

		double lati, loni;
		double latN[2000], lonN[2000];
		double latF, lonF;

		Position my_point;

		double value, value1;

		for (std::vector<Position>::iterator it = my_positions.begin(); it != my_positions.end(); it++) {

			if (!(*it).lat.ToDouble(&value)) { /* error! */ }
			lati = value;
			if (!(*it).lon.ToDouble(&value1)) { /* error! */ }
			loni = value1;

			waypointName[n] = (*it).wpt_num;

			latN[n] = lati;
			lonN[n] = loni;

			n++;
		}

		my_positions.clear();
		n--;

		int routepoints = n + 1;

		double myDist, myBrng, myDistForBrng;
		myBrng = 0.1;
		myDist = 0.1;

		double myLast, route_dist;

		route_dist = 0;
		myLast = 0;
		myDistForBrng = 0;
		double total_dist = 0;
		int i, c;

		latF = latN[0];
		lonF = lonN[0];

		lati = latN[0];
		loni = lonN[0];

		double latD;
		double lonD;

		int tc_index = 0;
		c = 0;
		double myD, myB;

		bool skipleg = false;
		double rdHours = 0;
		double rdHours1 = 0;
		double rdHours2 = 0;
		double rdHours3 = 0;
		double fttg = 0;
		double ttg = 0;
		double ttg1 = 0;
		double ttg2 = 0;
		double ttg3 = 0;
		double rdMiles = 0;
		double bitDist1 = 0;
		double bitDist2 = 0;
		double bitDist3 = 0;
		double lastBrg = 0;
		int skipCount = 0;

		bool skip0 = false;
		bool skip1 = false;
		int count3MinuteSteps = 0;
		double spd, dir;
		spd = 0;
		dir = 0;

		double iDist = 0;
		double tdist = 0; // For accumulating the total distance by adding the distance for each leg

		double skippedDistance = 0;

		double iTime = 0;

		long longMinutes = 0;


		int epNumber = 0;
		int legNumber = 0;

		wxString epName;

		wxDateTime dt, iPointDT, gdt;

		wxString ddt, sdt;
		wxTimeSpan HourSpan, MinuteSpan, threeMinuteSpan;
		HourSpan = wxTimeSpan::Hours(1);
		MinuteSpan = wxTimeSpan::Minutes(30);
		threeMinuteSpan = wxTimeSpan::Minutes(3);
		wxTimeSpan pointMinuteSpan = wxTimeSpan::Minutes(3);

		bool madeETA = false;

		wxDateTime dtStart, dtEnd, interimDateTimeETA;
		int tcRefNum, last_tcRefNum;
		tcRefNum = 0;
		last_tcRefNum = 0;

		sdt = m_textCtrl1->GetValue(); // date/time route starts
		dt.ParseDateTime(sdt);

		sdt = dt.Format(_T(" %a %d-%b-%Y  %H:%M"));


		dtStart = dt;

		//
		// 
		//

		double llat, llon;

		for (i = 0; i < n; i++) {	// n is number of routepoints	

			legNumber = i;

			lastBrg = myBrng;
			bitDist2 = myDist;

			DistanceBearingMercator(latN[i + 1], lonN[i + 1], latN[i], lonN[i], &myDist, &myBrng);

			if (myDist < speed) {
				destLoxodrome(latN[i], lonN[i], myBrng, myDist / 2, &latD, &lonD);
				tcRefNum = FindTCurrentStation(latD, lonD, maxDist);

				if (tcRefNum == 0) {
					tcRefNum = last_tcRefNum;

				}
				else {
					last_tcRefNum = tcRefNum;
				}


				if (tcRefNum != 0) {
					dummyTideArrow = FindDummyTCurrent(tcRefNum);

					thisTC.tcLat = dummyTideArrow.lat;
					thisTC.tcLon = dummyTideArrow.lon;
					thisTC.lat = latD;
					thisTC.lon = lonD;
					thisTC.tcRef = tcRefNum;
					thisTC.legNum = legNumber;
					thisTC.routeName = thisRoute;

					dummyTC.push_back(thisTC);
				}

			}

			if ((i == 0) && (myDist > speed)) {  // not yet reached a DR

				destLoxodrome(latN[i], lonN[i], myBrng, speed / 2, &latD, &lonD);
				tcRefNum = FindTCurrentStation(latD, lonD, maxDist);

				if (tcRefNum == 0) {
					tcRefNum = last_tcRefNum;

				}
				else {
					last_tcRefNum = tcRefNum;
				}

				if (tcRefNum != 0) {
					dummyTideArrow = FindDummyTCurrent(tcRefNum);

					thisTC.tcLat = dummyTideArrow.lat;
					thisTC.tcLon = dummyTideArrow.lon;
					thisTC.lat = latD;
					thisTC.lon = lonD;
					thisTC.tcRef = tcRefNum;
					thisTC.legNum = legNumber;
					thisTC.routeName = thisRoute;

					dummyTC.push_back(thisTC);
				}

			}

			//
			// set up the waypoint
			//
			//
			/*
			There are two things to record ... an OpenCPN route and a vector of positions for the tc.
			*/

			PlugIn_Waypoint*  newPoint = new PlugIn_Waypoint
			(latN[i], lonN[i], wxT("Circle"), waypointName[i]);
			newPoint->m_IconName = wxT("Circle");
			newPoint->m_MarkDescription = wxString::Format("%i", tcRefNum);      //dt.Format(_T(" %a %d-%b-%Y  %H:%M"));
			newPoint->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));
			newRoute->pWaypointList->Append(newPoint);


			if (i == 0) {

				interimDateTimeETA = dt;   // Initialise the interim with the start date/time

			}
			else {

				iDist = 0;
				iTime = 0;

				madeETA = false;

			}

			latF = latN[i];
			lonF = lonN[i];

			//
			// we are allowing for two legs to be skipped without the calculation stopping
			// (this is two consecutive legs of 3 minutes duration)
			//

			if (myDist < speed / 20 && i == 0) {  // test whether first leg needs to be skipped
				fttg = myDist / speed;
				iTime = fttg * 60;  // minutes for this leg

				skipleg = true;
				skippedDistance = myDist;

			}

			if (skipleg) {  // previous leg was skipped
				//
				// sailing a leg after the skipped leg
				//
				if (i == 0) {

					iDist = skippedDistance;

				}

				else {

					if (i == 1) {
						rdHours1 = bitDist2 / speed;
						ttg = 0.05 - (rdHours1);
						rdMiles = ttg * speed;

						fttg = (rdHours1)+(myDist / speed);

						if (fttg < 0.05) {

							wxMessageBox(_("Unable to calculate ETA over two legs at this speed. \n\n Aborting"), wxT("Problem"));
							return;
						}


					}
					else {  // not at routePoint #1

						rdHours1 = bitDist1 / speed;
						rdHours2 = bitDist2 / speed;
						ttg = 0.05 - (rdHours1 + rdHours2);
						rdMiles = ttg * speed;

						fttg = rdHours1 + rdHours2 + (myDist / speed);
						if (fttg < 0.05) {

							wxMessageBox(_("Unable to calculate ETA over two legs at this speed. \n\n Aborting"), wxT("Problem"));
							return;
						}

						destLoxodrome(latF, lonF, myBrng, rdMiles, &lati, &loni);
						iDist = skippedDistance + rdMiles;
						iTime = (iDist / speed) * 60;
						skippedDistance = 0;
						skipleg = false;

					}

				}

			}
			else {
				//
				// Not sailing a skipped leg
				//

				if (i == 1) {
					if (skipleg) {
						rdHours1 = bitDist2 / speed;
						ttg = 0.05 - rdHours1;
						rdMiles = ttg * speed;

						fttg = rdHours1 + (myDist / speed);
						destLoxodrome(latF, lonF, myBrng, rdMiles, &lati, &loni);

						if (fttg < 0.05) {

							skipleg = true; // two skips and you are out ... first and second legs
							wxMessageBox(_("Unable to calculate ETA over two legs at this speed. \n\n Aborting"), wxT("Problem"));
							return;

						}
						else {
							pointMinuteSpan = wxTimeSpan::Minutes(fttg * 60);
							iDist = skippedDistance + rdMiles;
							iTime = (iDist / speed) * 60;
							skippedDistance = 0;
							skipleg = false;

						}

					}
					else {
						rdHours1 = bitDist1 / speed;
						ttg = 0.05 - rdHours1;
						rdMiles = ttg * speed;

						fttg = rdHours1 + (myDist / speed);
						destLoxodrome(latF, lonF, myBrng, rdMiles, &lati, &loni);


						if (fttg < 0.05) {
							skippedDistance = rdMiles;
							skipleg = true;

						}
						else {
							iDist = skippedDistance + rdMiles;
							iTime = (iDist / speed) * 60;
							skippedDistance = 0;
							skipleg = false;
						}

					}

				}
				else {  // i != 1

					if (i == 0) {

						rdMiles = speed / 20;

						fttg = myDist / speed;
						destLoxodrome(latF, lonF, myBrng, rdMiles, &lati, &loni);
						iDist = rdMiles;
						iTime = (iDist / speed) * 60;


					}
					else {
						rdHours1 = bitDist1 / speed;
						ttg = 0.05 - rdHours1;
						rdMiles = ttg * speed;

						fttg = rdHours1 + (myDist / speed);
						destLoxodrome(latF, lonF, myBrng, rdMiles, &lati, &loni);
						iDist = rdMiles;
						iTime = (iDist / speed) * 60;

						if (fttg < 0.05) {
							skippedDistance = rdMiles;
							skipleg = true;

						}
						else {
							iDist = skippedDistance + rdMiles;
							iTime = (iDist / speed) * 60;

							skippedDistance = 0;
							skipleg = false;
						}

					}
				}

			}

			while (fttg > 0.05 && skipleg == false) {

				count3MinuteSteps++;

				latF = lati;
				lonF = loni;

				DistanceBearingMercator(latN[i + 1], lonN[i + 1], lati, loni, &myD, &myB);

				myDist = myD;

				bitDist1 = myD;

				if (count3MinuteSteps == 20) {  // we have reached a DR

					double myDDFinal, myBBFinal;

					destLoxodrome(lati, loni, myB, speed / 2, &latD, &lonD);  // project forward the location of the attachment

					DistanceBearingMercator(latD, lonD, lati, loni, &myDDFinal, &myBBFinal);

					if (myDDFinal < myD) {    // able to place the attachment on the leg

						tcRefNum = FindTCurrentStation(latD, lonD, maxDist);
						if (tcRefNum == 0) {
							tcRefNum = last_tcRefNum;

						}
						else {
							last_tcRefNum = tcRefNum;
						}

						dummyTideArrow = FindDummyTCurrent(tcRefNum);

						thisTC.tcLat = dummyTideArrow.lat;
						thisTC.tcLon = dummyTideArrow.lon;
						thisTC.lat = latD;
						thisTC.lon = lonD;
						thisTC.tcRef = tcRefNum;
						thisTC.legNum = legNumber;
						thisTC.routeName = thisRoute;

						dummyTC.push_back(thisTC);

					}

					double myDD, myBB;

					DistanceBearingMercator(latN[i], lonN[i], lati, loni, &myDD, &myBB);

					// myDD ... distance from DR to the next waypoint

					// need to stop adding an attachment where dest takes us past the last waypoint
					// myDD is distance to the last routepoint


					if (myDD < speed) {  // DR where no space to insert an attachment, like the last DR onwards

						destLoxodrome(lati, loni, myBB, myDD / 2, &latD, &lonD);
						tcRefNum = FindTCurrentStation(latD, lonD, maxDist);

						if (tcRefNum == 0) {
							tcRefNum = last_tcRefNum;

						}
						else {
							last_tcRefNum = tcRefNum;
						}
						if (tcRefNum != 0) {
							dummyTideArrow = FindDummyTCurrent(tcRefNum);

							thisTC.tcLat = dummyTideArrow.lat;
							thisTC.tcLon = dummyTideArrow.lon;
							thisTC.lat = latD;
							thisTC.lon = lonD;
							thisTC.tcRef = tcRefNum;
							thisTC.legNum = legNumber;
							thisTC.routeName = thisRoute;

							dummyTC.push_back(thisTC);
						}

					}

					DistanceBearingMercator(latN[i + 1], lonN[i + 1], lati, loni, &myDDFinal, &myBBFinal);
					// put an attachment halfway between DR and RoutePoint

					if (myDDFinal < speed) {

						destLoxodrome(lati, loni, myBBFinal, myDDFinal / 2, &latD, &lonD);

						tcRefNum = FindTCurrentStation(latD, lonD, maxDist);
						if (tcRefNum == 0) {
							tcRefNum = last_tcRefNum;

						}
						else {
							last_tcRefNum = tcRefNum;
						}

						if (tcRefNum != 0) {
							dummyTideArrow = FindDummyTCurrent(tcRefNum);

							thisTC.tcLat = dummyTideArrow.lat;
							thisTC.tcLon = dummyTideArrow.lon;
							thisTC.lat = latD;
							thisTC.lon = lonD;
							thisTC.tcRef = tcRefNum;
							thisTC.legNum = legNumber;
							thisTC.routeName = thisRoute;

							dummyTC.push_back(thisTC);

						}
					}


					epNumber++;
					epName = wxString::Format(wxT("%i"), epNumber);
					PlugIn_Waypoint*  epPoint = new PlugIn_Waypoint
					(lati, loni, wxT("Symbol-Square-White"), (_T("DR") + epName));
					epPoint->m_IconName = wxT("Symbol-Square-White");
					epPoint->m_MarkDescription = wxString::Format(wxT("%i"), tcRefNum);                                  //dt.Format(_T("%a %d-%b-%Y  %H:%M"));
					epPoint->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));
					newRoute->pWaypointList->Append(epPoint);

					llat = lati;
					llon = loni;

					count3MinuteSteps = 0;
					dt.Add(HourSpan);
					ddt = dt.Format(_T("%a %d-%b-%Y  %H:%M"));

					iDist = 0;
					iTime = 0;

					madeETA = true;

				}

				destLoxodrome(latF, lonF, myB, speed / 20, &lati, &loni);

				fttg = myD / speed;

			}

		}
		// End of new logic
		//
		PlugIn_Waypoint*  endPoint = new PlugIn_Waypoint
		(latN[n], lonN[n], wxT("Circle"), "end");

		endPoint->m_IconName = wxT("Circle");
		endPoint->m_MarkName = waypointName[n];
		endPoint->m_MarkDescription = ddt;
		endPoint->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));

		newRoute->pWaypointList->Append(endPoint);

		//AddPlugInRoute(newRoute); // add the route to OpenCPN routes and display the route on the chart			


		GetParent()->Refresh();


		break;
	}

	default:
	{            // Note the colon, not a semicolon
		cout << "Error, bad input, quitting\n";
		break;
	}
	}


	//end of if no error occured

	if (error_occured == true) {
		wxLogMessage(_("Error in calculation. Please check input!"));
		wxMessageBox(_("Error in calculation. Please check input!"));
	}

	b_showCurrentIndicator = m_cbAttach->GetValue();


}


void otidalplanUIDialog::DRCalculate(wxCommandEvent& event)
{

	bool fGPX = m_cbGPX->GetValue();
	if (fGPX) {
		CalcTimedDR(event, true, 1);
	}
	else {
		CalcTimedDR(event, false, 1);
	}
}


void otidalplanUIDialog::ETACalculate(wxCommandEvent& event)
{	
	
	bool fGPX = m_cbGPX->GetValue();
	if (fGPX) {
		CalcTimedETA(event, true, 1);
	}
	else {
		CalcTimedETA(event, false, 1);
	}
}

void otidalplanUIDialog::CalcTimedDR(wxCommandEvent& event, bool write_file, int Pattern)
{

	if (m_tRouteName->GetValue() == wxEmptyString) {
		wxMessageBox(_("Please enter a name for the calculated route!"));
		return;
	}

	wxString testName = SelectRoute(true); // SelectRoute takes the selected route and makes a vector of the waypoint positions
	if (testName == wxEmptyString) {
		wxMessageBox("No route has been selected");
		return;
	}

	m_choiceDepartureTimes->SetStringSelection(_T("1")); // we only need one DR route

	TidalRoute tr; // tidal route for saving in the config file
	PlugIn_Route* newRoute = new PlugIn_Route; // for immediate use as a route on OpenCPN chart display

	Position ptr; // position on the tidal route
	wxString m_RouteName;

	if (m_TidalRoutes.empty()) {

		m_RouteName = m_tRouteName->GetValue() + wxT(".") + wxT("DR");
		tr.Name = m_RouteName;
		tr.Type = _("DR");
	}
	else {
		for (std::list<TidalRoute>::iterator it = m_TidalRoutes.begin();
			it != m_TidalRoutes.end(); it++) {
			m_RouteName = m_tRouteName->GetValue() + wxT(".") + wxT("DR");
			if ((*it).Name == m_RouteName) {
				wxMessageBox(_("Route name already exists, please edit the name"));
				return;
			}
			else {
				tr.m_positionslist.clear();
				tr.Name = m_RouteName;
				tr.Type = _("DR");
			}
		}
	}

	newRoute->m_NameString = tr.Name;
	newRoute->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));

	tr.Start = wxT("Start");
	tr.End = wxT("End");
	tr.m_GUID = newRoute->m_GUID;


	bool error_occured = false;

	double lat1, lon1;



	int num_hours = 1;
	int n = 0;

	lat1 = 0.0;
	lon1 = 0.0;

	wxString s;
	if (write_file) {
		wxFileDialog dlg(this, _("Export DR Positions in GPX file as"), wxEmptyString, wxEmptyString, _T("GPX files (*.gpx)|*.gpx|All files (*.*)|*.*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (dlg.ShowModal() == wxID_CANCEL) {
			error_occured = true;     // the user changed idea...
			return;
		}

		s = dlg.GetPath();
		if (dlg.GetPath() == wxEmptyString) { error_occured = true; if (dbg) printf("Empty Path\n"); }
	}

	//Validate input ranges
	if (!error_occured) {
		if (std::abs(lat1) > 90) { error_occured = true; }
		if (std::abs(lon1) > 180) { error_occured = true; }
		if (error_occured) wxMessageBox(_("error in input range validation"));
	}

	//Start writing GPX
	TiXmlDocument doc;
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "");
	doc.LinkEndChild(decl);
	TiXmlElement * root = new TiXmlElement("gpx");
	TiXmlElement * Route = new TiXmlElement("rte");
	TiXmlElement * RouteName = new TiXmlElement("name");
	TiXmlText * text4 = new TiXmlText(this->m_tRouteName->GetValue().ToUTF8());

	if (write_file) {
		doc.LinkEndChild(root);
		root->SetAttribute("version", "0.1");
		root->SetAttribute("creator", "otidalplan_pi by Rasbats");
		root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
		root->SetAttribute("xmlns:gpxx", "http://www.garmin.com/xmlschemas/GpxExtensions/v3");
		root->SetAttribute("xsi:schemaLocation", "http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd");
		root->SetAttribute("xmlns:opencpn", "http://www.opencpn.org");
		Route->LinkEndChild(RouteName);
		RouteName->LinkEndChild(text4);
	}

		if (dbg) cout << "DR Calculation\n";
		double speed = 0;
		int    interval = 1;

		if (!this->m_tSpeed->GetValue().ToDouble(&speed)) { speed = 5.0; } // 5 kts default speed

		speed = speed * interval;

		double lati, loni;
		double latN[2000], lonN[2000];
		double latF, lonF;
		

		Position my_point;

		double value, value1;

		for (std::vector<Position>::iterator it = my_positions.begin(); it != my_positions.end(); it++) {

			if (!(*it).lat.ToDouble(&value)) { /* error! */ }
			lati = value;
			if (!(*it).lon.ToDouble(&value1)) { /* error! */ }
			loni = value1;

			waypointName[n] = (*it).name;
			waypointVisible[n] = (*it).visible;

			latN[n] = lati;
			lonN[n] = loni;

			n++;
		}

		my_positions.clear();
		n--;

		int routepoints = n + 1;

		double myDist, myBrng, myDistForBrng;
		myBrng = 0;
		myDist = 0;

		double myLast, route_dist;

		route_dist = 0;
		myLast = 0;
		myDistForBrng = 0;
		double total_dist = 0;
		int i, c;

		latF = latN[0];
		lonF = lonN[0];

		lati = latN[0];
		loni = lonN[0];

		double VBG;
		VBG = 0;
		int tc_index = 0;
		c = 0;
		double myD, myB;
		double myDI;

		bool skipleg = false;
		double lastVBG = 0;
		double lastVBG1 = 0;
		double VBG2 = 0;
		double rdHours = 0;
		double rdHours1 = 0;
		double rdHours2 = 0;
		double rdHours3 = 0;
		double fttg = 0;
		double ttg = 0;
		double ttg1 = 0;
		double ttg2 = 0;
		double ttg3 = 0;
		double rdMiles = 0;
		double bitDist1 = 0;
		double bitDist2 = 0;
		double bitDist3 = 0;
		double lastBrg = 0;
		int skipCount = 0;

		bool skip0 = false;
		bool skip1 = false;
		int count3MinuteSteps = 0;
		int total3MinuteSteps = 0;
		int count1MinuteSteps = 0;
		int total1MinuteSteps = 0;
		double spd, dir;
		spd = 0;
		dir = 0;

		double iDist = 0;
		double tdist = 0; // For accumulating the total distance by adding the distance for each leg
		double ptrDist = 0;
		double skippedDistance = 0;

		double iTime = 0;
		double ptrTime = 0;
		long longMinutes = 0;


		int epNumber = 0;
		wxString epName;

		wxDateTime dt;
		wxDateTime dtDRTime;

		wxDateTime dtStart, dtEnd, dtCurrent;

		wxString ddt, sdt;
		wxTimeSpan HourSpan, MinuteSpan, threeMinuteSpan, oneMinuteSpan;
		HourSpan = wxTimeSpan::Hours(1);

		bool madeETA = false;

		wxTimeSpan trTime;
		double trTimeHours;

		sdt = m_textCtrl1->GetValue(); // date/time route starts
		dt.ParseDateTime(sdt);

		sdt = dt.Format(_T(" %a %d-%b-%Y  %H:%M"));
		tr.StartTime = sdt;

		dtStart = dt;
		dtDRTime = dt;

		double waypointDistance = 0;
		double legDistance = 0;
		double timeToRun = 0;
		double timeToWaypoint = 0;
		int wpn = 0;	// waypoint number	

		int numEP;
		

		double fractpart, intpart;
		//
		// Start of new logic
		//

		VBG = speed;
		dtCurrent = dt;
		wxString isviz;

		tr.Start = waypointName[0].mb_str();
		for (wpn; wpn < n; wpn++) {
			isviz = waypointVisible[wpn];
			
			//
			// set up the waypoint
			//
			//
			PlugIn_Waypoint*  newPoint;
			if (isviz == "0") {
				newPoint = new PlugIn_Waypoint
				(latN[wpn], lonN[wpn], wxT("Symbol-Empty"), waypointName[wpn]);				
			}
			else {
				newPoint = new PlugIn_Waypoint
				(latN[wpn], lonN[wpn], wxT("Circle"), waypointName[wpn]);
			}
			newPoint->m_MarkName = waypointName[wpn];
			newPoint->m_MarkDescription = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
			newPoint->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));
			newRoute->pWaypointList->Append(newPoint);

			//
			// save the GPX file routepoint
			//

			my_point.lat = wxString::Format(wxT("%f"), latN[wpn]);
			my_point.lon = wxString::Format(wxT("%f"), lonN[wpn]);
			my_point.routepoint = 1;
			my_point.wpt_num = waypointName[wpn].mb_str();
			my_point.name = waypointName[wpn].mb_str();
			if (waypointVisible[wpn] == "1") {
				my_point.visible = "1";
			}
			else if (waypointVisible[wpn] == "0") {
				my_point.visible = "0";
			}

			my_point.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
			my_points.push_back(my_point);

			//
			// 
			//

			

			latF = latN[wpn]; // position of the last EP
			lonF = lonN[wpn];

			if (wpn == 0) {				

				DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], latN[wpn], lonN[wpn], &myBrng, &waypointDistance); // how far to the next waypoint?
				timeToWaypoint = waypointDistance / VBG;

				ptr.name = waypointName[wpn].mb_str();
				ptr.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
				ptr.lat = wxString::Format(_T("%8.4f"), latN[0]);
				ptr.lon = wxString::Format(_T("%8.4f"), lonN[0]);
				ptr.guid = newPoint->m_GUID;
				ptr.SMG = wxString::Format(_T("%5.1f"), VBG);
				ptrDist = waypointDistance;
				ptr.distTo = wxString::Format(_T("%.4f"), ptrDist);
				ptr.CTS = wxString::Format(_T("%03.0f"), myBrng);
				ptr.set = "----";
				ptr.rate = "----";
				ptr.icon_name = "Circle";
				tr.m_positionslist.push_back(ptr);


				//
				//
				//

				if (timeToWaypoint < 1) {  // no space for EP
					timeToRun = 1 - timeToWaypoint;		// getting to the next waypoint expend usedTime
					dtCurrent = AdvanceSeconds(dtCurrent, timeToWaypoint);
				}
				else {

					PositionBearingDistanceMercator_Plugin(latN[wpn], lonN[wpn], myBrng, VBG, &lati, &loni);  // first waypoint of the new leg

					dtCurrent = dtCurrent.Add(HourSpan);

					epNumber++;
					epName = "DR" + wxString::Format(wxT("%i"), epNumber);
					PlugIn_Waypoint*  epPoint = new PlugIn_Waypoint
					(lati, loni, wxT("Triangle"), epName);
					epPoint->m_IconName = wxT("square");				
					epPoint->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));
					newRoute->pWaypointList->Append(epPoint);   // for the OpenCPN display route

					// print EP for the GPX file
					my_point.lat = wxString::Format(wxT("%f"), lati);
					my_point.lon = wxString::Format(wxT("%f"), loni);
					my_point.routepoint = 0;
					my_point.wpt_num = _T("DR") + wxString::Format(_T("%i"), epNumber);
					my_point.name = _T("DR") + wxString::Format(_T("%i"), epNumber);
					my_point.visible = "1";
					my_point.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
					my_points.push_back(my_point);

					// print EP for the config file
					ptrDist = VBG;
					ptr.name = _T("DR") + epName;
					ptr.lat = wxString::Format(_T("%8.4f"), lati);
					ptr.lon = wxString::Format(_T("%8.4f"), loni);
					ptr.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
					ptr.guid = epPoint->m_GUID;
					ptr.distTo = wxString::Format(_T("%.4f"), ptrDist);
					ptr.brgTo = wxString::Format(_T("%03.0f"), myBrng);
					ptr.CTS = wxString::Format(_T("%03.0f"), myBrng);
					ptr.SMG = wxString::Format(_T("%5.1f"), VBG);
					ptr.set = "----";
					ptr.rate = "----";
					ptr.icon_name = wxT("square");
					tr.m_positionslist.push_back(ptr);

					// work out number of waypoints
					// must be more than one as we have worked this out already
					DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], lati, loni, &myBrng, &waypointDistance); // how far to the next waypoint?

					timeToWaypoint = waypointDistance / VBG;
					fractpart = modf(timeToWaypoint, &intpart);
					numEP = intpart;

					if (numEP == 0) {
						//wxMessageBox("help");
						timeToRun = 1 - timeToWaypoint;

					}
					else {


						for (int z = 1; z <= numEP; z++) {

							PositionBearingDistanceMercator_Plugin(latF, lonF, myBrng, VBG, &lati, &loni);  // first waypoint of the leg

							dtCurrent = dtCurrent.Add(HourSpan);

							epNumber++;
							epName = "DR" + wxString::Format(wxT("%i"), epNumber);
							PlugIn_Waypoint*  epPoint = new PlugIn_Waypoint
							(lati, loni, wxT("square"), epName);
							epPoint->m_IconName = wxT("square");
							epPoint->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));
							newRoute->pWaypointList->Append(epPoint);   // for the OpenCPN display route

							// print mid points for the GPX file
							my_point.lat = wxString::Format(wxT("%f"), lati);
							my_point.lon = wxString::Format(wxT("%f"), loni);
							my_point.routepoint = 0;
							my_point.wpt_num = _T("DR") + wxString::Format(_T("%i"), epNumber);
							my_point.name = _T("DR") + wxString::Format(_T("%i"), epNumber);
							my_point.visible = "1";
							my_point.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
							my_points.push_back(my_point);

							// print DR for the config file
							ptrDist = VBG;
							ptr.name = _T("DR") + epName;
							ptr.lat = wxString::Format(_T("%8.4f"), lati);
							ptr.lon = wxString::Format(_T("%8.4f"), loni);
							ptr.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
							ptr.guid = epPoint->m_GUID;
							ptr.distTo = wxString::Format(_T("%.4f"), ptrDist);
							ptr.brgTo = wxString::Format(_T("%03.0f"), myBrng);
							ptr.CTS = wxString::Format(_T("%03.0f"), myBrng);
							ptr.SMG = wxString::Format(_T("%5.1f"), VBG);
							ptr.set = "----";
							ptr.rate = "----";
							ptr.icon_name = wxT("square");
							tr.m_positionslist.push_back(ptr);


							DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], lati, loni, &myBrng, &waypointDistance); // how far to the next waypoint?
							timeToWaypoint = waypointDistance / VBG;

							if (timeToWaypoint < 1) {

								
								z = numEP + 1; // to stop the next EP being made 

								//wxString sSpeed = wxString::Format("%f", ttwpt);
								//wxMessageBox(sSpeed);

								timeToRun = 1 - timeToWaypoint;
								dtCurrent = AdvanceSeconds(dtCurrent, timeToWaypoint);
							}

							latF = lati;
							lonF = loni;

							//PositionBearingDistanceMercator_Plugin(lati, loni, myBrng, VBG, &latF, &lonF); // position of the next EP

						}
					}
				}

			}
			else { // *************** After waypoint zero ******************************

				

				//wxString sSpeed1 = wxString::Format("%f", ttwpt);
				//wxMessageBox(sSpeed1, "remainingTime");

				// timeToRun ... timeToWaypoint still to run for one hour

				DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], latN[wpn], lonN[wpn], &myBrng, &waypointDistance); // how far to the next waypoint?
				timeToWaypoint = waypointDistance / VBG;

				ptr.name = waypointName[wpn].mb_str();
				ptr.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
				ptr.lat = wxString::Format(_T("%8.4f"), latN[wpn]);
				ptr.lon = wxString::Format(_T("%8.4f"), lonN[wpn]);
				ptr.guid = newPoint->m_GUID;
				ptr.SMG = wxString::Format(_T("%5.1f"), VBG);
				ptrDist = waypointDistance;
				ptr.distTo = wxString::Format(_T("%.4f"), ptrDist);
				ptr.brgTo = wxString::Format(_T("%03.0f"), myBrng);
				ptr.CTS = wxString::Format(_T("%03.0f"), myBrng);
				ptr.set = "----";
				ptr.rate = "----";
				ptr.icon_name = "Circle";
				tr.m_positionslist.push_back(ptr);


				//
				//
				//

				if (timeToWaypoint < timeToRun) {					

					timeToRun = timeToRun - timeToWaypoint;
					dtCurrent = AdvanceSeconds(dtCurrent, timeToWaypoint);

				}
				else {
					// space for a DR
					// we have the position for the first DR on the new leg ... latloni
					// 

					waypointDistance = timeToRun * VBG;
					PositionBearingDistanceMercator_Plugin(latN[wpn], lonN[wpn], myBrng, waypointDistance, &lati, &loni);  // first waypoint of the new leg					

					dtCurrent = AdvanceSeconds(dtCurrent, timeToRun);

					epNumber++;
					epName = "DR" + wxString::Format(wxT("%i"), epNumber);
					PlugIn_Waypoint*  epPoint = new PlugIn_Waypoint
					(lati, loni, wxT("square"), epName);
					epPoint->m_IconName = wxT("square");
					epPoint->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));
					newRoute->pWaypointList->Append(epPoint);   // for the OpenCPN display route

					// print DR for the GPX file
					my_point.lat = wxString::Format(wxT("%f"), lati);
					my_point.lon = wxString::Format(wxT("%f"), loni);
					my_point.routepoint = 0;
					my_point.wpt_num = _T("DR") + wxString::Format(_T("%i"), epNumber);
					my_point.name = _T("DR") + wxString::Format(_T("%i"), epNumber);
					my_point.visible = "1";
					my_point.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
					my_points.push_back(my_point);

					// print DR for the config file
					ptrDist = waypointDistance;
					ptr.name = _T("DR") + epName;
					ptr.lat = wxString::Format(_T("%8.4f"), lati);
					ptr.lon = wxString::Format(_T("%8.4f"), loni);
					ptr.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
					ptr.guid = epPoint->m_GUID;
					ptr.distTo = wxString::Format(_T("%.4f"), ptrDist);
					ptr.brgTo = wxString::Format(_T("%03.0f"), myBrng);
					ptr.CTS = wxString::Format(_T("%03.0f"), myBrng);
					ptr.SMG = wxString::Format(_T("%5.1f"), VBG);
					ptr.set = "----";
					ptr.rate = "----";
					ptr.icon_name = wxT("square");
					tr.m_positionslist.push_back(ptr);



					DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], lati, loni, &myBrng, &waypointDistance); // how far to the next waypoint?

					latF = lati;
					lonF = loni;

					timeToWaypoint = waypointDistance / VBG;
					fractpart = modf(timeToWaypoint, &intpart);
					numEP = intpart;

					if (numEP == 0) {
						//wxMessageBox("help");
						timeToRun = 1 - timeToWaypoint;
						dtCurrent = AdvanceSeconds(dtCurrent, timeToWaypoint);
					}
					else {

						for (int z = 1; z <= numEP; z++) {

							PositionBearingDistanceMercator_Plugin(latF, lonF, myBrng, VBG, &lati, &loni);  // first waypoint of the leg							

							dtCurrent = dtCurrent.Add(HourSpan);

							epNumber++;
							epName = "DR" + wxString::Format(wxT("%i"), epNumber);
							PlugIn_Waypoint*  epPoint = new PlugIn_Waypoint
							(lati, loni, wxT("square"), epName);
							epPoint->m_IconName = wxT("square");
							epPoint->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));
							newRoute->pWaypointList->Append(epPoint);   // for the OpenCPN display route

							// print DR for the GPX file
							my_point.lat = wxString::Format(wxT("%f"), lati);
							my_point.lon = wxString::Format(wxT("%f"), loni);
							my_point.routepoint = 0;
							my_point.wpt_num = _T("DR") + wxString::Format(_T("%i"), epNumber);
							my_point.name = _T("DR") + wxString::Format(_T("%i"), epNumber);
							my_point.visible = "1";
							my_point.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
							my_points.push_back(my_point);

							// print DR for the config file
							ptrDist = VBG;
							ptr.name = _T("DR") + epName;
							ptr.lat = wxString::Format(_T("%8.4f"), lati);
							ptr.lon = wxString::Format(_T("%8.4f"), loni);
							ptr.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
							ptr.guid = epPoint->m_GUID;
							ptr.distTo = wxString::Format(_T("%.4f"), ptrDist);
							ptr.brgTo = wxString::Format(_T("%03.0f"), myBrng);
							ptr.CTS = wxString::Format(_T("%03.0f"), myBrng);
							ptr.SMG = wxString::Format(_T("%5.1f"), VBG);
							ptr.set = "----";
							ptr.rate = "----";
							ptr.icon_name = wxT("square");
							tr.m_positionslist.push_back(ptr);

							DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], lati, loni, &myBrng, &waypointDistance); // how far to the next waypoint?
							timeToWaypoint = waypointDistance / VBG;

							if (timeToWaypoint < 1) {

								dtCurrent.Subtract(HourSpan);
								z = numEP + 1; // to stop the next EP being made 

								//wxString sSpeed = wxString::Format("%f", ttwpt);
								//wxMessageBox(sSpeed);

								timeToRun = 1 - timeToWaypoint;
								dtCurrent = AdvanceSeconds(dtCurrent, timeToWaypoint);
							}

							latF = lati;
							lonF = loni;

						}
					}
				} // finished the wpn				
			}  // finished all the waypoints of a route

		}  // finished all the routes

		// print the last routepoint
		PlugIn_Waypoint*  endPoint = new PlugIn_Waypoint
		(latN[wpn], lonN[wpn], wxT("Circle"), waypointName[wpn]);
		endPoint->m_MarkName = waypointName[wpn];		
		endPoint->m_MarkDescription = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
		endPoint->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));	
		newRoute->pWaypointList->Append(endPoint);		

		// 
		// print the last waypoint for writing GPX
		//

		my_point.lat = wxString::Format(wxT("%f"), latN[wpn]);
		my_point.lon = wxString::Format(wxT("%f"), lonN[wpn]);
		my_point.routepoint = 1;
		my_point.wpt_num = waypointName[wpn].mb_str();
		my_point.name = waypointName[wpn].mb_str();		
		my_point.visible = "1";		
		my_point.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
		my_points.push_back(my_point);

		// print the last waypoint detail for the TidalRoute
		tr.EndTime = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));

		trTime = dtCurrent - dtStart;
		trTimeHours = (double)trTime.GetMinutes() / 60;
		tr.Time = wxString::Format(_("%.1f"), trTimeHours);
		tdist += iDist;
		tr.Distance = wxString::Format(_("%.1f"), tdist);

		ptrDist = waypointDistance;
		ptr.name = waypointName[wpn].mb_str();
		ptr.lat = wxString::Format(_T("%8.4f"), latN[n]);
		ptr.lon = wxString::Format(_T("%8.4f"), lonN[n]);
		ptr.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
		ptr.guid = endPoint->m_GUID;
		ptr.set = "----";
		ptr.rate = "----";
		ptr.CTS = _T("----");
		ptr.SMG = wxString::Format(_T("%5.1f"), VBG);
		ptr.distTo = wxString::Format(_T("%.4f"), ptrDist);
		ptr.brgTo = wxString::Format(_T("%03.0f"), myBrng);
		ptr.icon_name = wxT("Circle");


		tr.m_positionslist.push_back(ptr);
		tr.End = waypointName[wpn].mb_str();
		tr.Type = wxT("DR");
		m_TidalRoutes.push_back(tr);

		if (m_cbAddRoute->GetValue() == 1) {
			AddPlugInRoute(newRoute); // add the route to OpenCPN routes and display the route on the chart			
		}

		SaveXML(m_default_configuration_path); // add the route and extra detail (times, CTS etc) to the configuration file

		m_ConfigurationDialog.m_lRoutes->Append(tr.Name);
		m_ConfigurationDialog.Refresh();
		GetParent()->Refresh();
		wxString s_viz;

		for (std::vector<Position>::iterator itOut = my_points.begin(); itOut != my_points.end(); itOut++) {
			//wxMessageBox((*it).lat, _T("*it.lat"));

			double value, value1;
			wxString wptName, wptTime;
			if (!(*itOut).lat.ToDouble(&value)) { /* error! */ }
			lati = value;
			if (!(*itOut).lon.ToDouble(&value1)) { /* error! */ }
			loni = value1;

			s_viz = (*itOut).visible;
			wptTime = (*itOut).time;
			wptName = (*itOut).name;

			if ((*itOut).routepoint == 1) {
				if (write_file) { Addpoint(Route, wxString::Format(wxT("%f"), lati), wxString::Format(wxT("%f"), loni), wptName, wxT("Diamond"), _T("WPT"), s_viz, wptTime); }
			}
			else {
				if ((*itOut).routepoint == 0) {
					if (write_file) { Addpoint(Route, wxString::Format(wxT("%f"), lati), wxString::Format(wxT("%f"), loni), wptName, wxT("square"), _T("WPT"), s_viz, wptTime); }
				}
			}

		}

		my_points.clear();

		if (write_file) {

			TiXmlElement * Extensions = new TiXmlElement("extensions");

			TiXmlElement * StartN = new TiXmlElement("opencpn:start");
			TiXmlText * text5 = new TiXmlText(waypointName[0].ToUTF8());
			Extensions->LinkEndChild(StartN);
			StartN->LinkEndChild(text5);

			TiXmlElement * EndN = new TiXmlElement("opencpn:end");
			TiXmlText * text6 = new TiXmlText(waypointName[n].ToUTF8());
			Extensions->LinkEndChild(EndN);
			EndN->LinkEndChild(text6);

			wxString sSpd = wxString::Format(_T("%5.1f"), VBG);

			TiXmlElement * Pspd = new TiXmlElement("opencpn:planned_speed");
			TiXmlText * text7 = new TiXmlText(sSpd.ToUTF8());
			Extensions->LinkEndChild(Pspd);
			Pspd->LinkEndChild(text7);

			Route->LinkEndChild(Extensions);

			root->LinkEndChild(Route);
			// check if string ends with .gpx or .GPX
			if (!wxFileExists(s)) {
				//s = s + _T(".gpx");
			}
			wxCharBuffer buffer = s.ToUTF8();
			if (dbg) std::cout << buffer.data() << std::endl;
			doc.SaveFile(buffer.data());
		}



		//end of if no error occured

		if (error_occured == true) {
			wxLogMessage(_("Error in calculation. Please check input!"));
			wxMessageBox(_("Error in calculation. Please check input!"));
		}



	wxMessageBox(_("DR Route has been calculated!"));
}


double otidalplanUIDialog::ReadNavobj() {

	rte myRte;
	rtept myRtePt;
	vector<rtept> my_points;

	my_routes.clear();


	wxString rte_lat;
	wxString rte_lon;

	wxString wpt_guid;
	wxString wpt_name;
	wxString wpt_sym;
	wxString wpt_visible;

	wxString navobj_path = otidalplanUIDialog::StandardPath();
	wxString myFile = navobj_path + _T("navobj.xml");

	TiXmlDocument doc;
	wxString error;
	int myIndex = 0;

	if (!doc.LoadFile(myFile.mb_str())) {
		wxMessageBox(_T("Unable to read navobj file"));
		return -1;
	}
	else {
		TiXmlElement *root = doc.RootElement();
		if (!strcmp(root->Value(), "rte")) {
			wxMessageBox(_("Invalid xml file"));
			return -1;
		}

		int i = 0;
		
		bool nameFound = false;

		for (TiXmlElement* e = root->FirstChildElement(); e; e = e->NextSiblingElement(), i++) {

			if (!strcmp(e->Value(), "rte")) {
				nameFound = false;
				my_points.clear();

				for (TiXmlElement* f = e->FirstChildElement(); f; f = f->NextSiblingElement()) {

					if (!strcmp(f->Value(), "name")) {
						myRte.Name = wxString::FromUTF8(f->GetText());
						nameFound = true;
					}

					if (!strcmp(f->Value(), "rtept")) {

						wpt_visible = "1";

						rte_lat = wxString::FromUTF8(f->Attribute("lat"));
						rte_lon = wxString::FromUTF8(f->Attribute("lon"));

						myRtePt.lat = rte_lat;
						myRtePt.lon = rte_lon;								
                        

						for (TiXmlElement* i = f->FirstChildElement(); i; i = i->NextSiblingElement()) {
							
							if (!strcmp(i->Value(), "name")) {
														
								wpt_name = wxString::FromUTF8(i->GetText());								
								myRtePt.Name = wpt_name;								

							}		

							if (!strcmp(i->Value(), "sym")) {

								wpt_sym = wxString::FromUTF8(i->GetText());
								if (wpt_sym == "Empty" || wpt_sym == "Symbol-Empty" || wpt_sym == "empty"){
									myRtePt.visible = "0";
								}
								else {
									myRtePt.visible = "1";
								}

							}

							if (!strcmp(i->Value(), "extensions")) {								

								for (TiXmlElement* j = i->FirstChildElement(); j; j = j->NextSiblingElement()) {

									if (!strcmp(j->Value(), "opencpn:guid")) {
										wpt_guid = wxString::FromUTF8(j->GetText());

										myRtePt.m_GUID = wpt_guid;
									
									}

									if (!strcmp(j->Value(), "opencpn:viz")) {
										wpt_visible = wxString::FromUTF8(j->GetText());	
										myRtePt.visible = wpt_visible;

										if (wpt_sym == "Empty" || wpt_sym == "Symbol-Empty" || wpt_sym == "empty") {
											myRtePt.visible = "0";
										}										
									}

								}															
							}
						}

						myRtePt.index = myIndex;
						myIndex++;
						my_points.push_back(myRtePt);
						
					}

				}

				
				myRte.m_rteptList = my_points;
				if (!nameFound) {
					myRte.Name = _T("Unnamed");
				}
				my_routes.push_back(myRte);
				myIndex = 0;
				my_points.clear();
			}

			my_points.clear();
			myIndex = 0;
		}
	}
	return -1;
}

wxDateTime otidalplanUIDialog::AdvanceSeconds(wxDateTime currentTime, double HoursToAdvance)
{
	int secondsToAdvance = HoursToAdvance * 3600;
	wxTimeSpan SecondsSpan = wxTimeSpan::Seconds(secondsToAdvance); // One hour
	wxDateTime advancedTime = currentTime.Add(SecondsSpan);
	return advancedTime;

}
void otidalplanUIDialog::CalcTimedETA(wxCommandEvent& event, bool write_file, int Pattern)
{

	if (dummyTC.size() == 0) {
		wxMessageBox(_("Please attach tidal currents to a route first!"));
		return;
	}

	wxString thisRoute = SelectRoute(false);

	if (thisRoute == wxEmptyString) {
		wxMessageBox("No route has been selected");
		return;
	}


	if (m_tRouteName->GetValue() == wxEmptyString) {
		wxMessageBox(_("Please enter a name for the route!"));
		return;
	}

	if (m_textCtrl1->GetValue() == wxEmptyString) {
		wxMessageBox(_("Select a time!"));
		return;
	}

	wxString s_departureTimes = m_choiceDepartureTimes->GetStringSelection();
	int m_departureTimes = wxAtoi(s_departureTimes);
	int r = 0;
	wxString m_RouteName;

	for (r = 0; r < m_departureTimes; r++) {

		TidalRoute tr; // tidal route for saving in the config file ... provides eta, distances etc
		PlugIn_Route* newRoute = new PlugIn_Route; // for immediate use as a route on the OpenCPN chart display
		Position ptr; // for use in constructing the tidal route

		if (m_TidalRoutes.empty()) { // no existing list of routes
			m_RouteName = m_tRouteName->GetValue() + wxT(".") + wxString::Format(wxT("%i"), r) + wxT(".") + wxT("EP");
			tr.Name = m_RouteName;
			tr.Type = _("ETA");
		}
		else {
			for (std::list<TidalRoute>::iterator it = m_TidalRoutes.begin();
				it != m_TidalRoutes.end(); it++) {
				m_RouteName = m_tRouteName->GetValue() + wxT(".") + wxString::Format(wxT("%i"), r) + wxT(".") + wxT("EP");
				if ((*it).Name == m_RouteName) {
					wxMessageBox(_("Route name already exists, please edit the name"));
					return;
				}
				else {
					tr.m_positionslist.clear();
					tr.Name = m_RouteName;
					tr.Type = _("ETA");
				}
			}
		}

		newRoute->m_NameString = tr.Name;
		newRoute->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));

		tr.Start = wxT("Start");
		tr.End = wxT("End");
		tr.m_GUID = newRoute->m_GUID;

		bool error_occured = false;

		double lat1, lon1;
		int n = 0;

		lat1 = 0.0;
		lon1 = 0.0;

		wxString s;
		if (write_file) {
			wxFileDialog dlg(this, _("Export ETA Positions in GPX file as"), wxEmptyString, wxEmptyString, _T("GPX files (*.gpx)|*.gpx|All files (*.*)|*.*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			if (dlg.ShowModal() == wxID_CANCEL) {
				error_occured = true;     // the user changed idea...
				return;
			}

			s = dlg.GetPath();
			if (dlg.GetPath() == wxEmptyString) { error_occured = true; if (dbg) printf("Empty Path\n"); }
		}

		//Validate input ranges
		if (!error_occured) {
			if (std::abs(lat1) > 90) { error_occured = true; }
			if (std::abs(lon1) > 180) { error_occured = true; }
			if (error_occured) wxMessageBox(_("error in input range validation"));
		}

		//Start writing GPX
		TiXmlDocument doc;
		TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "");
		doc.LinkEndChild(decl);
		TiXmlElement * root = new TiXmlElement("gpx");
		TiXmlElement * Route = new TiXmlElement("rte");
		TiXmlElement * RouteName = new TiXmlElement("name");
		TiXmlText * text4 = new TiXmlText(this->m_tRouteName->GetValue().ToUTF8());

		if (write_file) {
			doc.LinkEndChild(root);
			root->SetAttribute("version", "0.1");
			root->SetAttribute("creator", "otidalplan_pi by Rasbats");
			root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
			root->SetAttribute("xmlns:gpxx", "http://www.garmin.com/xmlschemas/GpxExtensions/v3");
			root->SetAttribute("xsi:schemaLocation", "http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd");
			root->SetAttribute("xmlns:opencpn", "http://www.opencpn.org");
			Route->LinkEndChild(RouteName);
			RouteName->LinkEndChild(text4);
		}

		if (dbg) cout << "ETA Calculation\n";
		double speed = 0;
		int    interval = 1;

		if (!this->m_tSpeed->GetValue().ToDouble(&speed)) { speed = 5.0; } // 5 kts default speed

		speed = speed * interval;

		double lati, loni;
		double latN[2000], lonN[2000];
		double latF, lonF;

		Position my_point;

		double value, value1;
		//
		// Convert the vector into an arry of routepoints
		// Name, Lat, Lon
		//
		for (std::vector<Position>::iterator it = my_positions.begin(); it != my_positions.end(); it++) {

			if (!(*it).lat.ToDouble(&value)) { /* error! */ }
			lati = value;
			if (!(*it).lon.ToDouble(&value1)) { /* error! */ }
			loni = value1;

			waypointName[n] = (*it).name.mb_str();
			waypointVisible[n] = (*it).visible;

			latN[n] = lati;
			lonN[n] = loni;

			n++;
		}

		my_positions.clear();
		n--;   // Reset	

		wxTimeSpan HourSpan = wxTimeSpan::Hours(1); // One hour
		wxDateTime dtStart, dtEnd, dtCurrent;
		wxTimeSpan trTime;
		double trTimeHours = 0;

		wxString sdt = m_textCtrl1->GetValue(); // date/time route starts
		wxDateTime dt;
		wxString ddt;

		dt.ParseDateTime(sdt);
		dt = dt + wxTimeSpan::Hours(r);

		sdt = dt.Format(_T(" %a %d-%b-%Y  %H:%M"));
		tr.StartTime = sdt;

		dtStart = dt;
		dtCurrent = dt;
		int wpn = 0;	// waypoint number	
		double myBrng = 0;
		int cl = 0;
		double dir, spd;
		double VBG; // velocity of boat over ground
		double BC;  // course made good - not used

		double waypointDistance = 0;
		int numEP;
		double legDistance = 0;
		double timeToRun = 0;
		double timeToWaypoint = 0;

		// ptr variables
		double iDist = 0;
		double tdist = 0;
		double ptrDist = 0;
		int iTime = 0;
		wxDateTime ptrTime;

		int epNumber = 0;
		wxString epName;
		double fractpart, intpart;
		wxString isviz;

		/*
			// We are trying to do three things:
			//
			// **Make ptr points on the tidal route for making a route table etc
			//
			// **Option to save a GPX file of the calculated route
			//
			// **Making a new OpenCPN route for display on the chart
			//
		*/

		TotalTideArrow tcData;

		//wxString sSpeed = wxString::Format("%f", speed);
		//wxMessageBox(sSpeed);

		tr.Start = waypointName[0].mb_str();
		for (wpn; wpn < n; wpn++) {
			isviz = waypointVisible[wpn];
			

			DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], latN[wpn], lonN[wpn], &myBrng, &myDist);
			legDistance = myDist;

			cl = FindClosestDummyTCurrent(thisRoute, latN[wpn], lonN[wpn], speed);
			if (cl == 9999) {
				return;
			}
			if (cl != 0) {
				tcData = tcCalculate(dtCurrent, cl);
				dir = tcData.m_dir;
				spd = fabs(tcData.m_force);
			}
			else {
				dir = 0;
				spd = 0;
			}




			CTSWithCurrent(myBrng, VBG, dir, spd, BC, speed); // VBG = velocity of boat over ground			

			//
			// 
			//

			//
			// set up the waypoint
			//
			//

			PlugIn_Waypoint*  newPoint;
			if (waypointVisible[wpn] == "1") {
				newPoint = new PlugIn_Waypoint
				(latN[wpn], lonN[wpn], wxT("Circle"), waypointName[wpn]);			
			}
			else if (waypointVisible[wpn] == "0") {
				newPoint = new PlugIn_Waypoint
				(latN[wpn], lonN[wpn], wxT("Symbol-Empty"), waypointName[wpn]);
			}
			newPoint->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));
			newPoint->m_MarkDescription = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
			newRoute->pWaypointList->Append(newPoint);

			//
			// save the GPX file routepoint
			//

			my_point.lat = wxString::Format(wxT("%f"), latN[wpn]);
			my_point.lon = wxString::Format(wxT("%f"), lonN[wpn]);
			my_point.routepoint = 1;
			my_point.wpt_num = waypointName[wpn].mb_str();
			my_point.name = waypointName[wpn].mb_str();
			if (waypointVisible[wpn] == "1") {
				my_point.visible = "1";
			}
			else if (waypointVisible[wpn] == "0") {
				my_point.visible = "0";
			}
			my_point.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
			my_points.push_back(my_point);


			//
			//
			//
			
			ptr.name = waypointName[wpn].mb_str();
			ptr.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
			ptr.lat = wxString::Format(_T("%8.4f"), latN[wpn]);
			ptr.lon = wxString::Format(_T("%8.4f"), lonN[wpn]);
			ptr.guid = newPoint->m_GUID;
			ptr.CTS = wxString::Format(_T("%03.0f"), BC);
			ptr.SMG = wxString::Format(_T("%5.1f"), VBG);

			if (wpn == 0) {
				ptr.distTo = "----";
				ptr.brgTo = "----";				
			}
			else {
				ptrDist = waypointDistance;
				ptr.distTo = wxString::Format(_T("%.4f"), ptrDist);
				ptr.brgTo = wxString::Format(_T("%03.0f"), myBrng);			
			}

			ptr.set = wxString::Format(_T("%03.0f"), dir);
			ptr.rate = wxString::Format(_T("%5.1f"), spd);
			ptr.icon_name = "Circle";

			tr.m_positionslist.push_back(ptr);

	
			latF = latN[wpn]; // position of the last EP
			lonF = lonN[wpn];

			if (wpn == 0) {

				
				DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], latN[wpn], lonN[wpn], &myBrng, &waypointDistance); // how far to the next waypoint?
				timeToWaypoint = waypointDistance / VBG;


				if (timeToWaypoint < 1) {  // no space for EP

					timeToRun = 1 - timeToWaypoint;		// getting to the next waypoint expend usedTime
					dtCurrent = AdvanceSeconds(dtCurrent, timeToWaypoint);

				}
				else {

					PositionBearingDistanceMercator_Plugin(latN[wpn], lonN[wpn], myBrng, VBG, &lati, &loni);  // first waypoint of the new leg
					
					dtCurrent = dtCurrent.Add(HourSpan);

					epNumber++;
					epName = "EP" + wxString::Format(wxT("%i"), epNumber);
					PlugIn_Waypoint*  epPoint = new PlugIn_Waypoint
					(lati, loni, wxT("Triangle"), epName);
					epPoint->m_IconName = wxT("Triangle");
					epPoint->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));
					newRoute->pWaypointList->Append(epPoint);   // for the OpenCPN display route

					// print EP for the GPX file
					my_point.lat = wxString::Format(wxT("%f"), lati);
					my_point.lon = wxString::Format(wxT("%f"), loni);
					my_point.routepoint = 0;
					my_point.wpt_num = _T("EP") + wxString::Format(_T("%i"), epNumber);
					my_point.name = _T("EP") + wxString::Format(_T("%i"), epNumber);
					my_point.visible = "1";
					my_point.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
					my_points.push_back(my_point);

					// print EP for the config file

					ptrDist = VBG;
					ptr.name = _T("EP") + epName;
					ptr.lat = wxString::Format(_T("%8.4f"), lati);
					ptr.lon = wxString::Format(_T("%8.4f"), loni);
					ptr.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
					ptr.guid = epPoint->m_GUID;
					ptr.distTo = wxString::Format(_T("%.4f"), ptrDist);
					ptr.brgTo = wxString::Format(_T("%03.0f"), myBrng);
					ptr.CTS = wxString::Format(_T("%03.0f"), BC);
					ptr.SMG = wxString::Format(_T("%5.1f"), VBG);
					ptr.set = wxString::Format(_T("%03.0f"), dir);
					ptr.rate = wxString::Format(_T("%5.1f"), spd);
					ptr.icon_name = wxT("Triangle");
					tr.m_positionslist.push_back(ptr);

					// work out number of waypoints
					// must be more than one as we have worked this out already
					DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], lati, loni, &myBrng, &waypointDistance); // how far to the next waypoint?

					timeToWaypoint = waypointDistance / VBG;
					fractpart = modf(timeToWaypoint, &intpart);
					numEP = intpart;

					if (numEP == 0) {
						//wxMessageBox("help");
						timeToRun = 1 - timeToWaypoint;						

					}
					else {


						for (int z = 1; z <= numEP; z++) {

							PositionBearingDistanceMercator_Plugin(latF, lonF, myBrng, VBG, &lati, &loni);  // first waypoint of the leg

							dtCurrent = dtCurrent.Add(HourSpan);
							
							epNumber++;
							epName = "EP" + wxString::Format(wxT("%i"), epNumber);
							PlugIn_Waypoint*  epPoint = new PlugIn_Waypoint
							(lati, loni, wxT("Triangle"), (_T("EP") + epName));
							epPoint->m_IconName = wxT("Triangle");
							//epPoint->m_MarkDescription = ddt;
							epPoint->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));
							newRoute->pWaypointList->Append(epPoint);   // for the OpenCPN display route

							// print mid points for the GPX file
							my_point.lat = wxString::Format(wxT("%f"), lati);
							my_point.lon = wxString::Format(wxT("%f"), loni);
							my_point.routepoint = 0;
							my_point.wpt_num = _T("EP") + wxString::Format(_T("%i"), epNumber);
							my_point.name = _T("EP") + wxString::Format(_T("%i"), epNumber);
							my_point.visible = "1";
							my_point.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
							my_points.push_back(my_point);
							
							// print EP for the config file
							ptrDist = VBG;
							ptr.name = _T("EP") + epName;
							ptr.lat = wxString::Format(_T("%8.4f"), lati);
							ptr.lon = wxString::Format(_T("%8.4f"), loni);
							ptr.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
							ptr.guid = epPoint->m_GUID;
							ptr.distTo = wxString::Format(_T("%.4f"), ptrDist);
							ptr.brgTo = wxString::Format(_T("%03.0f"), myBrng);
							ptr.CTS = wxString::Format(_T("%03.0f"), BC);
							ptr.SMG = wxString::Format(_T("%5.1f"), VBG);
							ptr.set = wxString::Format(_T("%03.0f"), dir);
							ptr.rate = wxString::Format(_T("%5.1f"), spd);
							ptr.icon_name = wxT("Triangle");
							tr.m_positionslist.push_back(ptr);


							DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], lati, loni, &myBrng, &waypointDistance); // how far to the next waypoint?
							timeToWaypoint = waypointDistance / VBG;

							if (timeToWaypoint < 1) {

								dtCurrent.Subtract(HourSpan);
								z = numEP + 1; // to stop the next EP being made 

								//wxString sSpeed = wxString::Format("%f", ttwpt);
								//wxMessageBox(sSpeed);

								timeToRun = 1 - timeToWaypoint;
								dtCurrent = AdvanceSeconds(dtCurrent, timeToWaypoint);
							}

							cl = FindClosestDummyTCurrent(thisRoute, lati, loni, speed);
							if (cl == 9999) {
								return;
							}
							if (cl != 0) {
								tcData = tcCalculate(dtCurrent, cl);
								dir = tcData.m_dir;
								spd = fabs(tcData.m_force);
							}
							else {
								dir = 0;
								spd = 0;
							}

							CTSWithCurrent(myBrng, VBG, dir, spd, BC, speed); // VBG = velocity of boat over ground


							latF = lati;
							lonF = loni;

							//PositionBearingDistanceMercator_Plugin(lati, loni, myBrng, VBG, &latF, &lonF); // position of the next EP

						}
					}
				}

			}
			else { // *************** After waypoint zero ******************************

				


				DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], latN[wpn], lonN[wpn], &myBrng, &waypointDistance); // how far to the next waypoint?
				timeToWaypoint = waypointDistance / VBG;

				if (timeToWaypoint < timeToRun) {

					timeToRun = timeToRun - timeToWaypoint;
					dtCurrent = AdvanceSeconds(dtCurrent, timeToWaypoint);

				}
				else {
					// space for an EP
					// we have the position for the first EP on the new leg ... latloni
					// 

					waypointDistance = timeToRun * VBG;
					PositionBearingDistanceMercator_Plugin(latN[wpn], lonN[wpn], myBrng, waypointDistance, &lati, &loni);  // first waypoint of the new leg					

					dtCurrent = AdvanceSeconds(dtCurrent, timeToRun);

					epNumber++;
					epName = "EP" + wxString::Format(wxT("%i"), epNumber);
					PlugIn_Waypoint*  epPoint = new PlugIn_Waypoint
					(lati, loni, wxT("Triangle"), (_T("EP") + epName));
					epPoint->m_IconName = wxT("Triangle");
					epPoint->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));
					newRoute->pWaypointList->Append(epPoint);   // for the OpenCPN display route

					// print EP for the GPX file
					my_point.lat = wxString::Format(wxT("%f"), lati);
					my_point.lon = wxString::Format(wxT("%f"), loni);
					my_point.routepoint = 0;
					my_point.wpt_num = _T("EP") + wxString::Format(_T("%i"), epNumber);
					my_point.name = _T("EP") + wxString::Format(_T("%i"), epNumber);
					my_point.visible = "1";
					my_point.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
					my_points.push_back(my_point);

					// print EP for the config file
					ptrDist = waypointDistance;
					ptr.name = _T("EP") + epName;
					ptr.lat = wxString::Format(_T("%8.4f"), lati);
					ptr.lon = wxString::Format(_T("%8.4f"), loni);
					ptr.time = ddt;
					ptr.guid = epPoint->m_GUID;
					ptr.distTo = wxString::Format(_T("%.1f"), ptrDist);
					ptr.brgTo = wxString::Format(_T("%03.0f"), myBrng);
					ptr.CTS = wxString::Format(_T("%03.0f"), BC);
					ptr.SMG = wxString::Format(_T("%5.1f"), VBG);
					ptr.set = wxString::Format(_T("%03.0f"), dir);
					ptr.rate = wxString::Format(_T("%5.1f"), spd);
					ptr.icon_name = wxT("Triangle");
					tr.m_positionslist.push_back(ptr);

					DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], lati, loni, &myBrng, &waypointDistance); // how far to the next waypoint?

					latF = lati;
					lonF = loni;

					timeToWaypoint = waypointDistance / VBG;
					fractpart = modf(timeToWaypoint, &intpart);
					numEP = intpart;

					if (numEP == 0) {
						//wxMessageBox("help");
						timeToRun = 1 - timeToWaypoint;
						dtCurrent = AdvanceSeconds(dtCurrent, timeToWaypoint);
					}
					else {

						for (int z = 1; z <= numEP; z++) {

							PositionBearingDistanceMercator_Plugin(latF, lonF, myBrng, VBG, &lati, &loni);  // first waypoint of the leg							

							dtCurrent = dtCurrent.Add(HourSpan);

							epNumber++;
							epName = "EP" + wxString::Format(wxT("%i"), epNumber);
							PlugIn_Waypoint*  epPoint = new PlugIn_Waypoint
							(lati, loni, wxT("Triangle"), epName);
							epPoint->m_IconName = wxT("Triangle");
							epPoint->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));
							newRoute->pWaypointList->Append(epPoint);   // for the OpenCPN display route

							// print mid points for the GPX file
							my_point.lat = wxString::Format(wxT("%f"), lati);
							my_point.lon = wxString::Format(wxT("%f"), loni);
							my_point.routepoint = 0;
							my_point.wpt_num = _T("EP") + wxString::Format(_T("%i"), epNumber);
							my_point.name = _T("EP") + wxString::Format(_T("%i"), epNumber);
							my_point.visible = "1";
							my_point.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
							my_points.push_back(my_point);

							// print EP for the config file
							ptrDist = VBG;
							ptr.name = _T("EP") + epName;
							ptr.lat = wxString::Format(_T("%8.4f"), lati);
							ptr.lon = wxString::Format(_T("%8.4f"), loni);
							ptr.time = ddt;
							ptr.guid = epPoint->m_GUID;
							ptr.distTo = wxString::Format(_T("%.1f"), ptrDist);
							ptr.brgTo = wxString::Format(_T("%03.0f"), myBrng);
							ptr.CTS = wxString::Format(_T("%03.0f"), BC);
							ptr.SMG = wxString::Format(_T("%5.1f"), VBG);
							ptr.set = wxString::Format(_T("%03.0f"), dir);
							ptr.rate = wxString::Format(_T("%5.1f"), spd);
							ptr.icon_name = wxT("Triangle");
							tr.m_positionslist.push_back(ptr);
							
							cl = FindClosestDummyTCurrent(thisRoute, lati, loni, speed);
							if (cl == 9999) {
								return;
							}
							if (cl != 0) {
								tcData = tcCalculate(dtCurrent, cl);
								dir = tcData.m_dir;
								spd = fabs(tcData.m_force);
							}
							else {
								dir = 0;
								spd = 0;
							}

							CTSWithCurrent(myBrng, VBG, dir, spd, BC, speed); // VBG = velocity of boat over ground

							DistanceBearingMercator_Plugin(latN[wpn + 1], lonN[wpn + 1], lati, loni, &myBrng, &waypointDistance); // how far to the next waypoint?
							timeToWaypoint = waypointDistance / VBG;

							if (timeToWaypoint < 1) {

								dtCurrent.Subtract(HourSpan);
								z = numEP + 1; // to stop the next EP being made 

								//wxString sSpeed = wxString::Format("%f", ttwpt);
								//wxMessageBox(sSpeed);

								timeToRun = 1 - timeToWaypoint;
								dtCurrent = AdvanceSeconds(dtCurrent, timeToWaypoint);
							}

													
							/*
							cl = FindClosestDummyTCurrent(thisRoute, lati, loni, speed);
							if (cl == 9999) {
								return;
							}
							if (cl != 0) {
								tcData = tcCalculate(dtCurrent, cl);
								dir = tcData.m_dir;
								spd = fabs(tcData.m_force);
							}
							else {
								dir = 0;
								spd = 0;
							}

							CTSWithCurrent(myBrng, VBG, dir, spd, BC, speed); // VBG = velocity of boat over ground
							*/

							latF = lati;
							lonF = loni;

						}
					}
				} // finished the wpn				
			}  // finished all the waypoints of a route

		}  // finished all the routes

		// print the last routepoint
		PlugIn_Waypoint*  endPoint = new PlugIn_Waypoint
		(latN[wpn], lonN[wpn], wxT("Circle"), waypointName[wpn]);
		endPoint->m_MarkName = waypointName[wpn];
		endPoint->m_MarkDescription = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
		endPoint->m_GUID = wxString::Format(_T("%i"), (int)GetRandomNumber(1, 4000000));
		newRoute->pWaypointList->Append(endPoint);

		// 
		// print the last my_point for writing GPX
		//

		my_point.lat = wxString::Format(wxT("%f"), latN[wpn]);
		my_point.lon = wxString::Format(wxT("%f"), lonN[wpn]);
		my_point.routepoint = 1;
		my_point.wpt_num = waypointName[wpn].mb_str();
		my_point.name = waypointName[wpn].mb_str();
		my_point.visible = "1";
		my_point.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
		my_points.push_back(my_point);

		// print the last waypoint detail for the TidalRoute
		tr.EndTime = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));

		trTime = dtCurrent - dtStart;
		trTimeHours = (double)trTime.GetMinutes() / 60;
		tr.Time = wxString::Format(_("%.1f"), trTimeHours);
		tdist += iDist;
		tr.Distance = wxString::Format(_("%.1f"), tdist);

		ptrDist = waypointDistance;
		ptr.name = waypointName[wpn].mb_str();
		ptr.lat = wxString::Format(_T("%8.4f"), latN[n]);
		ptr.lon = wxString::Format(_T("%8.4f"), lonN[n]);
		ptr.time = dtCurrent.Format(_T(" %a %d-%b-%Y  %H:%M"));
		ptr.guid = endPoint->m_GUID;
		ptr.set = wxString::Format(_T("%03.0f"), dir);
		ptr.rate = wxString::Format(_T("%5.1f"), spd);
		ptr.CTS = _T("----");
		ptr.SMG = wxString::Format(_T("%5.1f"), VBG);
		ptr.distTo = wxString::Format(_T("%.1f"), ptrDist);
		ptr.brgTo = wxString::Format(_T("%03.0f"), myBrng);
		ptr.icon_name = wxT("Circle");

		tr.m_positionslist.push_back(ptr);
		tr.End = waypointName[wpn].mb_str();
		tr.Type = wxT("ETA");
		m_TidalRoutes.push_back(tr);



		if (m_cbAddRoute->GetValue() == 1) {
			AddPlugInRoute(newRoute); // add the route to OpenCPN routes and display the route on the chart			
		}

		SaveXML(m_default_configuration_path); // add the route and extra detail (times, CTS etc) to the configuration file

		m_ConfigurationDialog.m_lRoutes->Append(tr.Name);
		m_ConfigurationDialog.Refresh();
		GetParent()->Refresh();
		wxString s_viz;

		for (std::vector<Position>::iterator itOut = my_points.begin(); itOut != my_points.end(); itOut++) {

			double value, value1;
			wxString wptName, wptTime;
			if (!(*itOut).lat.ToDouble(&value)) { /* error! */ }
			lati = value;
			if (!(*itOut).lon.ToDouble(&value1)) { /* error! */ }
			loni = value1;

			s_viz = (*itOut).visible;
			wptTime = (*itOut).time;
			wptName = (*itOut).name;

			if ((*itOut).routepoint == 1) {
				if (write_file) { Addpoint(Route, wxString::Format(wxT("%f"), lati), wxString::Format(wxT("%f"), loni), wptName, wxT("Diamond"), _T("WPT"), s_viz, wptTime); }
			}
			else {
				if ((*itOut).routepoint == 0) {
					if (write_file) { Addpoint(Route, wxString::Format(wxT("%f"), lati), wxString::Format(wxT("%f"), loni), wptName, wxT("Triangle"), _T("WPT"), s_viz, wptTime); }
				}
			}

		}

		my_points.clear();

		if (write_file) {

			TiXmlElement * Extensions = new TiXmlElement("extensions");

			TiXmlElement * StartN = new TiXmlElement("opencpn:start");
			TiXmlText * text5 = new TiXmlText(waypointName[0].ToUTF8());
			Extensions->LinkEndChild(StartN);
			StartN->LinkEndChild(text5);

			TiXmlElement * EndN = new TiXmlElement("opencpn:end");
			TiXmlText * text6 = new TiXmlText(waypointName[n].ToUTF8());
			Extensions->LinkEndChild(EndN);
			EndN->LinkEndChild(text6);

			wxString sSpd = wxString::Format(_T("%5.1f"), speed);

			TiXmlElement * Pspd = new TiXmlElement("opencpn:planned_speed");
			TiXmlText * text7 = new TiXmlText(sSpd.ToUTF8());
			Extensions->LinkEndChild(Pspd);
			Pspd->LinkEndChild(text7);

			Route->LinkEndChild(Extensions);

			root->LinkEndChild(Route);
			// check if string ends with .gpx or .GPX
			if (!wxFileExists(s)) {
				//s = s + _T(".gpx");
			}
			wxCharBuffer buffer = s.ToUTF8();
			if (dbg) std::cout << buffer.data() << std::endl;
			doc.SaveFile(buffer.data());

		}

	}

	wxMessageBox(_("ETA Routes have been calculated!"));
}


void otidalplanUIDialog::Addpoint(TiXmlElement* Route, wxString ptlat, wxString ptlon, wxString ptname, wxString ptsym, wxString pttype, wxString ptviz, wxString ptTime) {
	//add point
	TiXmlElement * RoutePoint = new TiXmlElement("rtept");
	RoutePoint->SetAttribute("lat", ptlat.mb_str());
	RoutePoint->SetAttribute("lon", ptlon.mb_str());


	TiXmlElement * Name = new TiXmlElement("name");
	TiXmlText * text = new TiXmlText(ptname.ToUTF8());
	RoutePoint->LinkEndChild(Name);
	Name->LinkEndChild(text);

	TiXmlElement * Desc = new TiXmlElement("desc");
	TiXmlText * text3 = new TiXmlText(ptTime.mb_str());
	RoutePoint->LinkEndChild(Desc);
	Desc->LinkEndChild(text3);

	TiXmlElement * Symbol = new TiXmlElement("sym");
	TiXmlText * text1 = new TiXmlText(ptsym.mb_str());
	RoutePoint->LinkEndChild(Symbol);
	Symbol->LinkEndChild(text1);

	TiXmlElement * Type = new TiXmlElement("type");
	TiXmlText * text2 = new TiXmlText(pttype.mb_str());
	RoutePoint->LinkEndChild(Type);
	Type->LinkEndChild(text2);

	TiXmlElement * Extensions2 = new TiXmlElement("extensions");

	TiXmlElement * ExtensionsViz = new TiXmlElement("opencpn:viz");
	TiXmlText * textViz = new TiXmlText(ptviz);
	ExtensionsViz->LinkEndChild(textViz);
	Extensions2->LinkEndChild(ExtensionsViz);

	RoutePoint->LinkEndChild(Extensions2);
	
	Route->LinkEndChild(RoutePoint);
	//done adding point
}


int otidalplanUIDialog::GetRandomNumber(int range_min, int range_max)
{
	long u = (long)wxRound(((double)rand() / ((double)(RAND_MAX)+1) * (range_max - range_min)) + range_min);
	return (int)u;
}


/* C   - Sea Current Direction over ground
VC  - Velocity of Current

provisions to compute boat movement over ground

BG  - boat direction over ground
BGV - boat speed over ground (gps velocity)  
*/


void otidalplanUIDialog::OverGround(double B, double VB, double C, double VC, double &BG, double &VBG)
{
	if (VC == 0) { // short-cut if no currents
		BG = B, VBG = VB;
		return;
	}

	double Cx = VC * cos(deg2rad(C));
	double Cy = VC * sin(deg2rad(C));
	double BGx = VB * cos(deg2rad(B)) + Cx;
	double BGy = VB * sin(deg2rad(B)) + Cy;
	BG = rad2deg(atan2(BGy, BGx));
	VBG = distance(BGx, BGy);
}

double otidalplanUIDialog::AttributeDouble(TiXmlElement *e, const char *name, double def)
{
	const char *attr = e->Attribute(name);
	if (!attr)
		return def;
	char *end;
	double d = strtod(attr, &end);
	if (end == attr)
		return def;
	return d;
}

bool otidalplanUIDialog::OpenXML(wxString filename, bool reportfailure)
{
	Position pos;
	list<Position> m_pos;

	TiXmlDocument doc;
	wxString error;

	wxFileName fn(filename);

	SetTitle(_("otidalplan"));

	wxProgressDialog *progressdialog = NULL;
	wxDateTime start = wxDateTime::UNow();

	if (!doc.LoadFile(filename.mb_str()))
		FAIL(_("Failed to load file."));
	else {
		TiXmlHandle root(doc.RootElement());

		if (strcmp(root.Element()->Value(), "OpenCPNotidalplanConfiguration"))
			FAIL(_("Invalid xml file"));

		Positions.clear();

		int count = 0;
		for (TiXmlElement* e = root.FirstChild().Element(); e; e = e->NextSiblingElement())
			count++;

		int i = 0;
		for (TiXmlElement* e = root.FirstChild().Element(); e; e = e->NextSiblingElement(), i++) {
			if (progressdialog) {
				if (!progressdialog->Update(i))
					return true;
			}
			else {
				wxDateTime now = wxDateTime::UNow();
				/* if it's going to take more than a half second, show a progress dialog */
				if ((now - start).GetMilliseconds() > 250 && i < count / 2) {
					progressdialog = new wxProgressDialog(
						_("Load"), _("otidalplan"), count, this,
						wxPD_CAN_ABORT | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);
				}
			}

			if (!strcmp(e->Value(), "Position")) {
				wxString name = wxString::FromUTF8(e->Attribute("Name"));
				double lat = AttributeDouble(e, "Latitude", NAN);
				double lon = AttributeDouble(e, "Longitude", NAN);

				for (std::vector<RouteMapPosition>::iterator it = Positions.begin();
					it != Positions.end(); it++) {
					if ((*it).Name == name) {
						static bool warnonce = true;
						if (warnonce) {
							warnonce = false;
							wxMessageDialog mdlg(this, _("File contains duplicate position name, discarding\n"),
								_("otidalplan"), wxOK | wxICON_WARNING);
							mdlg.ShowModal();
						}

						goto skipadd;
					}
				}


			skipadd:;

			}

			else

				if (!strcmp(e->Value(), "TidalRoute")) {
					TidalRoute tr;
					m_pos.clear();
					wxString nm = wxString::FromUTF8(e->Attribute("Name"));
					wxString tp = wxString::FromUTF8(e->Attribute("Type"));
					wxString st = wxString::FromUTF8(e->Attribute("Start"));
					wxString en = wxString::FromUTF8(e->Attribute("End"));
					wxString tm = wxString::FromUTF8(e->Attribute("Time"));
					wxString tms = wxString::FromUTF8(e->Attribute("StartTime"));
					wxString tme = wxString::FromUTF8(e->Attribute("EndTime"));
					wxString dn = wxString::FromUTF8(e->Attribute("Distance"));
					tr.Name = nm;
					tr.Type = tp;
					tr.Start = st;
					tr.End = en;
					tr.Time = tm;
					tr.StartTime = tms;
					tr.EndTime = tme;
					tr.Distance = dn;

					for (TiXmlElement* f = e->FirstChildElement(); f; f = f->NextSiblingElement()) {

						if (!strcmp(f->Value(), "Route")) {
							pos.name = wxString::FromUTF8(f->Attribute("Waypoint"));
							pos.lat = wxString::FromUTF8(f->Attribute("Latitude"));
							pos.lon = wxString::FromUTF8(f->Attribute("Longitude"));
							pos.time = wxString::FromUTF8(f->Attribute("ETD"));
							pos.guid = wxString::FromUTF8(f->Attribute("GUID"));
							pos.CTS = wxString::FromUTF8(f->Attribute("CTS"));
							pos.SMG = wxString::FromUTF8(f->Attribute("SMG"));
							pos.distTo = wxString::FromUTF8(f->Attribute("Dist"));
							pos.brgTo = wxString::FromUTF8(f->Attribute("Brng"));
							pos.set = wxString::FromUTF8(f->Attribute("Set"));
							pos.rate = wxString::FromUTF8(f->Attribute("Rate"));
							pos.icon_name = wxString::FromUTF8(f->Attribute("icon_name"));

							m_pos.push_back(pos);

						}
					}
					tr.m_positionslist = m_pos;					
					AddTidalRoute(tr);
				}

				else					
				FAIL(_("Unrecognized xml node"));
		}
	}

	delete progressdialog;
	return true;
failed:

	if (reportfailure) {
		wxMessageDialog mdlg(this, error, _("otidalplan"), wxOK | wxICON_ERROR);
		mdlg.ShowModal();
	}
	return false;
	
}

void otidalplanUIDialog::SaveXML(wxString filename)
{
	TiXmlDocument doc;
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "");
	doc.LinkEndChild(decl);

	TiXmlElement * root = new TiXmlElement("OpenCPNotidalplanConfiguration");
	doc.LinkEndChild(root);

	char version[24];
	sprintf(version, "%d.%d", PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
	root->SetAttribute("version", version);
	root->SetAttribute("creator", "Opencpn otidalplan plugin");

	for (std::vector<RouteMapPosition>::iterator it = Positions.begin();
		it != Positions.end(); it++) { 
		TiXmlElement *c = new TiXmlElement("Position");

		c->SetAttribute("Name", (*it).Name.mb_str(wxConvUTF8));
		c->SetAttribute("Latitude", wxString::Format(_T("%.5f"), (*it).lat).mb_str());
		c->SetAttribute("Longitude", wxString::Format(_T("%.5f"), (*it).lon).mb_str());

		root->LinkEndChild(c);
	}
	for (std::list<TidalRoute>::iterator it = m_TidalRoutes.begin();
		it != m_TidalRoutes.end(); it++) {

			TiXmlElement * TidalRoute = new TiXmlElement("TidalRoute");
			TidalRoute->SetAttribute("Name", (*it).Name.mb_str(wxConvUTF8));
			TidalRoute->SetAttribute("Type", (*it).Type);
			TidalRoute->SetAttribute("Start", (*it).Start);
			TidalRoute->SetAttribute("End", (*it).End);
			TidalRoute->SetAttribute("Time", (*it).Time);
			TidalRoute->SetAttribute("StartTime", (*it).StartTime);
			TidalRoute->SetAttribute("EndTime", (*it).EndTime);
			TidalRoute->SetAttribute("Distance", (*it).Distance);

			for (std::list<Position>::iterator itp = (*it).m_positionslist.begin();
				itp != (*it).m_positionslist.end(); itp++) {
				TiXmlElement *cp = new TiXmlElement("Route");

				cp->SetAttribute("Waypoint", (*itp).name.mb_str(wxConvUTF8));
				cp->SetAttribute("Latitude", (*itp).lat);
				cp->SetAttribute("Longitude", (*itp).lon);
				cp->SetAttribute("ETD", (*itp).time);
				cp->SetAttribute("GUID", (*itp).guid);
				cp->SetAttribute("CTS", (*itp).CTS);
				cp->SetAttribute("SMG", (*itp).SMG);
				cp->SetAttribute("Dist", (*itp).distTo);
				cp->SetAttribute("Brng", (*itp).brgTo);
				cp->SetAttribute("Set", (*itp).set);
				cp->SetAttribute("Rate", (*itp).rate);
				cp->SetAttribute("icon_name", (*itp).icon_name);
		
				TidalRoute->LinkEndChild(cp);
			}

		root->LinkEndChild(TidalRoute);
	}

	if (!doc.SaveFile(filename.mb_str())) {
		wxMessageDialog mdlg(this, _("Failed to save xml file: ") + filename,
			_("otidalplan"), wxOK | wxICON_ERROR);
		mdlg.ShowModal();
	}
}

void otidalplanUIDialog::SelectRoutePoints(wxString routeName) {

	my_positions.clear();

	for (std::vector<rte>::iterator it = my_routes.begin(); it != my_routes.end(); it++) {

		if (routeName == (*it).Name) {
			Position myPosition;
			for (std::vector<rtept>::iterator it2 = (*it).m_rteptList.begin(); it2 != (*it).m_rteptList.end(); it2++) {												

				myPosition.lat = (*it2).lat;
				myPosition.lon = (*it2).lon;
				myPosition.name = (*it2).Name;				
				myPosition.visible = (*it2).visible;
				my_positions.push_back(myPosition);
			}

		}

	}
}

wxString otidalplanUIDialog::SelectRoute(bool isDR) {	

	ReadNavobj();

	GetRouteDialog RouteDialog(this, -1, _("Select the route to follow"), wxPoint(200, 200), wxSize(300, 200), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

	RouteDialog.dialogText->InsertColumn(0, _T(""), 0, wxLIST_AUTOSIZE);
	RouteDialog.dialogText->SetColumnWidth(0, 290);
	RouteDialog.dialogText->InsertColumn(1, _T(""), 0, wxLIST_AUTOSIZE);
	RouteDialog.dialogText->SetColumnWidth(1, 0);
	RouteDialog.dialogText->DeleteAllItems();

	int in = 0;
	wxString routeName = _T("");
	for (std::vector<rte>::iterator it = my_routes.begin(); it != my_routes.end(); it++) {

		routeName = (*it).Name;

		RouteDialog.dialogText->InsertItem(in, _T(""), -1);
		RouteDialog.dialogText->SetItem(in, 0, routeName);
		in++;
	}
	this->Fit();
	this->Refresh();

	long si = -1;
	long itemIndex = -1;
	int f = 0;

	wxListItem     row_info;
	wxString       cell_contents_string = wxEmptyString;
	bool foundRoute;
	routeLeg myLeg;
	rtept myRoutePoint;

	mySelectedRoute = wxEmptyString; // initialise to prevent a crash

	if (RouteDialog.ShowModal() == wxID_OK) {
	
		for (;;) {
			itemIndex = RouteDialog.dialogText->GetNextItem(itemIndex,
				wxLIST_NEXT_ALL,
				wxLIST_STATE_SELECTED);

			if (itemIndex == -1) break;

			// Got the selected item index
			if (RouteDialog.dialogText->IsSelected(itemIndex)) {
				si = itemIndex;
				foundRoute = true;
				break;
			}
		}

		if (foundRoute) {

			// Set what row it is (m_itemId is a member of the regular wxListCtrl class)
			row_info.m_itemId = si;
			// Set what column of that row we want to query for information.
			row_info.m_col = 0;
			// Set text mask
			row_info.m_mask = wxLIST_MASK_TEXT;

			// Get the info and store it in row_info variable.   
			RouteDialog.dialogText->GetItem(row_info);
			// Extract the text out that cell
			cell_contents_string = row_info.m_text;

			SelectRoutePoints(cell_contents_string);  // makes route waypoints for use in the eta calculations

			double value;
			rtept initPoint;

			for (std::vector<rte>::iterator it = my_routes.begin(); it != my_routes.end(); it++) {
				wxString routeName = (*it).Name;
				if (routeName == cell_contents_string) {
					mySelectedRoute = routeName;
					routePoints = (*it).m_rteptList;
					nextRoutePointIndex = 0;
					int iName = 1;
					routeLegs.clear();
					myLeg.m_rteptList.clear();

					countRoutePoints = 0;
					for (std::vector<rtept>::iterator it = routePoints.begin(); it != routePoints.end(); it++)
						countRoutePoints++;

					

					for (std::vector<rtept>::iterator it = routePoints.begin(); it != routePoints.end(); it++) {

						wxString sIndex = wxString::Format(_T("%i"), (*it).index);

						if ((*it).index != 0) {
							if (nextRoutePointIndex == 0) {

								(*it).lat.ToDouble(&value);
								initLat = value;
								myRoutePoint.lat = (*it).lat;

								(*it).lon.ToDouble(&value);
								initLon = value;
								myRoutePoint.lon = (*it).lon;

								myRoutePoint.Name = (*it).Name;

								wxString isviz = (*it).visible;								
								if ( isviz == wxEmptyString) {
									myRoutePoint.visible = "1";
								}
								else {
									myRoutePoint.visible = (*it).visible;
								}

								myRoutePoint.sym = (*it).sym;
								if (myRoutePoint.sym == "Empty" || myRoutePoint.sym == "Symbol-Empty" || myRoutePoint.sym == "empty") {
									myRoutePoint.visible = "0";
								}

								nextRoutePointIndex = 1;
								myLeg.m_rteptList.push_back(myRoutePoint);
							}

							if (nextRoutePointIndex != 0) {

								(*it).lat.ToDouble(&value);
								nextLat = value;
								myRoutePoint.lat = (*it).lat;

								(*it).lon.ToDouble(&value);
								nextLon = value;
								myRoutePoint.lon = (*it).lon;

								

								myLeg.LegName = wxString::Format("%i", iName);
								myLeg.m_rteptList.push_back(myRoutePoint);

								routeLegs.push_back(myLeg);

								nextRoutePointIndex = 0;
								iName++;
								myLeg.m_rteptList.clear();

							}
						}
					}
				}
			}	

		}
		else {
			wxMessageBox(_T("Route not found"));
			
			return wxEmptyString;
		}		

		return mySelectedRoute;
	}
	else {
		return wxEmptyString;
    }
	
}

wxString otidalplanUIDialog::StandardPath()
{
	wxStandardPathsBase& std_path = wxStandardPathsBase::Get();
	wxString s = wxFileName::GetPathSeparator();

#if defined(__WXMSW__)
	wxString stdPath = std_path.GetConfigDir();
#elif defined(__WXGTK__) || defined(__WXQT__)
	wxString stdPath = std_path.GetUserDataDir();
#elif defined(__WXOSX__)
	wxString stdPath = (std_path.GetUserConfigDir() + s + _T("opencpn"));
#endif


#ifdef __WXOSX__
	// Compatibility with pre-OCPN-4.2; move config dir to
	// ~/Library/Preferences/opencpn if it exists
	wxString oldPath = (std_path.GetUserConfigDir() + s);
	if (wxDirExists(oldPath) && !wxDirExists(stdPath)) {
		wxLogMessage("ShipDriver_pi: moving config dir %s to %s", oldPath, stdPath);
		wxRenameFile(oldPath, stdPath);
	}
#endif

	stdPath += s; // is this necessary?
	return stdPath;
}

void otidalplanUIDialog::OnContextMenu(double m_lat, double m_lon) {

	GetRouteDialog  LegDialog(this, -1, _("Select the route leg"), wxPoint(200, 200), wxSize(300, 200), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	LegDialog.SetTitle("Route Legs");
	LegDialog.dialogText->InsertColumn(0, _T(""), 0, wxLIST_AUTOSIZE);
	LegDialog.dialogText->SetColumnWidth(0, 290);
	LegDialog.dialogText->InsertColumn(1, _T(""), 0, wxLIST_AUTOSIZE);
	LegDialog.dialogText->SetColumnWidth(1, 0);
	LegDialog.dialogText->DeleteAllItems();
	

	int in = 0;
	wxString routeName = _T("");
	for (std::vector<routeLeg>::iterator it = routeLegs.begin(); it != routeLegs.end(); it++) {

		routeName = (*it).LegName;

		LegDialog.dialogText->InsertItem(in, _T(""), -1);
		LegDialog.dialogText->SetItem(in, 0, routeName);
		in++;
	}
	this->Fit();
	this->Refresh();

	long si = -1;
	long itemIndex = -1;
	int f = 0;
	int fc = 0;
	wxListItem     row_info;
	int value = 1;

	if (LegDialog.ShowModal() == wxID_OK) {
		
		fc = FindTCurrentStation(m_lat, m_lon, 10);

		int g = LegDialog.dialogText->GetItemCount();
		int p = 0;
		for ( p; p<g ; p++) {
			itemIndex = LegDialog.dialogText->GetNextItem(itemIndex,
				wxLIST_NEXT_ALL,
				wxLIST_STATE_SELECTED);

			if (itemIndex == -1) break;

			// Got the selected item index
			if (LegDialog.dialogText->IsSelected(itemIndex)) {
				si = itemIndex;
				fc = FindTCurrentStation(m_lat, m_lon, 10);

				row_info.m_itemId = si;
				// Set what column of that row we want to query for information.
				row_info.m_col = 0;
				// Set text mask
				row_info.m_mask = wxLIST_MASK_TEXT;
				// Get the info and store it in row_info variable.   
				LegDialog.dialogText->GetItem(row_info);
				// Extract the text out that cell
				value = wxAtoi(row_info.m_text);

				m_legTCurrent.m_LegNumber = value;
				m_legTCurrent.m_TCRefNumber = fc;
				m_legTCurrent.lat = m_tcLat;
				m_legTCurrent.lon = m_tcLon;

				rteTCurrents.push_back(m_legTCurrent);
			}
		}	

		
	}

	initLat = m_lat;
	initLon = m_lon;
}

void otidalplanUIDialog::LoadTCMFile()
{
	//delete m_ptcmgr;
	wxString TCDir = m_FolderSelected;
	TCDir.Append(wxFileName::GetPathSeparator());
	wxLogMessage(_("Using Tide/Current data from:  ") + TCDir);
	
	wxString cache_locn = TCDir;
	m_ptcmgr = new TCMgr(TCDir, cache_locn);
}

TotalTideArrow otidalplanUIDialog::FindDummyTCurrent(int refNum) {

	TotalTideArrow tta;
	tta.lat = 0;
	tta.lon = 0;
	
	pIDX = m_ptcmgr->GetIDX_entry(refNum);

	char type = pIDX->IDX_type;             // Entry "TCtcIUu" identifier
	if ((type == 'c') || (type == 'C'))  // only Currents
	{

		tta.lat = pIDX->IDX_lat;
		tta.lon = pIDX->IDX_lon;
	}

	return tta;

}

int otidalplanUIDialog::FindTCurrentStation(double m_lat, double m_lon, double searchDist) {
	bool foundTC = false;
	double radius = 0.1;
	double dist = 0;
	char N = 'N';
	int c = 0;
	wxString dimensions = wxT(""), s;
	wxString exPort = wxT("");

	wxListItem     row_info;
	wxString       cell_contents_string = wxEmptyString;

	double lat = 50;
	double lon = -4;
	m_tcNum = 0;

	bool newItem = false;

	int i;

	while (!foundTC) {
		for (i = 1; i < m_ptcmgr->Get_max_IDX() + 1; i++)
		{
			pIDX = m_ptcmgr->GetIDX_entry(i);

			char type = pIDX->IDX_type;             // Entry "TCtcIUu" identifier
			if ((type == 'c') || (type == 'C'))  // only Currents
			{

				lat = pIDX->IDX_lat;
				lon = pIDX->IDX_lon;

				dist = distanceLatLon(lat, lon, m_lat, m_lon, N);
				if (dist < radius) {
					wxString locn(pIDX->IDX_station_name, wxConvUTF8);
					wxString locna, locnb;
					if (locn.Contains(wxString(_T(",")))) {
						locna = locn.BeforeFirst(',');
						locnb = locn.AfterFirst(',');
					}
					else {
						locna = locn;
						locnb.Empty();
					}
					m_tcNum = pIDX->IDX_rec_num;
					m_tcLat = pIDX->IDX_lat;
					m_tcLon = pIDX->IDX_lon;
					foundTC = true;	
					break;
				}
			}

		}

		radius = radius + 0.1;
		if (radius > searchDist) {
			wxString notFound = _("No Current Stations found within 200NM");
			return 0;
		}

		

	}
	return m_tcNum;
}

int otidalplanUIDialog::FindClosestDummyTCurrent(wxString rteName, double m_lat, double m_lon, double maxDistance) {

	double tcDistance = 1000;
	double rLat, rLon;
	double compDistance = 1;
	int tcRefNum = 0;
	int itNum = 0;
	char N = 'N';
	bool foundTC = false;

	if (dummyTC.size() == 0) {
		wxMessageBox("No currents have been allocated");
		return 0;
	}

	while (!foundTC) {

		for (vector<tc>::iterator it = dummyTC.begin(); it != dummyTC.end(); it++) {

				if ((*it).routeName != rteName) {
					wxMessageBox("Incorrect currents for this route");
					return 9999;
				}
			
				tcRefNum = (*it).tcRef;

				rLat = (*it).lat;
				rLon = (*it).lon;
				tcDistance = distanceLatLon(m_lat, m_lon, rLat, rLon, N);
				if (tcDistance < compDistance) {
					foundTC = true;
					break;
				}
			
	
		}

		compDistance = compDistance + 1;
		if (compDistance > maxDistance) {
			wxString notFound = _("No Tidal Current Station found within 100NM");
			return 0;
		}

	}

	return tcRefNum;

}


int otidalplanUIDialog::FindClosestTCurrentStation(int legNum, double m_lat, double m_lon) {

	double tcDistance = 1000;
	double rLat, rLon;
	double compDistance = 1;
	int tcRefNum = 0;
	int itNum = 0;
	char N = 'N';
	bool foundTC = false;

	if (rteTCurrents.size() == 0) {
		wxMessageBox("No currents have been allocated");
		return 0;
	}

	while (!foundTC) {

		for (vector<routeCurrent>::iterator it = rteTCurrents.begin(); it != rteTCurrents.end(); it++) {

			itNum = (*it).m_LegNumber;
			
			if (itNum == legNum) {

				tcRefNum = (*it).m_TCRefNumber;
				rLat = (*it).lat;
				rLon = (*it).lon;
				tcDistance = distanceLatLon(m_lat, m_lon, rLat, rLon, N);
				if (tcDistance < compDistance) {
					foundTC = true;
					break;
				}

			}

		}

		compDistance = compDistance + 1;
		if (compDistance > 100) {
			wxString notFound = _("No Tidal Current Station found within 100NM");
			break;
		}

	}

	return tcRefNum;

}

double otidalplanUIDialog::distanceLatLon(double lat1, double lon1, double lat2, double lon2, char unit) {
	double theta, dist;
	theta = lon1 - lon2;
	dist = sin(deg2rad(lat1)) * sin(deg2rad(lat2)) + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * cos(deg2rad(theta));
	dist = acos(dist);
	dist = rad2deg(dist);
	dist = dist * 60 * 1.1515;
	switch (unit) {
	case 'M':
		break;
	case 'K':
		dist = dist * 1.609344;
		break;
	case 'N':
		dist = dist * 0.8684;
		break;
	}
	return (dist);
}
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:: This function converts decimal degrees to radians :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double otidalplanUIDialog::deg2rad(double deg) {
	return (deg * pi / 180);
}
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:: This function converts radians to decimal degrees :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double otidalplanUIDialog::rad2deg(double rad) {
	return (rad * 180 / pi);
};





enum { WIND, CURRENT };

