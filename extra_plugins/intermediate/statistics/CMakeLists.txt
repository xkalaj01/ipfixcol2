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
        src/Statistics.cpp
        src/Interface.h
        src/snmp/snmp.cpp
        src/Interface.cpp
        src/Service.h
        src/snmp/snmp.h
        src/Storage.cpp
        src/Storage.h
        src/Config.cpp
        src/Config.h

        src/snmp/mib_tables/ipfixTransportSessionTable.cpp
        src/snmp/mib_tables/ipfixTemplateTable.cpp
        src/snmp/mib_tables/ipfixTemplateDefinitionTable.cpp
        src/snmp/mib_tables/ipfixTransportSessionStatsTable.cpp
        src/snmp/mib_tables/ipfixTemplateStatsTable.cpp
        src/file/TextFile.cpp

        src/snmp/mib_tables/ipfixTransportSessionTable.h
        src/snmp/mib_tables/ipfixTemplateTable.h
        src/snmp/mib_tables/ipfixTemplateDefinitionTable.h
        src/snmp/mib_tables/ipfixTransportSessionStatsTable.h
        src/snmp/mib_tables/ipfixTemplateStatsTable.h
        src/file/TextFile.h)

target_link_libraries( statistics-intermediate
#        ${FDS_LIBRARIES}              # libfds
        ${NETSNMP_LIBRARIES}          # libnetsnmp
)

#TODO

install(
    TARGETS statistics-intermediate
    LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/ipfixcol2/"
)


if (ENABLE_DOC_MANPAGE)
    find_package(Rst2Man)
    if (NOT RST2MAN_FOUND)
        message(FATAL_ERROR "rst2man is not available. Install python-docutils or disable manual page generation (-DENABLE_DOC_MANPAGE=False)")
    endif()

    # Build a manual page
    set(SRC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/doc/ipfixcol2-statistics-inter.7.rst")
    set(DST_FILE "${CMAKE_CURRENT_BINARY_DIR}/ipfixcol2-statistics-inter.7")

    add_custom_command(TARGET statistics-intermediate PRE_BUILD
        COMMAND ${RST2MAN_EXECUTABLE} --syntax-highlight=none ${SRC_FILE} ${DST_FILE}
        DEPENDS ${SRC_FILE}
        VERBATIM
        )

    install(
        FILES "${DST_FILE}"
        DESTINATION "${CMAKE_INSTALL_FULL_MANDIR}/man7"
    )
endif()
