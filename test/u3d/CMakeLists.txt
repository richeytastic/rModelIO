cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

PROJECT( u3d)

set(WITH_TESTUTILS TRUE)
set(WITH_IDTF_CONVERTER TRUE)
include( "$ENV{DEV_PARENT_DIR}/libbuild/cmake/FindLibs.cmake")

set( INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
#include_directories( ${INCLUDE_DIR})
#set( INCLUDE_FILES
#    )

set( SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set( SRC_FILES
    ${SRC_DIR}/main
    )

add_executable( ${PROJECT_NAME} ${SRC_FILES} ${INCLUDE_FILES})
set_target_properties( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
# If Windows, separate binaries from build data because need to copy in 3rd party dlls
include( "$ENV{DEV_PARENT_DIR}/libbuild/cmake/ExeInstall.cmake")

get_target_property( exedir ${PROJECT_NAME} RUNTIME_OUTPUT_DIRECTORY)
file( COPY "${TestUtils_ROOT_DIR}/tx0.png" DESTINATION "${exedir}")
file( COPY "${PROJECT_SOURCE_DIR}/doc" DESTINATION "${PROJECT_BINARY_DIR}")

