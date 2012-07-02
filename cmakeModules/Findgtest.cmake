# - Try to find gtest
#
#  This module defines the following variables
#
#  gtest_FOUND - Was library found
#  gtest_INCLUDE_DIRS - the include directories
#
#  This module accepts the following variables
#
#  gtest_DIR - Set to location of DCL if not in PATH or current directory ThirdParty
#  gtest_VERSION - Version of DCL to look for
#  THIRDPARTY_DIR - Location of third party directory to perform checkouts int
#

macro(_FIND_GTEST_HEADER _include_dir)
	find_path(gtest_include_dir NAMES gtest/gtest.h
		HINTS
			${gtest_DIR}/include
			${THIRDPARTY_DIR}/gtest-${gtest_VERSION}/include
	)
	set(${_include_dir} ${gtest_include_dir})
endmacro()

# Find the headers
_FIND_GTEST_HEADER(gtest_INCLUDE_DIR)

# handle the QUIETLY and REQUIRED arguments and set SomeMarkitLibrary_FOUND to TRUE if 
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(gtest DEFAULT_MSG gtest_INCLUDE_DIR)

set(gtest_INCLUDE_DIRS ${gtest_INCLUDE_DIR})

include_directories(${gtest_INCLUDE_DIRS})
