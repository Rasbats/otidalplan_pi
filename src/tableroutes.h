/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  RouteProerties Support
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

#ifndef _TABLEROUTES_H_
#define _TABLEROUTES_H_

/*!
 * Includes
 */
#include "wx/listctrl.h"              // for ColorScheme
#include "wx/hyperlink.h"           // toh, 2009.02.08
#include <wx/choice.h>
#include <wx/tglbtn.h>
#include <wx/bmpcbox.h>
#include <wx/notebook.h>
#include <wx/filesys.h>
#include <wx/clrpicker.h>

#if wxCHECK_VERSION(2, 9, 0)
#include <wx/dialog.h>
#else
#include "scrollingdialog.h"
#endif

/*!
 * Forward declarations
 */

class   wxListCtrl;
class   Route;
class   RoutePoint;
class   HyperlinkList;
class   otidalplan_pi;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_ROUTEPROP 7000
#define SYMBOL_ROUTEPROP_TITLE _("Route Properties")
#define SYMBOL_ROUTEPROP_IDNAME ID_ROUTEPROP
#define SYMBOL_ROUTEPROP_SIZE wxSize(450, 300)
#define SYMBOL_ROUTEPROP_POSITION wxDefaultPosition

#ifdef __WXOSX__
#define SYMBOL_ROUTEPROP_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxSTAY_ON_TOP
#else
#define SYMBOL_ROUTEPROP_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#endif


#define ID_TEXTCTRL            7001
#define ID_TEXTCTRL2           7002
#define ID_TEXTCTRL1           7003
#define ID_TEXTCTRL3           7005
#define ID_LISTCTRL            7004
#define ID_ROUTEPROP_CANCEL    7006
#define ID_ROUTEPROP_OK        7007
#define ID_ROUTEPROP_SPLIT     7107
#define ID_ROUTEPROP_EXTEND    7207
#define ID_ROUTEPROP_COPYTXT   7307
#define ID_ROUTEPROP_PRINT     7407
#define ID_WAYPOINTRANGERINGS  7507 
#define ID_SHOWWAYPOINTRANGERINGS  7607 
#define ID_PLANSPEEDCTL        7008
#define ID_TEXTCTRL4           7009
#define ID_TEXTCTRLDESC        7010
#define ID_STARTTIMECTL        7011
#define ID_TIMEZONESEL         7012
#define ID_TRACKLISTCTRL       7013
#define ID_RCLK_MENU_COPY_TEXT 7014
#define ID_RCLK_MENU_EDIT_WP   7015
#define ID_RCLK_MENU_DELETE    7016
#define ID_RCLK_MENU_COPY      7017
#define ID_RCLK_MENU_COPY_LL   7018
#define ID_RCLK_MENU_PASTE     7019
#define ID_RCLK_MENU_PASTE_LL  7020
#define ID_TIMEZONESEL_UTC     7021
#define ID_TIMEZONESEL_LOCAL   7022
#define ID_TIMEZONESEL_LMT     7023

#define ID_MARKPROP 8000
#define SYMBOL_MARKPROP_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_MARKPROP_TITLE _("Waypoint Properties")
#define SYMBOL_MARKPROP_IDNAME ID_MARKPROP
#define SYMBOL_MARKPROP_SIZE wxSize(200, 300)
#define SYMBOL_MARKPROP_POSITION wxDefaultPosition
#define ID_MARKPROP_CANCEL 8001
#define ID_MARKPROP_OK 8002
#define ID_ICONCTRL 8003
#define ID_LATCTRL 8004
#define ID_LONCTRL 8005
#define ID_SHOWNAMECHECKBOX1 8006

////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * RouteProp class declaration
 */


class TableRoutes: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( RouteProp )
	DECLARE_EVENT_TABLE()

public:
    /// Constructors
	static TableRoutes* getInstance(wxWindow* parent, wxWindowID id = SYMBOL_ROUTEPROP_IDNAME,
                                   const wxString& caption = SYMBOL_ROUTEPROP_TITLE,
                                   const wxPoint& pos = SYMBOL_ROUTEPROP_POSITION,
                                   const wxSize& size = SYMBOL_ROUTEPROP_SIZE,
                                   long style = SYMBOL_ROUTEPROP_STYLE );
	~TableRoutes();
    static bool getInstanceFlag(){ return instanceFlag; } 
    
    void CreateControls();
    void CreateControlsCompact();
	
    void SetDialogTitle(const wxString & title);
	void OnRoutepropOkClick( wxCommandEvent& event );
	/*
    void OnRoutepropCancelClick( wxCommandEvent& event );
    
    void OnPlanSpeedCtlUpdated( wxCommandEvent& event );
    void OnStartTimeCtlUpdated( wxCommandEvent& event );
    void OnTimeZoneSelected( wxCommandEvent& event );
    void OnRoutepropListClick( wxListEvent& event );
    void OnRoutepropSplitClick( wxCommandEvent& event );
    void OnRoutepropExtendClick( wxCommandEvent& event );
    void OnRoutepropPrintClick( wxCommandEvent& event );
    void OnRoutepropCopyTxtClick( wxCommandEvent& event );
    void OnRoutePropMenuSelected( wxCommandEvent &event );
    bool IsThisRouteExtendable();
    void OnEvtColDragEnd(wxListEvent& event);
    void InitializeList();
    */
    /// Should we show tooltips?
    static bool ShowToolTips();

    wxTextCtrl  *m_TotalDistCtl;
    wxTextCtrl  *m_PlanSpeedCtl;
    wxTextCtrl	*m_StartTimeCtl;
    wxTextCtrl  *m_TimeEnrouteCtl;

    wxStaticText *m_PlanSpeedLabel;
    wxStaticText *m_StartTimeLabel;

    wxTextCtrl  *m_RouteNameCtl;
    wxTextCtrl  *m_RouteStartCtl;
    wxTextCtrl  *m_RouteDestCtl;
	wxTextCtrl  *m_TypeRouteCtl;

    wxListCtrl        *m_wpList;

    wxButton*     m_OKButton;
    wxButton*     m_CopyTxtButton;
    wxButton*     m_PrintButton;


    Route       *m_pRoute;
    Route       *m_pHead; // for route splitting
    Route       *m_pTail;
    RoutePoint *m_pExtendPoint;
    Route *m_pExtendRoute;
    RoutePoint    *m_pEnroutePoint;
    bool          m_bStartNow;

    double      m_planspeed;
    double      m_avgspeed;

    int         m_nSelected; // index of point selected in Properties dialog row
    int         m_tz_selection;

    wxDateTime	 m_starttime; // kept as UTC
//    wxRadioBox	*pDispTz;
    wxStaticText  *m_staticText1;
    wxStaticText  *m_staticText2;
    wxStaticText  *m_staticText3;
    wxChoice      *m_chColor;
    wxChoice      *m_chStyle;
    wxChoice      *m_chWidth;

    wxStaticBoxSizer* m_pListSizer;
    wxScrolledWindow *itemDialog1;

	otidalplan_pi *pPlugIn;
    
//private:
	TableRoutes();
	TableRoutes(wxWindow* parent, wxWindowID id = SYMBOL_ROUTEPROP_IDNAME,
              const wxString& caption = SYMBOL_ROUTEPROP_TITLE,
              const wxPoint& pos = SYMBOL_ROUTEPROP_POSITION,
              const wxSize& size = SYMBOL_ROUTEPROP_SIZE,
              long style = SYMBOL_ROUTEPROP_STYLE );
    
    static bool instanceFlag;
	static TableRoutes *single;
    
    int GetTZSelection(void);
    wxRadioButton  *m_prb_tzUTC;
    wxRadioButton  *m_prb_tzLocal;
    wxRadioButton  *m_prb_tzLMT;
    bool m_bcompact;
    
};


#endif // _TABLEROUTES_H_
