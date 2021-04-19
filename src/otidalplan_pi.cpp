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


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
  #include <wx/glcanvas.h>
#endif //precompiled headers

#include <wx/fileconf.h>
#include <wx/stdpaths.h>

#include "ocpn_plugin.h"

#include "otidalplan_pi.h"
#include "otidalplanUIDialogBase.h"
#include "otidalplanUIDialog.h"

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return new otidalplan_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}



//---------------------------------------------------------------------------------------------------------
//
//    otidalplan PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

#include "icons.h"


//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

otidalplan_pi::otidalplan_pi(void *ppimgr)
      :opencpn_plugin_116(ppimgr)
{
      // Create the PlugIn icons
     initialize_images();

	wxFileName fn;

	auto path = GetPluginDataDir("otidalplan_pi");
	fn.SetPath(path);
	fn.AppendDir("data");
	fn.SetFullName("otidalplan_panel_icon.png");

	path = fn.GetFullPath();

	wxInitAllImageHandlers();

	wxLogDebug(wxString("Using icon path: ") + path);
	if (!wxImage::CanRead(path)) {
		wxLogDebug("Initiating image handlers.");
		wxInitAllImageHandlers();
	}
	wxImage panelIcon(path);
	if (panelIcon.IsOk())
		m_panelBitmap = wxBitmap(panelIcon);
	else
		wxLogWarning("otidalplan panel icon has NOT been loaded");	
		
		
    m_bShowotidalplan = false;   

}

otidalplan_pi::~otidalplan_pi(void)
{
	  delete _img_otidalplan_pi;
      delete _img_otidalplan;
}

int otidalplan_pi::Init(void)
{
      AddLocaleCatalog( _T("opencpn-otidalplan_pi") );

      // Set some default private member parameters
      m_otidalplan_dialog_x = 0;
      m_otidalplan_dialog_y = 0;
      m_otidalplan_dialog_sx = 200;
      m_otidalplan_dialog_sy = 400;
      m_potidalplanDialog = NULL;
      m_potidalplanOverlayFactory = NULL;
      m_botidalplanShowIcon = true;

      ::wxDisplaySize(&m_display_width, &m_display_height);
  
	  m_pconfig = GetOCPNConfigObject();

      //    And load the configuration items
      LoadConfig();

      // Get a pointer to the opencpn display canvas, to use as a parent for the otidalplan dialog
      m_parent_window = GetOCPNCanvasWindow();
	
	  wxMenu dummy_menu;
	  m_position_menu_id = AddCanvasContextMenuItem
	  (new wxMenuItem(&dummy_menu, -1, _("Delete Tidal Current Station")), this);
	  SetCanvasContextMenuItemViz(m_position_menu_id, true);

      //    This PlugIn needs a toolbar icon, so request its insertion if enabled locally
      if(m_botidalplanShowIcon)
#ifdef OTIDALPLAN_USE_SVG
				  m_leftclick_tool_id = InsertPlugInToolSVG(_T("otidalplan"), _svg_otidalplan, _svg_otidalplan, _svg_otidalplan_toggled,
					  wxITEM_CHECK, _("otidalplan"), _T(""), NULL, otidalplan_TOOL_POSITION, 0, this);
#else
				  m_leftclick_tool_id = InsertPlugInTool(_T(""), _img_otidalplan, _img_otidalplan, wxITEM_CHECK,
					  _("otidalplan"), _T(""), NULL,
					  otidalplan_TOOL_POSITION, 0, this);
#endif			                                           
	 

      return (WANTS_OVERLAY_CALLBACK |
              WANTS_OPENGL_OVERLAY_CALLBACK |
              WANTS_TOOLBAR_CALLBACK    |
			  WANTS_CURSOR_LATLON |
              INSTALLS_TOOLBAR_TOOL     |
              WANTS_CONFIG              |              
              WANTS_PLUGIN_MESSAGING
            );
}

bool otidalplan_pi::DeInit(void)
{
    if(m_potidalplanDialog) {
        m_potidalplanDialog->Close();
        delete m_potidalplanDialog;
        m_potidalplanDialog = NULL;
    }

    delete m_potidalplanOverlayFactory;
    m_potidalplanOverlayFactory = NULL;

    return true;
}

int otidalplan_pi::GetAPIVersionMajor()
{
      return atoi(API_VERSION);
}

int otidalplan_pi::GetAPIVersionMinor()
{
      std::string v(API_VERSION);
    size_t dotpos = v.find('.');
    return atoi(v.substr(dotpos + 1).c_str());
}

int otidalplan_pi::GetPlugInVersionMajor()
{
      return PLUGIN_VERSION_MAJOR;
}

int otidalplan_pi::GetPlugInVersionMinor()
{
      return PLUGIN_VERSION_MINOR;
}

wxBitmap *otidalplan_pi::GetPlugInBitmap()
{
	return &m_panelBitmap;
}

wxString otidalplan_pi::GetCommonName()
{
      return _T("otidalplan");
}


wxString otidalplan_pi::GetShortDescription()
{
      return _("otidalplan PlugIn for OpenCPN");
}


wxString otidalplan_pi::GetLongDescription()
{
      return _("otidalplan PlugIn for OpenCPN\nCalculates EP positions/Courses using tidal current harmonics.\n\n\
			   ");
}

void otidalplan_pi::SetDefaults(void)
{
}


int otidalplan_pi::GetToolbarToolCount(void)
{
      return 1;
}



void otidalplan_pi::OnToolbarToolCallback(int id)
{
    
	if(!m_potidalplanDialog)
    {
		       		
		m_potidalplanDialog = new otidalplanUIDialog(m_parent_window, this);
        wxPoint p = wxPoint(m_otidalplan_dialog_x, m_otidalplan_dialog_y);
        m_potidalplanDialog->Move(0,0);        // workaround for gtk autocentre dialog behavior
        m_potidalplanDialog->Move(p);

        // Create the drawing factory
        m_potidalplanOverlayFactory = new otidalplanOverlayFactory( *m_potidalplanDialog );
        m_potidalplanOverlayFactory->SetParentSize( m_display_width, m_display_height);		
        
    }	

      // Qualify the otidalplan dialog position
            bool b_reset_pos = false;

#ifdef __WXMSW__
        //  Support MultiMonitor setups which an allow negative window positions.
        //  If the requested window does not intersect any installed monitor,
        //  then default to simple primary monitor positioning.
            RECT frame_title_rect;
            frame_title_rect.left =   m_otidalplan_dialog_x;
            frame_title_rect.top =    m_otidalplan_dialog_y;
            frame_title_rect.right =  m_otidalplan_dialog_x + m_otidalplan_dialog_sx;
            frame_title_rect.bottom = m_otidalplan_dialog_y + 30;


            if(NULL == MonitorFromRect(&frame_title_rect, MONITOR_DEFAULTTONULL))
                  b_reset_pos = true;
#else
       //    Make sure drag bar (title bar) of window on Client Area of screen, with a little slop...
            wxRect window_title_rect;                    // conservative estimate
            window_title_rect.x = m_otidalplan_dialog_x;
            window_title_rect.y = m_otidalplan_dialog_y;
            window_title_rect.width = m_otidalplan_dialog_sx;
            window_title_rect.height = 30;

            wxRect ClientRect = wxGetClientDisplayRect();
            ClientRect.Deflate(60, 60);      // Prevent the new window from being too close to the edge
            if(!ClientRect.Intersects(window_title_rect))
                  b_reset_pos = true;

#endif
			
            if(b_reset_pos)
            {
                  m_otidalplan_dialog_x = 20;
                  m_otidalplan_dialog_y = 170;
                  m_otidalplan_dialog_sx = 300;
                  m_otidalplan_dialog_sy = 540;
            }
			

      //Toggle otidalplan overlay display
      m_bShowotidalplan = !m_bShowotidalplan;

      //    Toggle dialog?
      if(m_bShowotidalplan) {
          m_potidalplanDialog->Show();
      } else {
          m_potidalplanDialog->Hide();         
          }

      // Toggle is handled by the toolbar but we must keep plugin manager b_toggle updated
      // to actual status to ensure correct status upon toolbar rebuild
      SetToolbarItemState( m_leftclick_tool_id, m_bShowotidalplan );
	  //SetCanvasContextMenuItemViz(m_position_menu_id, true);

      RequestRefresh(m_parent_window); // refresh main window
}

void otidalplan_pi::OnotidalplanDialogClose()
{
    m_bShowotidalplan = false;
    SetToolbarItemState( m_leftclick_tool_id, m_bShowotidalplan );
	SetCanvasContextMenuItemViz(m_position_menu_id, m_bShowotidalplan);

    m_potidalplanDialog->Hide();

    SaveConfig();

    RequestRefresh(m_parent_window); // refresh main window

}


wxString otidalplan_pi::StandardPath()
{
	wxStandardPathsBase& std_path = wxStandardPathsBase::Get();
	wxString s = wxFileName::GetPathSeparator();
#ifdef __WXMSW__
	wxString stdPath = std_path.GetConfigDir();
#endif
#ifdef __WXGTK__
	wxString stdPath = std_path.GetUserDataDir();
#endif
#ifdef __WXOSX__
	wxString stdPath = (std_path.GetUserConfigDir() + s + _T("opencpn"));   // should be ~/Library/Preferences/opencpn
#endif

	return stdPath + wxFileName::GetPathSeparator() +
		_T("plugins") + wxFileName::GetPathSeparator() +
		_T("otidalplan") + wxFileName::GetPathSeparator();

	stdPath += s + _T("plugins");
	if (!wxDirExists(stdPath))
		wxMkdir(stdPath);

	stdPath += s + _T("otidalplan");

#ifdef __WXOSX__
	// Compatibility with pre-OCPN-4.2; move config dir to
	// ~/Library/Preferences/opencpn if it exists
	wxString oldPath = (std_path.GetUserConfigDir() + s + _T("plugins") + s + _T("weather_routing"));
	if (wxDirExists(oldPath) && !wxDirExists(stdPath)) {
		wxLogMessage("weather_routing_pi: moving config dir %s to %s", oldPath, stdPath);
		wxRenameFile(oldPath, stdPath);
	}
#endif

	if (!wxDirExists(stdPath))
		wxMkdir(stdPath);

	stdPath += s;
	return stdPath;
}

bool otidalplan_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
	
	if(!m_potidalplanDialog ||
       !m_potidalplanDialog->IsShown() ||
       !m_potidalplanOverlayFactory)
        return false;

    m_potidalplanDialog->SetViewPort( vp );
    m_potidalplanOverlayFactory->RenderotidalplanOverlay ( dc, vp );
    return true;
}

bool otidalplan_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
	

	if(!m_potidalplanDialog ||
       !m_potidalplanDialog->IsShown() ||
       !m_potidalplanOverlayFactory)
        return false;

    m_potidalplanDialog->SetViewPort( vp );
    m_potidalplanOverlayFactory->RenderGLotidalplanOverlay ( pcontext, vp );
    return true;
}

void otidalplan_pi::SetCursorLatLon(double lat, double lon)
{
    if(m_potidalplanDialog)
        m_potidalplanDialog->SetCursorLatLon(lat, lon);

	m_cursor_lat = lat;
	m_cursor_lon = lon;

}

bool otidalplan_pi::LoadConfig(void)
{
    wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

    if(!pConf)
        return false;

    pConf->SetPath ( _T( "/PlugIns/otidalplan" ) );
	

	m_CopyFolderSelected = pConf->Read ( _T( "otidalplanFolder" ));
	
	if (m_CopyFolderSelected == wxEmptyString){
		 wxString g_SData_Locn = *GetpSharedDataLocation();
		// Establish location of Tide and Current data
		 pTC_Dir = new wxString(_T("tcdata"));
		 pTC_Dir->Prepend(g_SData_Locn);

		 m_CopyFolderSelected = *pTC_Dir;	  
	}

    m_otidalplan_dialog_sx = pConf->Read ( _T( "otidalplanDialogSizeX" ), 300L );
    m_otidalplan_dialog_sy = pConf->Read ( _T( "otidalplanDialogSizeY" ), 540L );
    m_otidalplan_dialog_x =  pConf->Read ( _T( "otidalplanDialogPosX" ), 20L );
    m_otidalplan_dialog_y =  pConf->Read ( _T( "otidalplanDialogPosY" ), 170L );	
	
	
    return true;
}

bool otidalplan_pi::SaveConfig(void)
{
    wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

    if(!pConf)
        return false;

    pConf->SetPath ( _T( "/PlugIns/otidalplan" ) );

	pConf->Write ( _T( "otidalplanFolder" ), m_CopyFolderSelected); 

    pConf->Write ( _T( "otidalplanDialogSizeX" ),  m_otidalplan_dialog_sx );
    pConf->Write ( _T( "otidalplanDialogSizeY" ),  m_otidalplan_dialog_sy );
    pConf->Write ( _T( "otidalplanDialogPosX" ),   m_otidalplan_dialog_x );
    pConf->Write ( _T( "otidalplanDialogPosY" ),   m_otidalplan_dialog_y );

    return true;
}

void otidalplan_pi::SetColorScheme(PI_ColorScheme cs)
{
    DimeWindow(m_potidalplanDialog);
}
/*
void otidalplan_pi::OnContextMenuItemCallback(int id)
{

	if (!m_potidalplanDialog)
		return;

	if (id == m_position_menu_id) {

		m_cursor_lat = GetCursorLat();
		m_cursor_lon = GetCursorLon();

		m_potidalplanDialog->OnContextMenu(m_cursor_lat, m_cursor_lon);
	}
}
*/
void otidalplan_pi::OnContextMenuItemCallback(int id)
{
	if (!m_potidalplanDialog)
		return;

	if (id == m_position_menu_id) {
		m_cursor_lat = GetCursorLat();
		m_cursor_lon = GetCursorLon();
		m_potidalplanDialog->getTidalCurrentStation(m_cursor_lat, m_cursor_lon);
	}
}

