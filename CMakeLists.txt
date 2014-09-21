cmake_minimum_required(VERSION 2.8.7)
project(qemacs)
set(QE_VERSION 5.0)

set(CONFIG_UNLOCKIO OFF)
set(CONFIG_PTSNAME ON)
set(CONFIG_GPROF OFF)
set(CONFIG_NETWORK ON)
set(CONFIG_WIN32 OFF)
set(CONFIG_CYGWIN OFF)
set(CONFIG_DARWIN OFF)
set(CONFIG_HAIKU OFF)
set(CONFIG_QT OFF)
set(CONFIG_LSHARED OFF)
set(CONFIG_DLLLIBS "dl")
set(CONFIG_EXTRALIBS "")
set(CONFIG_SIMPLEIDCT ON)
set(CONFIG_BIGENDIAN OFF)
set(CONFIG_MPEGAUDIO_HP ON)

set(CONFIG_TINY OFF)
set(CONFIG_X11 OFF)
set(CONFIG_XV ON)
set(CONFIG_XRENDER ON)
set(CONFIG_PNG OFF)
set(CONFIG_FFMPEG OFF)
set(CONFIG_HTML OFF)
set(CONFIG_DOC ON)
set(CONFIG_PLUGINS ON)
set(CONFIG_INITCALLS ON)
set(CONFIG_MMAP ON)
set(CONFIG_ALL_KMAPS ON)
set(CONFIG_ALL_MODES ON)
set(CONFIG_UNICODE_JOIN ON)

if(${CMAKE_SYSTEM_NAME} MATCHES "BeOS")
  set(CONFIG_HAIKU ON)
else()
  # Linux
  set(CONFIG_UNLOCKIO ON)
  set(CONFIG_EXTRALIBS "m")
endif()

option(CONFIG_WIN32 "Win32 driver" CONFIG_WIN32)
option(CONFIG_CYGWIN "Cygwin" CONFIG_CYGWIN)
option(CONFIG_X11 "X11" CONFIG_X11)
option(CONFIG_QT "Qt" CONFIG_QT)
option(CONFIG_QT "Plugins" CONFIG_PLUGINS)

include_directories(${CMAKE_SOURCE_DIR})

find_package(Qt5Widgets)
option(CONFIG_QT "Qt 5.x driver" Qt5Widgets_FOUND)
#option(CONFIG_QT "Qt 5.x driver" OFF)

if(CONFIG_X11)
  find_package(X11 REQUIRED)
endif()

configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h)
add_definitions(-DHAVE_QE_CONFIG_H)
add_definitions(-DCONFIG_INIT_CALLS)

set(qemacs_common_SRCS
  qe.c
  parser.c
  charset.c
  buffer.c
  input.c
  display.c
  util.c
  hex.c
  list.c
  cutils.c
)

if(CONFIG_WIN32)
  set(qemacs_common_SRCS ${qemacs_common_SRCS} win32.c)
  set(qemacs_common_SRCS ${qemacs_common_SRCS} unix.c)
else(CONFIG_WIN32)
  set(qemacs_common_SRCS ${qemacs_common_SRCS} tty.c)
  set(qemacs_common_SRCS ${qemacs_common_SRCS} unix.c)
endif(CONFIG_WIN32)

set(qemacs_SRCS
  ${qemacs_common_SRCS}
  extras.c
  variables.c
)

set(qemacs_tiny_SRCS
  ${qemacs_common_SRCS}
)

if(CONFIG_HAIKU)
  set(qemacs_SRCS ${qemacs_SRCS} haiku.cpp)
endif(CONFIG_HAIKU)

if(CONFIG_QSCRIPT)
  set(qemacs_SRCS ${qemacs_SRCS} script.c)
# missing? eval.c)
endif(CONFIG_QSCRIPT)

if(CONFIG_ALL_KMAPS)
  set(qemacs_SRCS ${qemacs_SRCS} kmap.c)
endif(CONFIG_ALL_KMAPS)

# more charsets if needed
set(qemacs_SRCS ${qemacs_SRCS} charsetjis.c charsetmore.c)

if(CONFIG_ALL_MODES)
  set(qemacs_SRCS
    ${qemacs_SRCS}
    unihex.c
    bufed.c
    clang.c
    htmlsrc.c
    lisp.c
    makemode.c
    markdown.c
    orgmode.c
    perl.c
    script.c
    extra-modes.c
  )
  if(NOT WIN32)
    set(qemacs_SRCS ${qemacs_SRCS}
      shell.c
      dired.c
      latex-mode.c
      archive.c
  )
  endif(NOT WIN32)
endif(CONFIG_ALL_MODES)

if(CONFIG_QT)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Gui REQUIRED)
  find_package(Qt5Widgets REQUIRED)
  include_directories(
    ${Qt5Widgets_INCLUDES}
    ${Qt5Core_INCLUDES}
    ${Qt5Gui_INCLUDES}
  )
  add_definitions(
    ${Qt5Widgets_DEFINITIONS}
    ${Qt5Core_DEFINITIONS}
    ${Qt5Gui_DEFINITIONS}
  )
  set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
  set(qemacs_SRCS ${qemacs_SRCS} qt.cpp)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif(CONFIG_QT)

if(CONFIG_X11)
  set(qemacs_SRCS ${qemacs_SRCS} x11.c)
endif(CONFIG_X11)

# Not used in qemacs
if(CONFIG_CFB)
  set(qemacs_SRCS
    ${qemacs_SRCS}
    libfbf.c
    fbfrender.c
    cfb.c
    fbffonts.c
  )
endif(CONFIG_CFB)

if(CONFIG_UNICODE_JOIN)
  set(qemacs_SRCS
    ${qemacs_SRCS}
    unicode_join.c arabic.c indic.c qfribidi.c
  )
endif(CONFIG_UNICODE_JOIN)

set(libqhtml_SRCS
  libqhtml/csstoqe.c
  libqhtml/cssparse.c
  libqhtml/css.c
  libqhtml/csstoqe.c
  libqhtml/xmlparse.c
)
add_library(qhtml STATIC ${libqhtml_SRCS})

if(CONFIG_HTML)
  include_directories(${CMAKE_SOURCE_DIR}/libqhtml)
  set(qemacs_SRCS ${qemacs_SRCS} html.c docbook.c)
  if(OFF)
# if(NOT WIN32)
    add_executable(html2png
      html2png.c
      util.c
      cutils.c
      arabic.c
      indic.c
      qfribidi.c
      display.c
      unicode_join.c
      charset.c
      charsetmore.c
      charsetjis.c
      libfbf.c
      fbfrender.c
      cfb.c
      fbffonts.c
    )
    target_link_libraries(html2png qhtml)
    set_property(TARGET html2png APPEND PROPERTY COMPILE_DEFINITIONS QE_VERSION=${QE_VERSION})
  endif(NOT WIN32)
endif(CONFIG_HTML)

# This is the right way to generate basemodules.txt
# and allmosdules.txt
macro(generate_module_init_calls_broken filename sources)
  foreach(srcFile ${sources})
    file(STRINGS ${srcFile} fileInitCalls REGEX "^qe_module_init\\([a-z_^\\)]+\\);")
    foreach(initCall ${fileInitCalls})
      file(APPEND ${filename} "${initCall}\n")
    endforeach()
  endforeach()
endmacro(generate_module_init_calls_broken)

macro(generate_module_init_calls target sources)
  add_custom_command(
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT ${CMAKE_BINARY_DIR}/${target}.txt
    COMMAND grep -h \"^qe_module_init\" ${sources} > ${CMAKE_BINARY_DIR}/${target}.txt
#    COMMAND echo "Hello"
    DEPENDS ${sources}
  )
  add_custom_target(${target} DEPENDS ${CMAKE_BINARY_DIR}/${target}.txt)
endmacro(generate_module_init_calls)

#if(CONFIG_INIT_CALLS)
  generate_module_init_calls(allmodules "${qemacs_SRCS}")
  generate_module_init_calls(basemodules "${qemacs_common_SRCS}")

  set(qemacs_SRCS ${qemacs_SRCS} qeend.c)
  set(qemacs_tiny_SRCS ${qemacs_tiny_SRCS} qeend.c)
#endif(CONFIG_INIT_CALLS)

add_executable(qe ${qemacs_SRCS})
add_dependencies(qe allmodules)
target_link_libraries(qe dl)

if(CONFIG_QT)
  target_link_libraries(qe ${Qt5Widgets_LIBRARIES})
endif(CONFIG_QT)

if(CONFIG_X11)
  target_link_libraries(qe ${X11_LIBRARIES})
  if(CONFIG_XV)
    target_link_libraries(qe ${X11_Xv_LIB})
  endif(CONFIG_XV)
  if(CONFIG_XRENDER)
    target_link_libraries(qe ${X11_Xrender_LIB})
  endif(CONFIG_XRENDER)
endif(CONFIG_X11)

if(CONFIG_HTML)
  target_link_libraries(qe qhtml)
endif(CONFIG_HTML)

add_executable(tqe ${qemacs_tiny_SRCS})
add_dependencies(tqe basemodules)
set_property(TARGET tqe APPEND PROPERTY COMPILE_DEFINITIONS CONFIG_TINY=1)

