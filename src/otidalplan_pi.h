/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  otidalplan Plugin
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 */

#ifndef _otidalplanPI_H_
#define _otidalplanPI_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#include <wx/glcanvas.h>
#endif //precompiled headers

#include "ocpn_plugin.h"
#include "otidalplanOverlayFactory.h"
#include "otidalplanUIDialog.h"
#include <wx/datetime.h>

#define ABOUT_AUTHOR_URL "http://mikerossiter.co.uk"


//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

#define otidalplan_TOOL_POSITION    -1          // Request default positioning of toolbar tool


class otidalplan_pi : public opencpn_plugin_116
{
public:
      otidalplan_pi(void *ppimgr);
      ~otidalplan_pi(void);

//    The required PlugIn Methods
      int Init(void);
      bool DeInit(void);

      int GetAPIVersionMajor();
      int GetAPIVersionMinor();
      int GetPlugInVersionMajor();
      int GetPlugInVersionMinor();
      wxBitmap *GetPlugInBitmap();
      wxString GetCommonName();
      wxString GetShortDescription();
      wxString GetLongDescription();	  

//    The override PlugIn Methods
      bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);
	  bool RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp);
      void SetCursorLatLon(double lat, double lon);
      void SetDefaults(void);
      int  GetToolbarToolCount(void);
      void OnToolbarToolCallback(int id);

	  double GetCursorLon(void) { return m_cursor_lon; }
	  double GetCursorLat(void) { return m_cursor_lat; }

// Other public methods
      void SetotidalplanDialogX    (int x){ m_otidalplan_dialog_x = x;};
      void SetotidalplanDialogY    (int x){ m_otidalplan_dialog_y = x;}
      void SetotidalplanDialogSizeX(int x){ m_otidalplan_dialog_sx = x;}
      void SetotidalplanDialogSizeY(int x){ m_otidalplan_dialog_sy = x;}
      void SetColorScheme(PI_ColorScheme cs);

	  void OnContextMenuItemCallback(int id);

      void OnotidalplanDialogClose();

	  wxString GetFolderSelected() {return m_CopyFolderSelected;}
	  int      GetIntervalSelected() {return m_CopyIntervalSelected;}
  
      otidalplanOverlayFactory *GetotidalplanOverlayFactory(){ return m_potidalplanOverlayFactory; }

	  double m_boat_lat, m_boat_lon;

	  double m_tr_spd;
	  double m_tr_dir;
	  otidalplanOverlayFactory *m_potidalplanOverlayFactory;

	  
	  wxString StandardPath();
	  otidalplanUIDialog     *m_potidalplanDialog;

private:
	  double m_cursor_lat, m_cursor_lon;
      bool LoadConfig(void);
      bool SaveConfig(void);

      wxFileConfig     *m_pconfig;
      wxWindow         *m_parent_window;  

      int              m_display_width, m_display_height;
      int              m_leftclick_tool_id;

      int              m_otidalplan_dialog_x, m_otidalplan_dialog_y;
      int              m_otidalplan_dialog_sx, m_otidalplan_dialog_sy;

	  wxString          m_CopyFolderSelected;
	  int               m_CopyIntervalSelected;
	  

      int              m_bTimeZone;
     
      int              m_bStartOptions;
      wxString         m_RequestConfig;
      wxString         *pTC_Dir;
      
      bool             m_botidalplanShowIcon;

      int              m_height;

      bool			   m_bShowotidalplan;

	  int              m_position_menu_id;
	  int              m_table_menu_id;

	  wxBitmap		   m_panelBitmap;

};

#endif
