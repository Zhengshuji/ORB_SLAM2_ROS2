# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_orb_slam2_stereo_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED orb_slam2_stereo_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(orb_slam2_stereo_FOUND FALSE)
  elseif(NOT orb_slam2_stereo_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(orb_slam2_stereo_FOUND FALSE)
  endif()
  return()
endif()
set(_orb_slam2_stereo_CONFIG_INCLUDED TRUE)

# output package information
if(NOT orb_slam2_stereo_FIND_QUIETLY)
  message(STATUS "Found orb_slam2_stereo: 1.0.0 (${orb_slam2_stereo_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'orb_slam2_stereo' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${orb_slam2_stereo_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(orb_slam2_stereo_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${orb_slam2_stereo_DIR}/${_extra}")
endforeach()
