# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\appffmpegDemo_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\appffmpegDemo_autogen.dir\\ParseCache.txt"
  "appffmpegDemo_autogen"
  )
endif()
