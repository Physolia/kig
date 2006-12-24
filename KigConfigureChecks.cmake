include(CheckIncludeFiles)
include(CheckFunctionExists)

check_include_files(ieeefp.h HAVE_IEEEFP_H)

set(CMAKE_REQUIRED_INCLUDES "math.h")
set(CMAKE_REQUIRED_LIBRARIES m)
check_function_exists(trunc      HAVE_TRUNC)
set(CMAKE_REQUIRED_INCLUDES)
set(CMAKE_REQUIRED_LIBRARIES)

macro_optional_find_package(BoostPython)

# at the end, output the configuration
configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/config-kig.h.cmake
   ${CMAKE_CURRENT_BINARY_DIR}/config-kig.h
)

macro_log_feature(
   BOOST_PYTHON_FOUND
   "Boost.Python"
   "Kig can optionally use Boost.Python for Python scripting"
   "http://www.boost.org/"
   FALSE
   "1.31"
   ""
)

