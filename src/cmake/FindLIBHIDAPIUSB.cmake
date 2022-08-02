# - Try to find the freetype library
# Once done this defines
#
#  LIBHIDAPIUSB_FOUND - system has libusb
#  LIBHIDAPIUSB_INCLUDE_DIR - the libusb include directory
#  LIBHIDAPIUSB_LIBRARIES - Link these to use libusb

# Copyright (c) 2006, 2008  Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (LIBHIDAPIUSB_INCLUDE_DIR AND LIBHIDAPIUSB_LIBRARIES)

  # in cache already
  set(LIBHIDAPIUSB_FOUND TRUE)

else (LIBHIDAPIUSB_INCLUDE_DIR AND LIBHIDAPIUSB_LIBRARIES)
  IF (NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig)
    pkg_check_modules(PC_LIBHIDAPIUSB hidapi-libusb)
  ENDIF(NOT WIN32)

  set(LIBHIDAPIUSB_LIBRARY_NAME hidapi-libusb)

  FIND_PATH(LIBHIDAPIUSB_INCLUDE_DIR hidapi.h
    PATHS ${PC_LIBHIDAPIUSB_INCLUDEDIR} ${PC_LIBHIDAPIUSB_INCLUDE_DIRS})

  FIND_LIBRARY(LIBHIDAPIUSB_LIBRARIES NAMES ${LIBHIDAPIUSB_LIBRARY_NAME}
    PATHS ${PC_LIBHIDAPIUSB_LIBDIR} ${PC_LIBHIDAPIUSB_LIBRARY_DIRS})

  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBHIDAPIUSB DEFAULT_MSG LIBHIDAPIUSB_LIBRARIES LIBHIDAPIUSB_INCLUDE_DIR)

  MARK_AS_ADVANCED(LIBHIDAPIUSB_INCLUDE_DIR LIBHIDAPIUSB_LIBRARIES)

endif (LIBHIDAPIUSB_INCLUDE_DIR AND LIBHIDAPIUSB_LIBRARIES)