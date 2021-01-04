/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  otidalplan Plugin Friends
 * Author:   David Register, Mike Rossiter
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 */

#ifndef __otidalplanUIDIALOG_H__
#define __otidalplanUIDIALOG_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include <wx/fileconf.h>
#include <wx/glcanvas.h>

#include "otidalplanUIDialogBase.h"
#include "routeprop.h"
#include "NavFunc.h"

#include <wx/progdlg.h>
#include <list>
#include <vector>
#include "tcmgr.h"
#include "wx/dateevt.h"
#include "wx/stattext.h"
#include "ocpn_plugin.h"
#include "wx/dialog.h"
#include <wx/calctrl.h>
#include "wx/window.h"
#include <wx/colordlg.h>
#include <wx/event.h>
#include "tinyxml.h"
#include <wx/scrolwin.h>
#include <wx/datetime.h>
#include <wx/thread.h>
#include <wx/event.h>
#include <wx/listctrl.h>
#include "tableroutes.h"

/* XPM */
static const char *eye[] = {
	"20 20 7 1",
	". c none",
	"# c #000000",
	"a c #333333",
	"b c #666666",
	"c c #999999",
	"d c #cccccc",
	"e c #ffffff",
	"....................",
	"....................",
	"....................",
	"....................",
	".......######.......",
	".....#aabccb#a#.....",
	"....#deeeddeebcb#...",
	"..#aeeeec##aceaec#..",
	".#bedaeee####dbcec#.",
	"#aeedbdabc###bcceea#",
	".#bedad######abcec#.",
	"..#be#d######dadb#..",
	"...#abac####abba#...",
	".....##acbaca##.....",
	".......######.......",
	"....................",
	"....................",
	"....................",
	"....................",
	"...................." };


using namespace std;

#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

#define RT_RCDATA2           MAKEINTRESOURCE(999)

/* Maximum value that can be returned by the rand function. */
#ifndef RAND_MAX
#define RAND_MAX 0x7fff
#endif

#define distance(X, Y) sqrt((X)*(X) + (Y)*(Y)) // much faster than hypot#define distance(X, Y) sqrt((X)*(X) + (Y)*(Y)) // much faster than hypot

class otidalplanOverlayFactory;
class PlugIn_ViewPort;
class PositionRecordSet;


class wxFileConfig;
class otidalplan_pi;
class wxGraphicsContext;
class routeprop;
class TableRoutes;
class ConfigurationDialog;
class NewPositionDialog;

class rtept
{
public:

	wxString Name, m_GUID;
	int index;
	wxString lat, lon;
	wxString visible;
	wxString sym;

};

class rte
{
public:

	wxString Name;

	vector<rtept> m_rteptList;

};

class routeCurrent
{
public:
	int m_LegNumber;
	int m_TCRefNumber;
	double lat, lon;
};

class routeLeg
{
public:

	wxString LegName;
	vector<rtept> m_rteptList;
	

};


class Position
{
public:
    wxString lat, lon, wpt_num;
	wxString name;
	wxString guid;
	wxString time;
	wxString etd;
	wxString CTS;
	wxString SMG;
	wxString distTo;
	wxString brgTo;
	wxString set;
	wxString rate;
	wxString icon_name;
	bool show_name;
	wxString visible;
	int routepoint;

};

class tc{

public:
	double lat, lon;
	int tcRef;
	int legNum;
	double tcLat, tcLon;
	wxString routeName;
};

struct RouteMapPosition {
	RouteMapPosition(wxString n, double lat0, double lon0)
		: Name(n), lat(lat0), lon(lon0) {}
public:
	wxString Name;
	double lat, lon;
};


struct Arrow
{
	wxDateTime m_dt;
	double m_dir;
	double m_force;
	double m_lat;
	double m_lon;
	double m_cts;
	double m_tforce;

};

struct TotalTideArrow
{
	double lat;
	double lon;
	double m_dir;
	double m_force;
};

class TidalRoute
{
public:
	
	wxString Name, Type, Start, StartTime, End, EndTime, Time, Distance, m_GUID;
	list<Position> m_positionslist;
	
};

#define pi 3.14159265358979323846


class otidalplanUIDialog: public otidalplanUIDialogBase {
public:

    otidalplanUIDialog(wxWindow *parent, otidalplan_pi *ppi);
    ~otidalplanUIDialog();

	void OpenFile( bool newestFile = false );
	void OnFolderSelChanged(wxFileDirPickerEvent& event);
    
    void SetCursorLatLon( double lat, double lon );

    void SetViewPort( PlugIn_ViewPort *vp );
	PlugIn_ViewPort *vp;

	wxDateTime m_dtNow;
	double m_dInterval;

	bool onNext;
	bool onPrev;

    wxString m_FolderSelected;
	int m_IntervalSelected;
	
	time_t myCurrentTime; 

	wxString MakeDateTimeLabel(wxDateTime myDateTime);
	void OnInformation(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnShowRouteTable();
	void GetTable(wxString myRoute);
	void AddChartRoute(wxString myRoute);
	void AddTidalRoute(TidalRoute tr);

	virtual void Lock() { routemutex.Lock(); }
	virtual void Unlock() { routemutex.Unlock(); }
	void OverGround(double B, double VB, double C, double VC, double &BG, double &VBG);
	bool OpenXML(wxString filename, bool reportfailure);
	void SaveXML(wxString filename);

	wxString StandardPath();
	void OnContextMenu(double m_lat, double m_lon);
	void LoadTCMFile();
	TotalTideArrow FindDummyTCurrent(int refNum);
	int FindClosestDummyTCurrent(wxString rteName, double m_lat, double m_lon, double maxDistance);
	int FindTCurrentStation(double m_lat, double m_lon, double searchDist);
	int FindClosestTCurrentStation(int legNum, double m_lat, double m_lon);
	
	double distanceLatLon(double lat1, double lon1, double lat2, double lon2, char unit);
	double deg2rad(double deg);
	double rad2deg(double rad);
	wxString SelectRoute(bool isDR);
	void SelectRoutePoints(wxString routeName);

	int m_tcNum;
	double m_tcLat, m_tcLon;

	double AttributeDouble(TiXmlElement *e, const char *name, double def);
	vector<RouteMapPosition>Positions;
	wxString m_default_configuration_path;
	list<Arrow> m_arrowList;
	list<Arrow> m_cList;
	list<TotalTideArrow> m_totaltideList;
	list<TidalRoute> m_TidalRoutes;
	bool b_showTidalArrow;
	RouteProp* routetable;
	ConfigurationDialog m_ConfigurationDialog;

	vector<Position> my_positions;
	vector<Position> my_points;

	vector<tc> dummyTC;
	bool b_showCurrentIndicator;
	bool b_showAttachmentLegs;

	wxString rte_start;
	wxString rte_end;
	
	void Addpoint(TiXmlElement* Route, wxString ptlat, wxString ptlon, wxString ptname, wxString ptsym, wxString pttype, wxString ptviz, wxString ptTime);

protected:
	


private:

	double myDist;

	double initLat;
	double initLon;
	double nextLat;
	double nextLon;

	int nextRoutePointIndex;
	double nextRoutePoint;
	double followDir;
	int countRoutePoints;
	wxMutex routemutex;

    void OnClose( wxCloseEvent& event );
    void OnMove( wxMoveEvent& event );
    void OnSize( wxSizeEvent& event );

	void OnSummary(wxCommandEvent& event);
	void OnShowTables(wxCommandEvent& event);

	vector<rte> my_routes;
	vector<rtept> routePoints;
	vector<routeLeg> routeLegs;

	routeCurrent m_legTCurrent;
	vector<routeCurrent>rteTCurrents;

	TotalTideArrow tcForLeg;
	TotalTideArrow tcCalculate(wxDateTime tcdt, int tcInt);

	
	void OnDeleteAllRoutes(wxCommandEvent& event);

	void OnAttachCurrents(wxCommandEvent& event);

	wxString mySelectedRoute;

	void DummyDR(wxCommandEvent& event, bool write_file, int Pattern);
	void CalcTimedDR(wxCommandEvent& event, bool write_file, int Pattern);
	void CalcTimedETA(wxCommandEvent& event, bool write_file, int Pattern);
	void DummyTimedDR(wxCommandEvent& event, bool write_file, int Pattern);

	wxDateTime AdvanceSeconds(wxDateTime currentTime, double HoursToAdvance);

	double ReadNavobj();
	
	void DRCalculate(wxCommandEvent& event);
	void ETACalculate(wxCommandEvent& event);

	int GetRandomNumber(int range_min, int range_max);
	

    //    Data
    wxWindow *pParent;
	otidalplan_pi *pPlugIn;

    PlugIn_ViewPort  *m_vp;
 
    double m_cursor_lat, m_cursor_lon;
	wxString        g_SData_Locn;
	TCMgr           *m_ptcmgr;
	IDX_entry		*pIDX;
	wxString        *pTC_Dir;

	bool error_found;
	bool dbg;
	
	
	wxString waypointName[2000];
	wxString waypointVisible[2000];

};

class GetRouteDialog : public wxDialog
{
public:

	GetRouteDialog(wxWindow * parent, wxWindowID id, const wxString & title = "Routes available",
		const wxPoint & pos = wxDefaultPosition,
		const wxSize & size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE);

	~GetRouteDialog();

	wxStdDialogButtonSizer* m_sdbSizer1;
	wxButton* m_sdbSizer1OK;
	wxButton* m_sdbSizer1Cancel;
	wxListView * dialogText;

	void OnCancel();

protected:
	
	

private:

};


#endif

