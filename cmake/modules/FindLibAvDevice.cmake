# Copyright (c) 2009, Whispersoft s.r.l.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
# * Neither the name of Whispersoft s.r.l. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# Finds AvDevice library
#
#  AvDevice_INCLUDE_DIR - where to find avdevice.h, etc.
#  AvDevice_LIBRARIES   - List of libraries when using AvDevice.
#  AvDevice_FOUND       - True if AvDevice found.
#

find_path(AvDevice_INCLUDE_DIR1 libavdevice/avdevice.h
  /opt/local/include
  /usr/local/include
  /usr/include
  )
find_path(AvDevice_INCLUDE_DIR2 ffmpeg/avdevice.h
  /opt/local/include
  /usr/local/include
  /usr/include
  )
if ( AvDevice_INCLUDE_DIR1 )
  set (AV_CODEC_INCLUDE_FILE libavdevice/avdevice.h)
  set (AvDevice_INCLUDE_DIR ${AvDevice_INCLUDE_DIR1})
endif  ( AvDevice_INCLUDE_DIR1 )
if ( AvDevice_INCLUDE_DIR2 )
  set (AV_CODEC_INCLUDE_FILE ffmpeg/avdevice.h)
  set (AvDevice_INCLUDE_DIR ${AvDevice_INCLUDE_DIR2})
endif  ( AvDevice_INCLUDE_DIR2 )

set(AvDevice_NAMES avdevice)
find_library(AvDevice_LIBRARY
  NAMES ${AvDevice_NAMES}
  PATHS /usr/lib /usr/local/lib /opt/local/lib
)

if (AvDevice_INCLUDE_DIR AND AvDevice_LIBRARY)
   set(AvDevice_FOUND TRUE)
   set( AvDevice_LIBRARIES ${AvDevice_LIBRARY} )
else (AvDevice_INCLUDE_DIR AND AvDevice_LIBRARY)
   set(AvDevice_FOUND FALSE)
   set(AvDevice_LIBRARIES)
endif (AvDevice_INCLUDE_DIR AND AvDevice_LIBRARY)

if (AvDevice_FOUND)
   if (NOT AvDevice_FIND_QUIETLY)
      message(STATUS "Found AvDevice: ${AvDevice_LIBRARY}")
   endif (NOT AvDevice_FIND_QUIETLY)
else (AvDevice_FOUND)
   if (AvDevice_FIND_REQUIRED)
      message(STATUS "Looked for AvDevice libraries named ${AvDevice_NAMES}.")
      message(STATUS "Include file detected: [${AvDevice_INCLUDE_DIR}].")
      message(STATUS "Lib file detected: [${AvDevice_LIBRARY}].")
      message(FATAL_ERROR "=========> Could NOT find AvDevice library")
   endif (AvDevice_FIND_REQUIRED)
endif (AvDevice_FOUND)

mark_as_advanced(
  AvDevice_LIBRARY
  AvDevice_INCLUDE_DIR
  )
