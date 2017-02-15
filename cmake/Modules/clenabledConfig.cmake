INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_CLENABLED clenabled)

FIND_PATH(
    CLENABLED_INCLUDE_DIRS
    NAMES clenabled/api.h
    HINTS $ENV{CLENABLED_DIR}/include
        ${PC_CLENABLED_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    CLENABLED_LIBRARIES
    NAMES gnuradio-clenabled
    HINTS $ENV{CLENABLED_DIR}/lib
        ${PC_CLENABLED_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CLENABLED DEFAULT_MSG CLENABLED_LIBRARIES CLENABLED_INCLUDE_DIRS)
MARK_AS_ADVANCED(CLENABLED_LIBRARIES CLENABLED_INCLUDE_DIRS)

