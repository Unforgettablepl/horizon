# Locate LuaJIT library
# This module defines
#  LUAJIT_FOUND, if false, do not try to link to Lua
#  LUA_LIBRARIES
#  LUA_INCLUDE_DIR, where to find lua.h
#  LUAJIT_VERSION_STRING, the version of Lua found (since CMake 2.8.8)

## Copied from default CMake FindLua51.cmake

find_path(LUA_INCLUDE_DIR 
  NAMES lua.h
  PATH_SUFFIXES lua lua5.3
  HINTS
  ~/Library/Frameworks
  /Library/Frameworks
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  /usr
  /usr/local/opt/include
  /usr/local/include
  /usr/include
  ${_VCPKG_INSTALLED_DIR}/x${PLATFORM}-windows/include
)

find_library(LUA_LIBRARIES
  NAMES lua lua5.3
  PATH_SUFFIXES lib
  HINTS
  ~/Library/Frameworks
  /Library/Frameworks
  /sw
  /opt/local
  /opt/csw
  /opt
  /usr
  /usr/local/
  /usr/lib/
  ${_VCPKG_INSTALLED_DIR}/x${PLATFORM}-windows
)

if (NOT LUA_INCLUDE_DIR)
  message(FATAL_ERROR "Lua headers not found")
else()
  message(STATUS "Lua headers found at ${LUA_INCLUDE_DIR}")
endif()

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LUA_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(lua
                                  REQUIRED_VARS LUA_LIBRARIES LUA_INCLUDE_DIR
                                  VERSION_VAR LUA_VERSION_STRING)

mark_as_advanced(LUA_INCLUDE_DIR LUA_LIBRARIES)
