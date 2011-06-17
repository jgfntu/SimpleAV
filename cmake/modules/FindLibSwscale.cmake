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

# Finds Swscale library
#
#  Swscale_INCLUDE_DIR - where to find swscale.h, etc.
#  Swscale_LIBRARIES   - List of libraries when using Swscale.
#  Swscale_FOUND       - True if Swscale found.
#

find_path(Swscale_INCLUDE_DIR1 libswscale/swscale.h
  /opt/local/include
  /usr/local/include
  /usr/include
  )
find_path(Swscale_INCLUDE_DIR2 ffmpeg/swscale.h
  /opt/local/include
  /usr/local/include
  /usr/include
  )
if ( Swscale_INCLUDE_DIR1 )
  set (AV_FORMAT_INCLUDE_FILE libswscale/swscale.h)
  set (Swscale_INCLUDE_DIR ${Swscale_INCLUDE_DIR1})
endif  ( Swscale_INCLUDE_DIR1 )
if ( Swscale_INCLUDE_DIR2 )
  set (AV_FORMAT_INCLUDE_FILE ffmpeg/swscale.h)
  set (Swscale_INCLUDE_DIR ${Swscale_INCLUDE_DIR2})
endif  ( Swscale_INCLUDE_DIR2 )

set(Swscale_NAMES swscale)
find_library(Swscale_LIBRARY
  NAMES ${Swscale_NAMES}
  PATHS /usr/lib /usr/local/lib /opt/local/lib
  )

if (Swscale_INCLUDE_DIR AND Swscale_LIBRARY)
   set(Swscale_FOUND TRUE)
   set( Swscale_LIBRARIES ${Swscale_LIBRARY} )
else (Swscale_INCLUDE_DIR AND Swscale_LIBRARY)
   set(Swscale_FOUND FALSE)
   set(Swscale_LIBRARIES)
endif (Swscale_INCLUDE_DIR AND Swscale_LIBRARY)

if (Swscale_FOUND)
   if (NOT Swscale_FIND_QUIETLY)
      message(STATUS "Found Swscale: ${Swscale_LIBRARY}")
   endif (NOT Swscale_FIND_QUIETLY)
else (Swscale_FOUND)
   if (Swscale_FIND_REQUIRED)
      message(STATUS "Looked for Swscale libraries named ${Swscale_NAMES}.")
      message(STATUS "Include file detected: [${Swscale_INCLUDE_DIR}].")
      message(STATUS "Lib file detected: [${Swscale_LIBRARY}].")
      message(FATAL_ERROR "=========> Could NOT find Swscale library")
   endif (Swscale_FIND_REQUIRED)
endif (Swscale_FOUND)

mark_as_advanced(
  Swscale_LIBRARY
  Swscale_INCLUDE_DIR
  )
