# ~~~
# Summary:      Local, non-generic plugin setup
# Copyright (c) 2020-2021 Mike Rossiter
# License:      GPLv3+
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.


# -------- Options ----------

set(OCPN_TEST_REPO
    "opencpn/otidalplan-alpha"
    CACHE STRING "Default repository for untagged builds"
)
set(OCPN_BETA_REPO
    "opencpn/otidalplan-beta"
    CACHE STRING
    "Default repository for tagged builds matching 'beta'"
)
set(OCPN_RELEASE_REPO
    "opencpn/otidalplan-prod"
    CACHE STRING
    "Default repository for tagged builds not matching 'beta'"
)

#
#
# -------  Plugin setup --------
#
set(PKG_NAME otidalplan_pi)
set(PKG_VERSION  2.2.0)
set(PKG_PRERELEASE "")  # Empty, or a tag like 'beta'

set(DISPLAY_NAME otidalplan)    # Dialogs, installer artifacts, ...
set(PLUGIN_API_NAME otidalplan) # As of GetCommonName() in plugin API
set(PKG_SUMMARY "EP positions on a route")
set(PKG_DESCRIPTION [=[
Calculates Estimated Positions using tidal harmonics.
]=])

set(PKG_AUTHOR "Mike Rossiter")
set(PKG_IS_OPEN_SOURCE "yes")
set(PKG_HOMEPAGE https://github.com/Rasbats/otidalplan_pi)
set(PKG_INFO_URL https://opencpn.org/OpenCPN/plugins/otidalplan.html)

SET(SRC
        src/AboutDialog.cpp
        src/AboutDialog.h
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
        src/NavFunc.cpp
        src/NavFunc.h
        src/routeprop.cpp
        src/routeprop.h
        src/tableroutes.cpp
        src/tableroutes.h
        src/IDX_entry.cpp
        src/IDX_entry.h
        src/logger.cpp
        src/logger.h
        src/Station_Data.cpp
        src/Station_Data.h
        src/TC_Error_Code.h
        src/TCDataFactory.cpp
        src/TCDataFactory.h
        src/TCDataSource.cpp
        src/TCDataSource.h
        src/TCDS_Ascii_Harmonic.cpp
        src/TCDS_Ascii_Harmonic.h
        src/TCDS_Binary_Harmonic.cpp
        src/TCDS_Binary_Harmonic.h
        src/tzdata.h
    )

set(PKG_API_LIB api-20)  #  A directory in libs/ e. g., api-17 or api-16

macro(late_init)
  # Perform initialization after the PACKAGE_NAME library, compilers
  # and ocpn::api is available.
endmacro ()

macro(add_plugin_libraries)
  # Add libraries required by this plugin
  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/tinyxml")
  target_link_libraries(${PACKAGE_NAME} ocpn::tinyxml)

  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/wxJSON")
  target_link_libraries(${PACKAGE_NAME} ocpn::wxjson)

  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/plugin_dc")
  target_link_libraries(${PACKAGE_NAME} ocpn::plugin-dc)

  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/jsoncpp")
  target_link_libraries(${PACKAGE_NAME} ocpn::jsoncpp)

  # The wxsvg library enables SVG overall in the plugin
  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/wxsvg")
  target_link_libraries(${PACKAGE_NAME} ocpn::wxsvg)
endmacro ()
