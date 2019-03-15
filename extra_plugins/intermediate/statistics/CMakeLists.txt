cmake_minimum_required(VERSION 2.8.8)
project(statistics)

# Description of the project
set(STATISTICS_DESCRIPTION
        "Intermediate plugin for IPFIXcol2 that collects statistics about collector's activity."
        )

set(STATISTICS_VERSION_MAJOR 1)
set(STATISTICS_VERSION_MINOR 0)
set(STATISTICS_VERSION_PATCH 0)
set(STATISTICS_VERSION
        ${STATISTICS_VERSION_MAJOR}.${STATISTICS_VERSION_MINOR}.${STATISTICS_VERSION_PATCH})

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(GNUInstallDirs)

# Include custom FindXXX modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules/")

find_package(IPFIXcol2 2.0.0 REQUIRED)
#find_package(LibFds REQUIRED)
find_package(NetSNMP REQUIRED)

# Set default build type if not specified by user
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release
            CACHE STRING "Choose type of build (Release/Debug)." FORCE)
endif()

option(ENABLE_DOC_MANPAGE    "Enable manual page building"              ON)

# Hard coded definitions
set(CMAKE_C_FLAGS            "${CMAKE_C_FLAGS} -fvisibility=hidden -std=gnu11")
set(CMAKE_C_FLAGS_RELEASE    "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG      "-g -O0 -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS          "${CMAKE_CXX_FLAGS} -fvisibility=hidden -std=gnu++11")
set(CMAKE_CXX_FLAGS_RELEASE  "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG    "-g -O0 -Wall -Wextra -pedantic")

# Header files for source code building
include_directories(
        "${IPFIXCOL2_INCLUDE_DIRS}"  # IPFIXcol2 header files
#        "${FDS_INCLUDE_DIRS}"        # libfds header files
#        "${NETSNMP_INCLUDE_DIR}"  # NetSNMP header files
)

# Create a linkable module
add_library(statistics-intermediate MODULE
        src/statistics.cpp
        src/map.h
        src/map.cpp
        src/interface.h
        src/snmp/snmp.cpp
        src/snmp/mib.h
        src/snmp/mib.cpp src/interface.cpp src/service.h src/snmp/snmp.h)

target_link_libraries( statistics-intermediate
#        ${FDS_LIBRARIES}              # libfds
        ${NETSNMP_LIBRARIES}          # libnetsnmp
)

#TODO

install(
    TARGETS statistics-intermediate
    LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/ipfixcol2/"
)


#if (ENABLE_DOC_MANPAGE)
    # Build a manual page
 #   set(SRC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/doc/ipfixcol2-anonymization-inter.7.rst")
  #  set(DST_FILE "${CMAKE_CURRENT_BINARY_DIR}/ipfixcol2-anonymization-inter.7")

   # add_custom_command(TARGET anonymization-intermediate PRE_BUILD
    #    COMMAND ${RST2MAN_EXECUTABLE} --syntax-highlight=none ${SRC_FILE} ${DST_FILE}
     #   DEPENDS ${SRC_FILE}
      #  VERBATIM
       # )

   # install(
   #     FILES "${DST_FILE}"
   #     DESTINATION "${INSTALL_DIR_MAN}/man7"
   # )
#endif()
