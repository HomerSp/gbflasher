# - Try to find the freetype library
# Once done this defines
#
#  LIBHIDAPIRAW_FOUND - system has libusb
#  LIBHIDAPIRAW_INCLUDE_DIR - the libusb include directory
#  LIBHIDAPIRAW_LIBRARIES - Link these to use libusb

# Copyright (c) 2006, 2008  Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (LIBHIDAPIRAW_INCLUDE_DIR AND LIBHIDAPIRAW_LIBRARIES)

  # in cache already
  set(LIBHIDAPIRAW_FOUND TRUE)

else (LIBHIDAPIRAW_INCLUDE_DIR AND LIBHIDAPIRAW_LIBRARIES)
  IF (NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig)
    pkg_check_modules(PC_LIBHIDAPIRAW hidapi-hidraw)
  ENDIF(NOT WIN32)

  set(LIBHIDAPIRAW_LIBRARY_NAME hidapi-hidraw)

  FIND_PATH(LIBHIDAPIRAW_INCLUDE_DIR hidapi.h
    PATHS ${PC_LIBHIDAPIRAW_INCLUDEDIR} ${PC_LIBHIDAPIRAW_INCLUDE_DIRS})

  FIND_LIBRARY(LIBHIDAPIRAW_LIBRARIES NAMES ${LIBHIDAPIRAW_LIBRARY_NAME}
    PATHS ${PC_LIBHIDAPIRAW_LIBDIR} ${PC_LIBHIDAPIRAW_LIBRARY_DIRS})

  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBHIDAPIRAW DEFAULT_MSG LIBHIDAPIRAW_LIBRARIES LIBHIDAPIRAW_INCLUDE_DIR)

  MARK_AS_ADVANCED(LIBHIDAPIRAW_INCLUDE_DIR LIBHIDAPIRAW_LIBRARIES)

endif (LIBHIDAPIRAW_INCLUDE_DIR AND LIBHIDAPIRAW_LIBRARIES)