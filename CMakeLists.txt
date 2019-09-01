##---------------------------------------------------------------------------
## Author:      Dave Register
##***************************************************************************
## *   Copyright (C) 2010 by David S. Register                               *
## *   This program is free software; you can redistribute it and/or modify  *
## *   it under the terms of the GNU General Public License as published by  *
## *   the Free Software Foundation; either version 2 of the License, or     *
## *   (at your option) any later version.                                   *
## *                                                                         *
## *   This program is distributed in the hope that it will be useful,       *
## *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
## *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
## *   GNU General Public License for more details.                          *
## *                                                                         *
## *   You should have received a copy of the GNU General Public License     *
## *   along with this program; if not, write to the                         *
## *   Free Software Foundation, Inc.,                                       *
## *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
## ***************************************************************************

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

PROJECT(otidalplan_pi)

SET(PACKAGE_NAME otidalplan_pi)
SET(VERBOSE_NAME otidalplan)
SET(TITLE_NAME otidalplan)
SET(CPACK_PACKAGE_CONTACT "Mike Rossiter")

SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "1")

#SET(CMAKE_BUILD_TYPE Debug)
OPTION(OTIDALPLAN_USE_SVG "Use SVG graphics" ON)

INCLUDE("cmake/PluginConfigure.cmake")

IF(OTIDALPLAN_USE_SVG)
  ADD_DEFINITIONS(-DOTIDALPLAN_USE_SVG)
ENDIF(OTIDALPLAN_USE_SVG)


SET(SRC_otidalplan
		src/AboutDialog.cpp
		src/AboutDialog.h
        src/bbox.cpp
        src/bbox.h	
        src/otidalplan_pi.h
        src/otidalplan_pi.cpp
        src/otidalplanOverlayFactory.cpp
        src/otidalplanOverlayFactory.h
        src/otidalplanUIDialogBase.cpp
        src/otidalplanUIDialogBase.h
        src/otidalplanUIDialog.cpp
        src/otidalplanUIDialog.h
        src/icons.h
        src/icons.cpp
        src/tcmgr.cpp       
        src/tcmgr.h
        src/tinystr.h
	    src/tinyxml.h
		src/tinyxml.cpp
	    src/tinyxmlerror.cpp
		src/tinyxmlparser.cpp		
		src/NavFunc.cpp
		src/NavFunc.h
		src/routeprop.cpp
		src/routeprop.h
		src/tableroutes.cpp
		src/tableroutes.h

    )

ADD_DEFINITIONS(-DTIXML_USE_STL)

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_otidalplan})

INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")
