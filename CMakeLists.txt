cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

project( rModelIO)

set(WITH_ASSIMP TRUE)
set(WITH_LIBLAS FALSE)
set(WITH_RFEATURES TRUE)

# Don't set true here because the library shouldn't be compiled
# with host references to IDTFConverter. Ensure set TRUE for app
# CMakeFiles and set the corresponding variable in CMake.
set(WITH_IDTF_CONVERTER FALSE)
include( "cmake/FindLibs.cmake")

set( INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set( SRC_DIR "${PROJECT_SOURCE_DIR}/src")
include_directories( ${INCLUDE_DIR})

set( INCLUDE_FILES
    "${INCLUDE_DIR}/AssetImporter.h"
    "${INCLUDE_DIR}/IDTFExporter.h"
    "${INCLUDE_DIR}/LaTeXU3DInserter.h"
    "${INCLUDE_DIR}/OBJExporter.h"
    "${INCLUDE_DIR}/ObjModelExporter.h"
    "${INCLUDE_DIR}/ObjModelImporter.h"
    "${INCLUDE_DIR}/PDFGenerator.h"
    "${INCLUDE_DIR}/PLYExporter.h"
    "${INCLUDE_DIR}/U3DExporter.h"
    )

set( SRC_FILES
    ${SRC_DIR}/AssetImporter
    ${SRC_DIR}/IDTFExporter
    ${SRC_DIR}/LaTeXU3DInserter
    ${SRC_DIR}/OBJExporter
    ${SRC_DIR}/ObjModelExporter
    ${SRC_DIR}/ObjModelImporter
    ${SRC_DIR}/PDFGenerator
    ${SRC_DIR}/PLYExporter
    ${SRC_DIR}/U3DExporter
    )

add_library( ${PROJECT_NAME} ${SRC_FILES} ${INCLUDE_FILES})
include( "cmake/LinkLibs.cmake")
