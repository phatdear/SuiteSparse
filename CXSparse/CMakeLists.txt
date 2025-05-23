#-------------------------------------------------------------------------------
# CXSparse/CMakeLists.txt:  cmake for CXSparse
#-------------------------------------------------------------------------------

# Copyright (c) 2006-2023, Timothy A. Davis.  All Rights Reserved.
# SPDX-License-Identifier: LGPL-2.1+

#-------------------------------------------------------------------------------
# get the version
#-------------------------------------------------------------------------------

cmake_minimum_required ( VERSION 3.22 )

set ( CXSPARSE_DATE "June 20, 2024" )
set ( CXSPARSE_VERSION_MAJOR 4 CACHE STRING "" FORCE )
set ( CXSPARSE_VERSION_MINOR 4 CACHE STRING "" FORCE )
set ( CXSPARSE_VERSION_SUB   1 CACHE STRING "" FORCE )

message ( STATUS "Building CXSparse version: v"
    ${CXSPARSE_VERSION_MAJOR}.
    ${CXSPARSE_VERSION_MINOR}.
    ${CXSPARSE_VERSION_SUB} " (" ${CXSPARSE_DATE} ")" )

#-------------------------------------------------------------------------------
# define the project
#-------------------------------------------------------------------------------

project ( CXSparse
    VERSION "${CXSPARSE_VERSION_MAJOR}.${CXSPARSE_VERSION_MINOR}.${CXSPARSE_VERSION_SUB}"
    LANGUAGES C )

#-------------------------------------------------------------------------------
# SuiteSparse policies
#-------------------------------------------------------------------------------

set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    ${PROJECT_SOURCE_DIR}/../SuiteSparse_config/cmake_modules )

include ( SuiteSparsePolicy )

#-------------------------------------------------------------------------------
# complex support
#-------------------------------------------------------------------------------

# MS Visual Studio does not support the complex type for ANSI C11.
# FIXME: see GraphBLAS for how to use complex types in MS Visual Studio.

if ( MSVC OR ("${CMAKE_C_SIMULATE_ID}" STREQUAL "MSVC") )
    option ( CXSPARSE_USE_COMPLEX "ON: complex data type enabled.  OFF (default): complex data type disabled." OFF )
else ( )
    option ( CXSPARSE_USE_COMPLEX "ON (default): complex data type enabled.  OFF: complex data type disabled." ON )
endif ( )

if ( CXSPARSE_USE_COMPLEX )
    message ( STATUS "complex data type: enabled" )
    set ( CXSPARSE_HAS_COMPLEX 1 )
else ( )
    message ( STATUS "complex data type: disabled" )
    add_compile_definitions ( NCOMPLEX )
    set ( CXSPARSE_HAS_COMPLEX 0 )
endif ( )

# check for strict usage
if ( SUITESPARSE_USE_STRICT AND CXSPARSE_USE_COMPLEX AND NOT CXSPARSE_HAS_COMPLEX )
    message ( FATAL_ERROR "complex types required for CXSparse but not available" )
endif ( )

#-------------------------------------------------------------------------------
# find library dependencies
#-------------------------------------------------------------------------------

if ( NOT SUITESPARSE_ROOT_CMAKELISTS )
    find_package ( SuiteSparse_config 7.8.0
        PATHS ${CMAKE_SOURCE_DIR}/../SuiteSparse_config/build NO_DEFAULT_PATH )
    if ( NOT TARGET SuiteSparse::SuiteSparseConfig )
        find_package ( SuiteSparse_config 7.8.0 REQUIRED )
    endif ( )
endif ( )

#-------------------------------------------------------------------------------
# Configure cs.h with version number
#-------------------------------------------------------------------------------

configure_file ( "Config/cs.h.in"
    "${PROJECT_SOURCE_DIR}/Include/cs.h"
    NEWLINE_STYLE LF )

#-------------------------------------------------------------------------------
# include directories
#-------------------------------------------------------------------------------

include_directories ( Source Include )

#-------------------------------------------------------------------------------
# dynamic cxsparse library properties
#-------------------------------------------------------------------------------

if ( CXSPARSE_HAS_COMPLEX )
    file ( GLOB CXSPARSE_SOURCES "Source/*.c" )
else ( )
    # exclude functions that work on complex data types
    file ( GLOB CXSPARSE_SOURCES
        "Source/cs_a*.c"
        "Source/cs_chol*.c"
        "Source/cs_compress.c"
        "Source/cs_counts.c"
        "Source/cs_cumsum.c"
        "Source/cs_[d-z]*.c"
        "Source/cxsparse_version.c"
        )
endif ( )

if ( BUILD_SHARED_LIBS )
    add_library ( CXSparse SHARED ${CXSPARSE_SOURCES} )

    set_target_properties ( CXSparse PROPERTIES
        VERSION ${CXSPARSE_VERSION_MAJOR}.${CXSPARSE_VERSION_MINOR}.${CXSPARSE_VERSION_SUB}
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        OUTPUT_NAME cxsparse
        SOVERSION ${CXSPARSE_VERSION_MAJOR}
        PUBLIC_HEADER "Include/cs.h"
        WINDOWS_EXPORT_ALL_SYMBOLS ON )

    if ( ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.25" )
        set_target_properties ( CXSparse PROPERTIES EXPORT_NO_SYSTEM ON )
    endif ( )

    target_include_directories ( CXSparse
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
                  $<INSTALL_INTERFACE:${SUITESPARSE_INCLUDEDIR}> )
endif ( )

#-------------------------------------------------------------------------------
# static cxsparse library properties
#-------------------------------------------------------------------------------

if ( BUILD_STATIC_LIBS )
    add_library ( CXSparse_static STATIC ${CXSPARSE_SOURCES} )

    set_target_properties ( CXSparse_static PROPERTIES
        OUTPUT_NAME cxsparse
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        PUBLIC_HEADER "Include/cs.h" )

    if ( MSVC OR ("${CMAKE_C_SIMULATE_ID}" STREQUAL "MSVC") )
        set_target_properties ( CXSparse_static PROPERTIES
            OUTPUT_NAME cxsparse_static )
    endif ( )

    if ( ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.25" )
        set_target_properties ( CXSparse_static PROPERTIES EXPORT_NO_SYSTEM ON )
    endif ( )

    target_include_directories ( CXSparse_static
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
                  $<INSTALL_INTERFACE:${SUITESPARSE_INCLUDEDIR}> )
endif ( )

#-------------------------------------------------------------------------------
# add the library dependencies
#-------------------------------------------------------------------------------

if ( BUILD_SHARED_LIBS )
    target_link_libraries ( CXSparse PRIVATE SuiteSparse::SuiteSparseConfig )
    target_include_directories ( CXSparse PUBLIC
        "$<TARGET_PROPERTY:SuiteSparse::SuiteSparseConfig,INTERFACE_INCLUDE_DIRECTORIES>" )
endif ( )
if ( BUILD_STATIC_LIBS )
    if ( TARGET SuiteSparse::SuiteSparseConfig_static )
        target_link_libraries ( CXSparse_static PUBLIC SuiteSparse::SuiteSparseConfig_static )
    else ( )
        target_link_libraries ( CXSparse_static PUBLIC SuiteSparse::SuiteSparseConfig )
    endif ( )
endif ( )

# libm:
if ( NOT WIN32 )
    if ( BUILD_SHARED_LIBS )
        target_link_libraries ( CXSparse PRIVATE m )
    endif ( )
    if ( BUILD_STATIC_LIBS )
        set ( CXSPARSE_STATIC_LIBS "${CXSPARSE_STATIC_LIBS} -lm" )
        target_link_libraries ( CXSparse_static PUBLIC m )
    endif ( )
endif ( )

#-------------------------------------------------------------------------------
# installation location
#-------------------------------------------------------------------------------

include ( CMakePackageConfigHelpers )

if ( BUILD_SHARED_LIBS )
    install ( TARGETS CXSparse
        EXPORT CXSparseTargets
        LIBRARY DESTINATION ${SUITESPARSE_LIBDIR}
        ARCHIVE DESTINATION ${SUITESPARSE_LIBDIR}
        RUNTIME DESTINATION ${SUITESPARSE_BINDIR}
        PUBLIC_HEADER DESTINATION ${SUITESPARSE_INCLUDEDIR} )
endif ( )
if ( BUILD_STATIC_LIBS )
    install ( TARGETS CXSparse_static
        EXPORT CXSparseTargets
        ARCHIVE DESTINATION ${SUITESPARSE_LIBDIR}
        PUBLIC_HEADER DESTINATION ${SUITESPARSE_INCLUDEDIR} )
endif ( )

# create (temporary) export target file during build
export ( EXPORT CXSparseTargets
    NAMESPACE SuiteSparse::
    FILE ${CMAKE_CURRENT_BINARY_DIR}/CXSparseTargets.cmake )

# install export target, config and version files for find_package
install ( EXPORT CXSparseTargets
    NAMESPACE SuiteSparse::
    DESTINATION ${SUITESPARSE_PKGFILEDIR}/cmake/CXSparse )

# generate config file to be used in common build tree
set ( SUITESPARSE_IN_BUILD_TREE ON )
configure_package_config_file (
    Config/CXSparseConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CXSparseConfig.cmake
    INSTALL_DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/CXSparseConfig.cmake )

# generate config file to be installed
set ( SUITESPARSE_IN_BUILD_TREE OFF )
configure_package_config_file (
    Config/CXSparseConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/target/CXSparseConfig.cmake
    INSTALL_DESTINATION ${SUITESPARSE_PKGFILEDIR}/cmake/CXSparse )

write_basic_package_version_file (
    ${CMAKE_CURRENT_BINARY_DIR}/CXSparseConfigVersion.cmake
    COMPATIBILITY SameMajorVersion )

install ( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/target/CXSparseConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/CXSparseConfigVersion.cmake
    DESTINATION ${SUITESPARSE_PKGFILEDIR}/cmake/CXSparse )

#-------------------------------------------------------------------------------
# create pkg-config file
#-------------------------------------------------------------------------------

if ( NOT MSVC )
    set ( prefix "${CMAKE_INSTALL_PREFIX}" )
    set ( exec_prefix "\${prefix}" )
    cmake_path ( IS_ABSOLUTE SUITESPARSE_LIBDIR SUITESPARSE_LIBDIR_IS_ABSOLUTE )
    if (SUITESPARSE_LIBDIR_IS_ABSOLUTE)
        set ( libdir "${SUITESPARSE_LIBDIR}")
    else ( )
        set ( libdir "\${exec_prefix}/${SUITESPARSE_LIBDIR}")
    endif ( )
    cmake_path ( IS_ABSOLUTE SUITESPARSE_INCLUDEDIR SUITESPARSE_INCLUDEDIR_IS_ABSOLUTE )
    if (SUITESPARSE_INCLUDEDIR_IS_ABSOLUTE)
        set ( includedir "${SUITESPARSE_INCLUDEDIR}")
    else ( )
        set ( includedir "\${prefix}/${SUITESPARSE_INCLUDEDIR}")
    endif ( )
    if ( BUILD_SHARED_LIBS )
        set ( SUITESPARSE_LIB_BASE_NAME $<TARGET_FILE_BASE_NAME:CXSparse> )
    else ( )
        set ( SUITESPARSE_LIB_BASE_NAME $<TARGET_FILE_BASE_NAME:CXSparse_static> )
    endif ( )
    configure_file (
        Config/CXSparse.pc.in
        CXSparse.pc.out
        @ONLY
        NEWLINE_STYLE LF )
    file ( GENERATE
        OUTPUT CXSparse.pc
        INPUT ${CMAKE_CURRENT_BINARY_DIR}/CXSparse.pc.out
        NEWLINE_STYLE LF )
    install ( FILES
        ${CMAKE_CURRENT_BINARY_DIR}/CXSparse.pc
        DESTINATION ${SUITESPARSE_PKGFILEDIR}/pkgconfig )
endif ( )

#-------------------------------------------------------------------------------
# Demo library and programs
#-------------------------------------------------------------------------------

if ( SUITESPARSE_DEMOS )

    #---------------------------------------------------------------------------
    # demo library
    #---------------------------------------------------------------------------

    message ( STATUS "Also compiling the demos in CXSparse/Demo" )

    #---------------------------------------------------------------------------
    # Demo programs
    #---------------------------------------------------------------------------

    add_executable ( cs_demo1 "Demo/cs_demo1.c" "Demo/cs_demo.c" )
    add_executable ( cs_demo2 "Demo/cs_demo2.c" "Demo/cs_demo.c" )
    add_executable ( cs_demo3 "Demo/cs_demo3.c" "Demo/cs_demo.c" )

    add_executable ( cs_di_demo1 "Demo/cs_di_demo1.c" "Demo/cs_di_demo.c" )
    add_executable ( cs_di_demo2 "Demo/cs_di_demo2.c" "Demo/cs_di_demo.c" )
    add_executable ( cs_di_demo3 "Demo/cs_di_demo3.c" "Demo/cs_di_demo.c" )

    add_executable ( cs_dl_demo1 "Demo/cs_dl_demo1.c" "Demo/cs_dl_demo.c" )
    add_executable ( cs_dl_demo2 "Demo/cs_dl_demo2.c" "Demo/cs_dl_demo.c" )
    add_executable ( cs_dl_demo3 "Demo/cs_dl_demo3.c" "Demo/cs_dl_demo.c" )

    if ( CXSPARSE_HAS_COMPLEX )
        add_executable ( cs_ci_demo1 "Demo/cs_ci_demo1.c" "Demo/cs_ci_demo.c" )
        add_executable ( cs_ci_demo2 "Demo/cs_ci_demo2.c" "Demo/cs_ci_demo.c" )
        add_executable ( cs_ci_demo3 "Demo/cs_ci_demo3.c" "Demo/cs_ci_demo.c" )

        add_executable ( cs_cl_demo1 "Demo/cs_cl_demo1.c" "Demo/cs_cl_demo.c" )
        add_executable ( cs_cl_demo2 "Demo/cs_cl_demo2.c" "Demo/cs_cl_demo.c" )
        add_executable ( cs_cl_demo3 "Demo/cs_cl_demo3.c" "Demo/cs_cl_demo.c" )

        add_executable ( cs_idemo "Demo/cs_idemo.c" )
        add_executable ( cs_ldemo "Demo/cs_ldemo.c" )
    endif ( )

    # Libraries required for Demo programs
    if ( BUILD_SHARED_LIBS )
        target_link_libraries ( cs_demo1 PUBLIC CXSparse )
        target_link_libraries ( cs_demo2 PUBLIC CXSparse )
        target_link_libraries ( cs_demo3 PUBLIC CXSparse )

        target_link_libraries ( cs_di_demo1 PUBLIC CXSparse )
        target_link_libraries ( cs_di_demo2 PUBLIC CXSparse )
        target_link_libraries ( cs_di_demo3 PUBLIC CXSparse )

        target_link_libraries ( cs_dl_demo1 PUBLIC CXSparse )
        target_link_libraries ( cs_dl_demo2 PUBLIC CXSparse )
        target_link_libraries ( cs_dl_demo3 PUBLIC CXSparse )
    else ( )
        target_link_libraries ( cs_demo1 PUBLIC CXSparse_static )
        target_link_libraries ( cs_demo2 PUBLIC CXSparse_static )
        target_link_libraries ( cs_demo3 PUBLIC CXSparse_static )

        target_link_libraries ( cs_di_demo1 PUBLIC CXSparse_static )
        target_link_libraries ( cs_di_demo2 PUBLIC CXSparse_static )
        target_link_libraries ( cs_di_demo3 PUBLIC CXSparse_static )

        target_link_libraries ( cs_dl_demo1 PUBLIC CXSparse_static )
        target_link_libraries ( cs_dl_demo2 PUBLIC CXSparse_static )
        target_link_libraries ( cs_dl_demo3 PUBLIC CXSparse_static )
    endif ( )

    if ( CXSPARSE_HAS_COMPLEX )
        if ( BUILD_SHARED_LIBS )
            target_link_libraries ( cs_ci_demo1 PUBLIC CXSparse )
            target_link_libraries ( cs_ci_demo2 PUBLIC CXSparse )
            target_link_libraries ( cs_ci_demo3 PUBLIC CXSparse )

            target_link_libraries ( cs_cl_demo1 PUBLIC CXSparse )
            target_link_libraries ( cs_cl_demo2 PUBLIC CXSparse )
            target_link_libraries ( cs_cl_demo3 PUBLIC CXSparse )
        else ( )
            target_link_libraries ( cs_ci_demo1 PUBLIC CXSparse_static )
            target_link_libraries ( cs_ci_demo2 PUBLIC CXSparse_static )
            target_link_libraries ( cs_ci_demo3 PUBLIC CXSparse_static )

            target_link_libraries ( cs_cl_demo1 PUBLIC CXSparse_static )
            target_link_libraries ( cs_cl_demo2 PUBLIC CXSparse_static )
            target_link_libraries ( cs_cl_demo3 PUBLIC CXSparse_static )
        endif ( )

        if ( NOT WIN32 )
            target_link_libraries ( cs_ci_demo1 PUBLIC m )
            target_link_libraries ( cs_ci_demo2 PUBLIC m )
            target_link_libraries ( cs_ci_demo3 PUBLIC m )

            target_link_libraries ( cs_cl_demo1 PUBLIC m )
            target_link_libraries ( cs_cl_demo2 PUBLIC m )
            target_link_libraries ( cs_cl_demo3 PUBLIC m )
        endif ( )

        if ( BUILD_SHARED_LIBS )
            target_link_libraries ( cs_idemo PUBLIC CXSparse )
            target_link_libraries ( cs_ldemo PUBLIC CXSparse )
        else ( )
            target_link_libraries ( cs_idemo PUBLIC CXSparse_static )
            target_link_libraries ( cs_ldemo PUBLIC CXSparse_static )
        endif ( )
    endif ( )

else ( )

    message ( STATUS "Skipping the demos in CXSparse/Demo" )

endif ( )

#-------------------------------------------------------------------------------
# report status
#-------------------------------------------------------------------------------

include ( SuiteSparseReport )
