/**************************************************************************
*
* Project:  OpenCPN
* Purpose:  RouteProperties Support
* Author:   David Register
*
***************************************************************************
*   Copyright (C) 2010 by David S. Register                               *
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
**************************************************************************/

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/datetime.h>
#include <wx/clipbrd.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/stattext.h>
#include <wx/clrpicker.h>
#include <wx/event.h>

#include "routeprop.h"


/*!
* Helper stuff for calculating Route Plans
*/

#define    pi        (4.*atan(1.0))
#define    tpi        (2.*pi)
#define    twopi    (2.*pi)
#define    degs    (180./pi)
#define    rads    (pi/180.)

#define    MOTWILIGHT    1    // in some languages there may be a distinction between morning/evening
#define    SUNRISE        2
#define    DAY            3
#define    SUNSET        4
#define    EVTWILIGHT    5
#define    NIGHT        6

/* Next high tide, low tide, transition of the mark level, or some
combination.
Bit      Meaning
0       low tide
1       high tide
2       falling transition
3       rising transition
*/

#define    LW    1
#define    HW    2
#define    FALLING    4
#define    RISING    8

char tide_status[][8] = {
    " LW ",
    " HW ",
    " ~~v ",
    " ~~^ "

};

// Sunrise/twilight calculation for route properties.
// limitations: latitude below 60, year between 2000 and 2100
// riset is +1 for rise -1 for set
// adapted by author's permission from QBASIC source as published at
//     http://www.stargazing.net/kepler

wxString GetDaylightString(int index)
{
    switch (index)
    {
        case 0:
            return      _T(" - ");
        case 1:
            return      _("MoTwilight");
        case 2:
            return      _("Sunrise");
        case 3:
            return      _("Daytime");
        case 4:
            return      _("Sunset");
        case 5:
            return      _("EvTwilight");
        case 6:
            return      _("Nighttime");

        default:
            return      _T("");
    }
}


static double sign( double x )
{
    if( x < 0. ) return -1.;
    else
        return 1.;
}

static double FNipart( double x )
{
    return ( sign( x ) * (int) ( fabs( x ) ) );
}

static double FNday( int y, int m, int d, int h )
{
    long fd = ( 367 * y - 7 * ( y + ( m + 9 ) / 12 ) / 4 + 275 * m / 9 + d );
    return ( (double) fd - 730531.5 + h / 24. );
}

static double FNrange( double x )
{
    double b = x / tpi;
    double a = tpi * ( b - FNipart( b ) );
    if( a < 0. ) a = tpi + a;
    return ( a );
}

double getDaylightEvent( double glat, double glong, int riset, double altitude, int y, int m,
        int d )
{
    double day = FNday( y, m, d, 0 );
    double days, correction;
    double utold = pi;
    double utnew = 0.;
    double sinalt = sin( altitude * rads ); // go for the sunrise/sunset altitude first
    double sinphi = sin( glat * rads );
    double cosphi = cos( glat * rads );
    double g = glong * rads;
    double t, L, G, ec, lambda, E, obl, delta, GHA, cosc;
    int limit = 12;
    while( ( fabs( utold - utnew ) > .001 ) ) {
        if( limit-- <= 0 ) return ( -1. );
        days = day + utnew / tpi;
        t = days / 36525.;
        //     get arguments of Sun's orbit
        L = FNrange( 4.8949504201433 + 628.331969753199 * t );
        G = FNrange( 6.2400408 + 628.3019501 * t );
        ec = .033423 * sin( G ) + .00034907 * sin( 2 * G );
        lambda = L + ec;
        E = -1. * ec + .0430398 * sin( 2 * lambda ) - .00092502 * sin( 4. * lambda );
        obl = .409093 - .0002269 * t;
        delta = asin( sin( obl ) * sin( lambda ) );
        GHA = utold - pi + E;
        cosc = ( sinalt - sinphi * sin( delta ) ) / ( cosphi * cos( delta ) );
        if( cosc > 1. ) correction = 0.;
        else
            if( cosc < -1. ) correction = pi;
            else
                correction = acos( cosc );
        double tmp = utnew;
        utnew = FNrange( utold - ( GHA + g + riset * correction ) );
        utold = tmp;
    }
    return ( utnew * degs / 15. );    // returns decimal hours UTC
}

static double getLMT( double ut, double lon )
{
    double t = ut + lon / 15.;
    if( t >= 0. ) if( t <= 24. ) return ( t );
    else
        return ( t - 24. );
    else
        return ( t + 24. );
}

int getDaylightStatus( double lat, double lon, wxDateTime utcDateTime )
{
    if( fabs( lat ) > 60. ) return ( 0 );
    int y = utcDateTime.GetYear();
    int m = utcDateTime.GetMonth() + 1;  // wxBug? months seem to run 0..11 ?
    int d = utcDateTime.GetDay();
    int h = utcDateTime.GetHour();
    int n = utcDateTime.GetMinute();
    int s = utcDateTime.GetSecond();
    if( y < 2000 || y > 2100 ) return ( 0 );

    double ut = (double) h + (double) n / 60. + (double) s / 3600.;
    double lt = getLMT( ut, lon );
    double rsalt = -0.833;
    double twalt = -12.;

    //wxString msg;

    if( lt <= 12. ) {
        double sunrise = getDaylightEvent( lat, lon, +1, rsalt, y, m, d );
        if( sunrise < 0. ) return ( 0 );
        else
            sunrise = getLMT( sunrise, lon );

        //            msg.Printf(_T("getDaylightEvent lat=%f lon=%f\n riset=%d rsalt=%f\n y=%d m=%d d=%d\n sun=%f lt=%f\n ut=%f\n"),
        // lat, lon, +1, rsalt, y, m, d, sunrise, lt, ut);
        //msg.Append(utcDateTime.Format());
        //            OCPNMessageDialog md1(gFrame, msg, _("Sunrise Message"), wxICON_ERROR );
        //            md1.ShowModal();

        if( fabs( lt - sunrise ) < 0.15 ) return ( SUNRISE );
        if( lt > sunrise ) return ( DAY );
        double twilight = getDaylightEvent( lat, lon, +1, twalt, y, m, d );
        if( twilight < 0. ) return ( 0 );
        else
            twilight = getLMT( twilight, lon );
        if( lt > twilight ) return ( MOTWILIGHT );
        else
            return ( NIGHT );
    } else {
        double sunset = getDaylightEvent( lat, lon, -1, rsalt, y, m, d );
        if( sunset < 0. ) return ( 0 );
        else
            sunset = getLMT( sunset, lon );
        if( fabs( lt - sunset ) < 0.15 ) return ( SUNSET );
        if( lt < sunset ) return ( DAY );
        double twilight = getDaylightEvent( lat, lon, -1, twalt, y, m, d );
        if( twilight < 0. ) return ( 0 );
        else
            twilight = getLMT( twilight, lon );
        if( lt < twilight ) return ( EVTWILIGHT );
        else
            return ( NIGHT );
    }
}

#define    UTCINPUT         0
#define    LTINPUT          1    // i.e. this PC local time
#define    LMTINPUT         2    // i.e. the remote location LMT time
#define    INPUT_FORMAT     1
#define    DISPLAY_FORMAT   2
#define    TIMESTAMP_FORMAT 3

wxString ts2s(wxDateTime ts, int tz_selection, long LMT_offset, int format)
{
    wxString s = _T("");
    wxString f;
    if (format == INPUT_FORMAT) f = _T("%m/%d/%Y %H:%M");
    else if (format == TIMESTAMP_FORMAT) f = _T("%m/%d/%Y %H:%M:%S");
    else f = _T(" %m/%d %H:%M");
    switch (tz_selection) {
    case 0: s.Append(ts.Format(f));
        if (format != INPUT_FORMAT) s.Append(_T(" UT"));
        break;
    case 1: s.Append(ts.FromUTC().Format(f)); break;
    case 2:
        wxTimeSpan lmt(0,0,(int)LMT_offset,0);
        s.Append(ts.Add(lmt).Format(f));
        if (format != INPUT_FORMAT) s.Append(_T(" LMT"));
    }
    return(s);
}

/*!
 * RouteProp type definition
 */

IMPLEMENT_DYNAMIC_CLASS( RouteProp, wxDialog )

 // RouteProp event table definition

BEGIN_EVENT_TABLE( RouteProp, wxDialog )
    //EVT_TEXT( ID_PLANSPEEDCTL, RouteProp::OnPlanSpeedCtlUpdated )
   // EVT_TEXT_ENTER( ID_STARTTIMECTL, RouteProp::OnStartTimeCtlUpdated )
   // EVT_RADIOBOX ( ID_TIMEZONESEL, RouteProp::OnTimeZoneSelected )
  //  EVT_BUTTON( ID_ROUTEPROP_CANCEL, RouteProp::OnRoutepropCancelClick )
      EVT_BUTTON( ID_ROUTEPROP_OK, RouteProp::OnRoutepropOkClick )
   // EVT_LIST_ITEM_SELECTED( ID_LISTCTRL, RouteProp::OnRoutepropListClick )
  //  EVT_LIST_ITEM_SELECTED( ID_TRACKLISTCTRL, RouteProp::OnRoutepropListClick )
  //  EVT_BUTTON( ID_ROUTEPROP_SPLIT, RouteProp::OnRoutepropSplitClick )
  //  EVT_BUTTON( ID_ROUTEPROP_EXTEND, RouteProp::OnRoutepropExtendClick )
 //   EVT_BUTTON( ID_ROUTEPROP_PRINT, RouteProp::OnRoutepropPrintClick )
END_EVENT_TABLE()

/*!
 * RouteProp constructors
 */

bool RouteProp::instanceFlag = false;
RouteProp* RouteProp::single = NULL;
RouteProp* RouteProp::getInstance( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style )
{
    if(! instanceFlag)
    {
        single = new RouteProp( parent, id, title, pos, size, style);
        instanceFlag = true;
        return single;
    }
    else
    {
        return single;
    }
}

RouteProp::RouteProp()
{
}

RouteProp::RouteProp( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos,
        const wxSize& size, long style )
{
    m_TotalDistCtl = NULL;
    m_wpList = NULL;
    m_nSelected = 0;
    m_pHead = NULL;
    m_pTail = NULL;
    m_pEnroutePoint = NULL;
    m_bStartNow = false;

    m_pRoute = 0;
    m_pEnroutePoint = NULL;
    m_bStartNow = false;
    long wstyle = style;
#ifdef __WXOSX__
    wstyle |= wxSTAY_ON_TOP;
#endif

    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );
        
    m_bcompact = false;
    
    CreateControls();
}



RouteProp::~RouteProp()
{
    delete m_TotalDistCtl;
    delete m_PlanSpeedCtl;
    delete m_TimeEnrouteCtl;

    delete m_RouteNameCtl;
    delete m_RouteStartCtl;
    delete m_RouteDestCtl;

    delete m_StartTimeCtl;

    delete m_wpList;

    // delete global print route selection dialog
   
    instanceFlag = false;
}


/*!
 * Control creation for RouteProp
 */

void RouteProp::CreateControlsCompact()
{
     

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer( wxVERTICAL );
    SetSizer( itemBoxSizer1 );

    itemDialog1 = new wxScrolledWindow( this, wxID_ANY,
                                      wxDefaultPosition, wxSize(-1, -1), wxVSCROLL);
    itemDialog1->SetScrollRate(0, 1);

    itemBoxSizer1->Add( itemDialog1, 1, wxEXPAND | wxALL, 0 );

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );


    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Name"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add( itemStaticText4, 0,
                              wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5 );

    m_RouteNameCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, _T(""), wxDefaultPosition,
		wxSize(400, -1), wxTE_READONLY);
    itemBoxSizer2->Add( m_RouteNameCtl, 0,
                        wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM , 5 );

    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Depart From"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add( itemStaticText7, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5 );

    m_RouteStartCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL2, _T(""), wxDefaultPosition,
		wxSize(-1, -1), wxTE_READONLY);
    itemBoxSizer2->Add( m_RouteStartCtl, 0,
                              wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Destination"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add( itemStaticText8, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5 );


    m_RouteDestCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, _T(""), wxDefaultPosition,
		wxSize(-1, -1), wxTE_READONLY);
    itemBoxSizer2->Add( m_RouteDestCtl, 0,
                        wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );

    
    
    wxFlexGridSizer* itemFlexGridSizer6a = new wxFlexGridSizer( 4, 2, 0, 0 );
    itemFlexGridSizer6a->AddGrowableCol(1, 0);
    
    itemBoxSizer2->Add( itemFlexGridSizer6a, 0, wxEXPAND | wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC,
            _("Total Distance"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( itemStaticText11, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
            5 );

    m_TotalDistCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL3, _T(""), wxDefaultPosition,
                                     wxSize( -1, -1 ), wxTE_READONLY );
    itemFlexGridSizer6a->Add( m_TotalDistCtl, 0,
                              wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    
    
    
    m_PlanSpeedLabel = new wxStaticText( itemDialog1, wxID_STATIC, _("Plan Speed"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( m_PlanSpeedLabel, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
            5 );

    m_PlanSpeedCtl = new wxTextCtrl( itemDialog1, ID_PLANSPEEDCTL, _T(""), wxDefaultPosition,
		wxSize(150, -1), wxTE_READONLY);
    itemFlexGridSizer6a->Add( m_PlanSpeedCtl, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    
    
    
    wxStaticText* itemStaticText12a = new wxStaticText( itemDialog1, wxID_STATIC, _("Time Enroute"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( itemStaticText12a, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
            5 );

    m_TimeEnrouteCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL4, _T(""), wxDefaultPosition,
                                       wxSize( -1, -1 ), wxTE_READONLY );
    itemFlexGridSizer6a->Add( m_TimeEnrouteCtl, 0,
                              wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    
    m_StartTimeLabel = new wxStaticText( itemDialog1, wxID_STATIC, _("Departure Time"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( m_StartTimeLabel, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
            5 );

    m_StartTimeCtl = new wxTextCtrl( itemDialog1, ID_STARTTIMECTL, _T(""), wxDefaultPosition,
		wxSize(-1, -1), wxTE_READONLY);
    itemFlexGridSizer6a->Add( m_StartTimeCtl, 0,
            wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );

    wxString pDispTimeZone[] = { _("UTC"), _("Local @ PC"), _("LMT @ Location") };
    
    wxStaticText* itemStaticText12b = new wxStaticText( itemDialog1, wxID_STATIC, _("Times shown as"),
                                                                                    wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add( itemStaticText12b, 0, wxEXPAND | wxALL, 5 );
    

    m_prb_tzUTC = new wxRadioButton(itemDialog1, ID_TIMEZONESEL_UTC, _("UTC"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    itemBoxSizer2->Add( m_prb_tzUTC, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,5 );
 
    m_prb_tzLocal = new wxRadioButton(itemDialog1, ID_TIMEZONESEL_LOCAL, _("Local @ PC"));
    itemBoxSizer2->Add( m_prb_tzLocal, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,5 );

    m_prb_tzLMT = new wxRadioButton(itemDialog1, ID_TIMEZONESEL_LMT, _("LMT @ Location"));
    itemBoxSizer2->Add( m_prb_tzLMT, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,5 );

    
    wxFlexGridSizer* itemFlexGridSizer6b = new wxFlexGridSizer( 3, 2, 0, 0 );
    itemBoxSizer2->Add( itemFlexGridSizer6b, 0, wxEXPAND | wxALIGN_LEFT | wxALL, 5 );
    
    m_staticText1 = new wxStaticText( itemDialog1, wxID_ANY, _("Route Calculation Type:"), wxDefaultPosition, wxDefaultSize,
            0 );
    itemFlexGridSizer6b->Add( m_staticText1, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

	m_TypeRouteCtl = new wxTextCtrl(itemDialog1, ID_TEXTCTRL4, _T(""), wxDefaultPosition,
		wxSize(-1, -1), wxTE_READONLY);
	itemFlexGridSizer6a->Add(m_TimeEnrouteCtl, 0,
		wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5);
    
    //itemFlexGridSizer6b->Add( m_chColor, 0,  wxALIGN_CENTER_VERTICAL | wxALL, 5 );
    
 
    wxStaticBox* itemStaticBoxSizer14Static = new wxStaticBox( itemDialog1, wxID_ANY, _("Waypoints") );
    m_pListSizer = new wxStaticBoxSizer( itemStaticBoxSizer14Static, wxVERTICAL );
    itemBoxSizer2->Add( m_pListSizer, 1, wxEXPAND | wxALL, 1 );
 
    
    wxScrolledWindow *itemlistWin = new wxScrolledWindow( itemDialog1, wxID_ANY,
                                                          wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
    itemlistWin->SetScrollRate(2, 2);
    
    
    
    m_pListSizer->Add( itemlistWin, 1, wxEXPAND | wxALL, 6 );
    
    
    //      Create the list control
    m_wpList = new wxListCtrl( itemlistWin, ID_LISTCTRL, wxDefaultPosition, wxSize( 100, -1 ),
                               wxLC_REPORT | wxLC_HRULES | wxLC_VRULES );
        
    
    wxBoxSizer* itemBoxSizerBottom = new wxBoxSizer( wxVERTICAL );
    itemBoxSizer1->Add( itemBoxSizerBottom, 0, wxALIGN_LEFT | wxALL | wxEXPAND, 5 );
    
    wxBoxSizer* itemBoxSizerAux = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizerBottom->Add( itemBoxSizerAux, 1, wxALIGN_LEFT | wxALL, 3 );
    
    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizerBottom->Add( itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 3 );


    m_OKButton = new wxButton( this, ID_ROUTEPROP_OK, _("OK"), wxDefaultPosition,
            wxDefaultSize, 0 );
    itemBoxSizer16->Add( m_OKButton, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1);
    m_OKButton->SetDefault();



    //      To correct a bug in MSW commctl32, we need to catch column width drag events, and do a Refresh()
    //      Otherwise, the column heading disappear.....
    //      Does no harm for GTK builds, so no need for conditional
  
    int char_size = GetCharWidth();

    m_wpList->InsertColumn( 0, _("Leg"), wxLIST_FORMAT_LEFT, 10 );
    m_wpList->InsertColumn( 1, _("At Waypoint"), wxLIST_FORMAT_LEFT, char_size * 14 );
    m_wpList->InsertColumn( 2, _("Distance"), wxLIST_FORMAT_RIGHT, char_size * 9 );

    
    m_wpList->InsertColumn( 3, _("Bearing"), wxLIST_FORMAT_LEFT, char_size * 10 );

    m_wpList->InsertColumn( 4, _("Latitude"), wxLIST_FORMAT_LEFT, char_size * 11 );
    m_wpList->InsertColumn( 5, _("Longitude"), wxLIST_FORMAT_LEFT, char_size * 11 );
    m_wpList->InsertColumn( 6, _("ETA"), wxLIST_FORMAT_LEFT, char_size * 15 );
    m_wpList->InsertColumn( 7, _("Speed"), wxLIST_FORMAT_CENTER, char_size * 9 );
    m_wpList->InsertColumn( 8, _("Course to steer"), wxLIST_FORMAT_LEFT, char_size * 11 );
    m_wpList->InsertColumn( 9, _("Tidal Set"), wxLIST_FORMAT_LEFT, char_size * 11 );
    m_wpList->InsertColumn( 10, _("Tidal Rate"), wxLIST_FORMAT_LEFT, char_size * 10 );

    
   
    //Set the maximum size of the entire  dialog
    int width, height;
    ::wxDisplaySize( &width, &height );
    SetSizeHints( -1, -1, width-100, height-100 );
    

    //  Fetch any config file values
   // m_planspeed = g_PlanSpeed;

       
}

void RouteProp::CreateControls()
{
    
    
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer( wxVERTICAL );
    SetSizer( itemBoxSizer1 );
    
    itemDialog1 = new wxScrolledWindow( this, wxID_ANY,
                                        wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
    itemDialog1->SetScrollRate(2, 2);
    
    itemBoxSizer1->Add( itemDialog1, 2, wxEXPAND | wxALL, 0 );
    
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );
    
    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox( itemDialog1, wxID_ANY,
                                                              _("Properties") );
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer( itemStaticBoxSizer3Static,
                                                                  wxVERTICAL );
    itemBoxSizer2->Add( itemStaticBoxSizer3, 0, wxEXPAND | wxALL, 5 );
    
    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Name"),
                                                      wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer3->Add( itemStaticText4, 0,
                              wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5 );
    
    m_RouteNameCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, _T(""), wxDefaultPosition,
                                     wxSize( 710, -1 ), 0 );
    itemStaticBoxSizer3->Add( m_RouteNameCtl, 0,
                              wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5 );
    
    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer( 2, 2, 0, 0 );
    itemStaticBoxSizer3->Add( itemFlexGridSizer6, 1, wxALIGN_LEFT | wxALL, 5 );
    
    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Depart From"),
                                                      wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add( itemStaticText7, 0,
                             wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5 );
    
    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Destination"),
                                                      wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add( itemStaticText8, 0,
                             wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5 );
    
    m_RouteStartCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL2, _T(""), wxDefaultPosition,
                                      wxSize( 300, -1 ), 0 );
    itemFlexGridSizer6->Add( m_RouteStartCtl, 0,
                             wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    m_RouteDestCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, _T(""), wxDefaultPosition,
                                     wxSize( 300, -1 ), 0 );
    itemFlexGridSizer6->Add( m_RouteDestCtl, 0,
                             wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    wxFlexGridSizer* itemFlexGridSizer6a = new wxFlexGridSizer( 2, 4, 0, 0 );
    itemStaticBoxSizer3->Add( itemFlexGridSizer6a, 1, wxALIGN_LEFT | wxALL, 5 );
    
    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC,
                                                       _("Total Distance"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( itemStaticText11, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
                              5 );
    
    m_PlanSpeedLabel = new wxStaticText( itemDialog1, wxID_STATIC, _("Plan Speed"),
                                         wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( m_PlanSpeedLabel, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
                              5 );
    
    wxStaticText* itemStaticText12a = new wxStaticText( itemDialog1, wxID_STATIC, _("Time Enroute"),
                                                        wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( itemStaticText12a, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
                              5 );
    
    m_StartTimeLabel = new wxStaticText( itemDialog1, wxID_STATIC, _("Departure Time (Y/m/d h:m)"),
                                         wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( m_StartTimeLabel, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
                              5 );
    
    m_TotalDistCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL3, _T(""), wxDefaultPosition,
                                     wxDefaultSize, wxTE_READONLY );
    itemFlexGridSizer6a->Add( m_TotalDistCtl, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    m_PlanSpeedCtl = new wxTextCtrl( itemDialog1, ID_PLANSPEEDCTL, _T(""), wxDefaultPosition,
                                     wxSize( 100, -1 ), wxTE_PROCESS_ENTER );
    itemFlexGridSizer6a->Add( m_PlanSpeedCtl, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    m_TimeEnrouteCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL4, _T(""), wxDefaultPosition,
                                       wxSize( 200, -1 ), wxTE_READONLY );
    itemFlexGridSizer6a->Add( m_TimeEnrouteCtl, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    m_StartTimeCtl = new wxTextCtrl( itemDialog1, ID_STARTTIMECTL, _T(""), wxDefaultPosition,
                                     wxSize( 150, -1 ), wxTE_PROCESS_ENTER );
    itemFlexGridSizer6a->Add( m_StartTimeCtl, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    wxString pDispTimeZone[] = { _("UTC"), _("Local @ PC"), _("LMT @ Location") };
    wxBoxSizer* bSizer2;
    bSizer2 = new wxBoxSizer( wxHORIZONTAL );
    
    wxStaticBox* itemStaticBoxTZ = new wxStaticBox( itemDialog1, wxID_ANY,  _("Times shown as") );
    wxStaticBoxSizer* itemStaticBoxSizerTZ = new wxStaticBoxSizer( itemStaticBoxTZ, wxHORIZONTAL );
    bSizer2->Add( itemStaticBoxSizerTZ, 0, wxEXPAND | wxALL, 5 );
    
    
    m_prb_tzUTC = new wxRadioButton(itemDialog1, ID_TIMEZONESEL_UTC, _("UTC"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    itemStaticBoxSizerTZ->Add( m_prb_tzUTC, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,5 );
    
    m_prb_tzLocal = new wxRadioButton(itemDialog1, ID_TIMEZONESEL_LOCAL, _("Local @ PC"));
    itemStaticBoxSizerTZ->Add( m_prb_tzLocal, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,5 );
    
    m_prb_tzLMT = new wxRadioButton(itemDialog1, ID_TIMEZONESEL_LMT, _("LMT @ Location"));
    itemStaticBoxSizerTZ->Add( m_prb_tzLMT, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,5 );
    
    m_staticText1 = new wxStaticText( itemDialog1, wxID_ANY, _("RouteCalculation Type:"), wxDefaultPosition, wxDefaultSize,
                                      0 );
    //m_staticText1->Wrap( -1 );
    bSizer2->Add( m_staticText1, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
    
	m_TypeRouteCtl = new wxTextCtrl(itemDialog1, ID_TEXTCTRL4, _T(""), wxDefaultPosition,
		wxSize(-1, -1), wxTE_READONLY);

	bSizer2->Add(m_TypeRouteCtl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  
        itemStaticBoxSizer3->Add( bSizer2, 1, wxEXPAND, 0 );
        
        wxStaticBox* itemStaticBoxSizer14Static = new wxStaticBox( this, wxID_ANY, _("Waypoints") );
        m_pListSizer = new wxStaticBoxSizer( itemStaticBoxSizer14Static, wxVERTICAL );
        itemBoxSizer1->Add( m_pListSizer, 2, wxEXPAND | wxALL, 1 );
        
        //      Create the list control
        m_wpList = new wxListCtrl( this, ID_LISTCTRL, wxDefaultPosition, wxSize( -1, -1 ),
                                   wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_EDIT_LABELS );
        
        m_wpList->SetMinSize(wxSize(-1, 100) );
        m_pListSizer->Add( m_wpList, 1, wxEXPAND | wxALL, 6 );
          
        
        
        wxBoxSizer* itemBoxSizerBottom = new wxBoxSizer( wxHORIZONTAL );
        itemBoxSizer1->Add( itemBoxSizerBottom, 0, wxALIGN_LEFT | wxALL | wxEXPAND, 5 );
        
        //wxBoxSizer* itemBoxSizerAux = new wxBoxSizer( wxHORIZONTAL );
        //itemBoxSizerBottom->Add( itemBoxSizerAux, 1, wxALIGN_LEFT | wxALL, 3 );
        
        //m_PrintButton = new wxButton( this, ID_ROUTEPROP_PRINT, _("Print Route"),
         // wxDefaultPosition, wxDefaultSize, 0 );
        //itemBoxSizerAux->Add( m_PrintButton, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 3 );
        //m_PrintButton->Enable( true );            
      
      wxBoxSizer* itemBoxSizer16 = new wxBoxSizer( wxHORIZONTAL );
      itemBoxSizerBottom->Add( itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 3 );
      
      m_OKButton = new wxButton( this, ID_ROUTEPROP_OK, _("OK"), wxDefaultPosition,
      wxDefaultSize, 0 );
      itemBoxSizer16->Add( m_OKButton, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1);
      m_OKButton->SetDefault();
      
      //      To correct a bug in MSW commctl32, we need to catch column width drag events, and do a Refresh()
      //      Otherwise, the column heading disappear.....
      //      Does no harm for GTK builds, so no need for conditional
   
      
      
      
      int char_size = GetCharWidth();
      
      m_wpList->InsertColumn( 0, _("Leg"), wxLIST_FORMAT_LEFT, char_size * 6 );
      m_wpList->InsertColumn( 1, _("At Waypoint"), wxLIST_FORMAT_LEFT, char_size * 14 );
      m_wpList->InsertColumn( 2, _("Distance"), wxLIST_FORMAT_RIGHT, char_size * 9 );
      
   
          m_wpList->InsertColumn( 3, _("Bearing"), wxLIST_FORMAT_LEFT, char_size * 10 );
      
      m_wpList->InsertColumn( 4, _("Latitude"), wxLIST_FORMAT_LEFT, char_size * 11 );
      m_wpList->InsertColumn( 5, _("Longitude"), wxLIST_FORMAT_LEFT, char_size * 11 );
      m_wpList->InsertColumn( 6, _("ETA"), wxLIST_FORMAT_LEFT, char_size * 15 );
      m_wpList->InsertColumn( 7, _("Speed"), wxLIST_FORMAT_CENTER, char_size * 9 );
      m_wpList->InsertColumn( 8, _("CTS"), wxLIST_FORMAT_LEFT, char_size * 11 );
      m_wpList->InsertColumn( 9, _("Tidal Set"), wxLIST_FORMAT_LEFT, char_size * 11 );
	  m_wpList->InsertColumn( 10, _("Tidal Rate"), wxLIST_FORMAT_LEFT, char_size * 10 );
      
      
      //Set the maximum size of the entire  dialog
      int width, height;
      ::wxDisplaySize( &width, &height );
      SetSizeHints( -1, -1, width-100, height-100 );
      
 
    
      
      
      }
      

/*
 * Should we show tooltips?
 */

bool RouteProp::ShowToolTips()
{
    return TRUE;
}

void RouteProp::SetDialogTitle(const wxString & title)
{
    SetTitle(title);
}

void RouteProp::OnRoutepropOkClick(wxCommandEvent& event)
{	
	Hide();
	event.Skip();
}



//-------------------------------------------------------------------------------
//
//    Mark Properties Dialog Implementation
//
//-------------------------------------------------------------------------------
/*!
 * MarkProp type definition
 */

