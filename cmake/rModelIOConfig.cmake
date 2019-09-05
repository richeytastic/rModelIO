# =============================================================================
# The rModelIO CMake configuration file.
#
#           ** File generated automatically, DO NOT MODIFY! ***

# To use from an external project, in your project's CMakeLists.txt add:
#   FIND_PACKAGE( rModelIO REQUIRED)
#   INCLUDE_DIRECTORIES( rModelIO ${rModelIO_INCLUDE_DIRS})
#   LINK_DIRECTORIES( ${rModelIO_LIBRARY_DIR})
#   TARGET_LINK_LIBRARIES( MY_TARGET_NAME ${rModelIO_LIBRARIES})
#
# This module defines the following variables:
#   - rModelIO_FOUND         : True if rModelIO is found.
#   - rModelIO_ROOT_DIR      : The root directory where rModelIO is installed.
#   - rModelIO_INCLUDE_DIRS  : The rModelIO include directories.
#   - rModelIO_LIBRARY_DIR   : The rModelIO library directory.
#   - rModelIO_LIBRARIES     : The rModelIO imported libraries to link to.
#
# =============================================================================

get_filename_component( rModelIO_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component( rModelIO_ROOT_DIR  "${rModelIO_CMAKE_DIR}"           PATH)

set( rModelIO_INCLUDE_DIRS "${rModelIO_ROOT_DIR}/../include" CACHE PATH "The rModelIO include directories.")
set( rModelIO_LIBRARY_DIR  "${rModelIO_ROOT_DIR}"            CACHE PATH "The rModelIO library directory.")

include( "${CMAKE_CURRENT_LIST_DIR}/Macros.cmake")
get_library_suffix( _lsuff)
set( _hints rModelIO${_lsuff} librModelIO${_lsuff})
find_library( rModelIO_LIBRARIES NAMES ${_hints} PATHS "${rModelIO_LIBRARY_DIR}/static" "${rModelIO_LIBRARY_DIR}")
set( rModelIO_LIBRARIES     ${rModelIO_LIBRARIES}         CACHE FILEPATH "The rModelIO imported libraries to link to.")

# handle QUIETLY and REQUIRED args and set rModelIO_FOUND to TRUE if all listed variables are TRUE
include( "${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake")
find_package_handle_standard_args( rModelIO rModelIO_FOUND rModelIO_LIBRARIES rModelIO_INCLUDE_DIRS)
