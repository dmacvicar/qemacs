cmake_minimum_required(VERSION 3.0)
project(qemacs)
set(QE_VERSION 5.0)
include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)

set(CONFIG_UNLOCKIO OFF)
set(CONFIG_PTSNAME ON)
set(CONFIG_GPROF OFF)
set(CONFIG_NETWORK ON)
set(CONFIG_WIN32 OFF)
set(CONFIG_CYGWIN OFF)
set(CONFIG_DARWIN OFF)
set(CONFIG_HAIKU OFF)
set(CONFIG_QT OFF)
set(CONFIG_GTK OFF)
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
set(CONFIG_QE_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CONFIG_QE_DATADIR ${CONFIG_QE_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/qemacs)

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
option(CONFIG_NETWORK "Network" CONFIG_NETWORK)
option(CONFIG_QT "Qt" CONFIG_QT)
option(CONFIG_PLUGINS "Plugins" CONFIG_PLUGINS)
option(CONFIG_TINY "Tiny" CONFIG_TINY)

include_directories(${CMAKE_SOURCE_DIR})

find_package(Qt5Widgets)
option(CONFIG_QT "Qt 5.x driver" Qt5Widgets_FOUND)
#option(CONFIG_QT "Qt 5.x driver" OFF)

option(CONFIG_GTK "Gtk" ${CONFIG_GTK})

if(CONFIG_GTK)
  pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
endif()


if(CONFIG_X11)
  find_package(X11 REQUIRED)
endif()

#add_subdirectory(ext)

include(FetchContent)

FetchContent_Declare(treesitter
  GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(treesitter)

include_directories(
    ${treesitter_SOURCE_DIR}/include
    ${treesitter_SOURCE_DIR}/src
)

FetchContent_Declare(treesitter_json
  GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-json.git
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(treesitter_json)

FetchContent_Declare(treesitter_go
  GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-go.git
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(treesitter_go)

include_directories(
    ${treesitter_SOURCE_DIR}/include
    ${treesitter_SOURCE_DIR}/src
)

configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h)
include_directories(${CMAKE_BINARY_DIR})
add_definitions(-DHAVE_QE_CONFIG_H)
add_definitions(-DCONFIG_INIT_CALLS)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Debug build.")
  add_definitions(-DDEBUG)
endif()

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
  ${treesitter_SOURCE_DIR}/lib/src/lib.c
  treesitter.c
  ${treesitter_json_SOURCE_DIR}/src/parser.c
  ${treesitter_go_SOURCE_DIR}/src/parser.c
)

if(CONFIG_WIN32)
  set(qemacs_common_SRCS ${qemacs_common_SRCS} win32.c)
  set(qemacs_common_SRCS ${qemacs_common_SRCS} unix.c)
else(CONFIG_WIN32)
  set(qemacs_common_SRCS ${qemacs_common_SRCS} tty.c)
  if (NOT CONFIG_QT AND NOT CONFIG_GTK)
     set(qemacs_common_SRCS ${qemacs_common_SRCS} unix.c)
  endif()
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
  if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DQT_NO_DEBUG)
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
  endif(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  set(qemacs_SRCS ${qemacs_SRCS} qt.cpp)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif(CONFIG_QT)

if(CONFIG_X11)
  set(qemacs_SRCS ${qemacs_SRCS} x11.c)
endif(CONFIG_X11)

if(CONFIG_GTK)
  set(qemacs_SRCS ${qemacs_SRCS} gtk.c)
endif(CONFIG_GTK)

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
#  target_link_libraries(qe Qt5::Widgets)
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

if(CONFIG_GTK)
  target_link_libraries(qe ${GTK3_LIBRARIES})
  include_directories(${GTK3_INCLUDE_DIRS})
endif(CONFIG_GTK)

if(CONFIG_HTML)
  target_link_libraries(qe qhtml)
endif(CONFIG_HTML)

if(CONFIG_TINY)
add_executable(tqe ${qemacs_tiny_SRCS} unix.c)
add_dependencies(tqe basemodules)
set_property(TARGET tqe APPEND PROPERTY COMPILE_DEFINITIONS CONFIG_TINY=1)
endif(CONFIG_TINY)
