cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project( rModelIO)

set(WITH_ASSIMP TRUE)
set(WITH_LIBLAS FALSE)
set(WITH_RFEATURES TRUE)
set(WITH_QT TRUE)   # Needed by VCG!
set(WITH_VCG TRUE)
include( "cmake/LibCommon.cmake")

set( INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set( SRC_DIR "${PROJECT_SOURCE_DIR}/src")
include_directories( ${INCLUDE_DIR})

set( INCLUDE_FILES
    "${INCLUDE_DIR}/IOUtils.h"
    #"${INCLUDE_DIR}/AAMReader.h"
    "${INCLUDE_DIR}/AssetImporter.h"
    "${INCLUDE_DIR}/AssetExporter.h"
    #"${INCLUDE_DIR}/LASImporter.h"
    "${INCLUDE_DIR}/ObjModelExporter.h"
    "${INCLUDE_DIR}/ObjModel2VCG.h"
    "${INCLUDE_DIR}/U3DExporter.h"
    "${INCLUDE_DIR}/VCGTypes.h"
    )

set( SRC_FILES
    ${SRC_DIR}/IOUtils
    #${SRC_DIR}/AAMReader
    ${SRC_DIR}/AssetImporter
    ${SRC_DIR}/AssetExporter
    #${SRC_DIR}/LASImporter
    ${SRC_DIR}/ObjModelExporter
    ${SRC_DIR}/ObjModel2VCG
    ${SRC_DIR}/U3DExporter
    )

add_library( ${PROJECT_NAME} ${SRC_FILES} ${INCLUDE_FILES})
include( "cmake/LibInstall.cmake")
