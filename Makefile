# QEmacs, tiny but powerful multimode editor
#
# Copyright (c) 2000-2002 Fabrice Bellard.
# Copyright (c) 2000-2014 Charlie Gordon.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

DEPTH=.

include $(DEPTH)/config.mak

ifeq ($(CC),gcc)
  CFLAGS   += -Wall -g -O2 -funsigned-char
  # do not warn about zero-length formats.
  CFLAGS   += -Wno-format-zero-length
  LDFLAGS  := -g
endif

#include local compiler configuration file
-include $(DEPTH)/cflags.mk

ifdef TARGET_GPROF
  CFLAGS  += -p
  LDFLAGS += -p
endif

TLDFLAGS := $(LDFLAGS)

ifdef TARGET_ARCH_X86
  #CFLAGS+=-fomit-frame-pointer
  ifeq ($(GCC_MAJOR),2)
    CFLAGS+=-m386 -malign-functions=0
  else
    CFLAGS+=-march=i386 -falign-functions=0
  endif
endif

DEFINES=-DHAVE_QE_CONFIG_H

########################################################
# do not modify after this

TARGETLIBS:=
TARGETS+= qe$(EXE) tqe$(EXE) kmaps ligatures

OBJS:= qe.o parser.o charset.o buffer.o input.o display.o util.o hex.o \
       list.o cutils.o
TOBJS:= $(OBJS)

OBJS+= extras.o variables.o

ifdef CONFIG_PNG_OUTPUT
  HTMLTOPPM_LIBS+= -lpng
endif

ifdef CONFIG_DLL
  LIBS+=$(DLLIBS)
  # export some qemacs symbols
  LDFLAGS+=-Wl,-E
endif

ifdef CONFIG_DOC
  TARGETS+= qe-doc.html
endif

ifdef CONFIG_HAIKU
  OBJS+= haiku.o
  LIBS+= -lbe
endif

ifdef CONFIG_WIN32
  OBJS+= unix.o win32.o
  TOBJS+= unix.o win32.o
#  OBJS+= printf.o
#  TOBJS+= printf.o
  LIBS+= -lmsvcrt -lgdi32 -lwsock32
  TLIBS+= -lmsvcrt -lgdi32 -lwsock32
else
  OBJS+= unix.o tty.o
  TOBJS+= unix.o tty.o
  LIBS+= $(EXTRALIBS)
endif

ifdef CONFIG_QSCRIPT
  OBJS+= qscript.o eval.o
endif

OBJS+= lua-plugins.o
TOBJS+= lua-plugins.o
LIBS+= -llua
TLIBS+= -llua

ifdef CONFIG_ALL_KMAPS
  OBJS+= kmap.o
endif

ifdef CONFIG_UNICODE_JOIN
  OBJS+= unicode_join.o arabic.o indic.o qfribidi.o
endif

# more charsets if needed
OBJS+= charsetjis.o charsetmore.o

ifdef CONFIG_ALL_MODES
  OBJS+= unihex.o bufed.o clang.o xml.o htmlsrc.o \
         lisp.o makemode.o markdown.o orgmode.o perl.o script.o extra-modes.o
  ifndef CONFIG_WIN32
    OBJS+= shell.o dired.o latex-mode.o archive.o
  endif
endif

# currently not used in qemacs
ifdef CONFIG_CFB
  OBJS+= libfbf.o fbfrender.o cfb.o fbffonts.o
endif

ifdef CONFIG_X11
  OBJS+= x11.o
  ifdef CONFIG_XRENDER
    LIBS+= -lXrender
  endif
  ifdef CONFIG_XV
    LIBS+= -lXv
  endif
  LIBS+= -L/usr/X11R6/lib -lXext -lX11
endif

ifdef CONFIG_HTML
  CFLAGS+= -I./libqhtml
  DEP_LIBS+= libqhtml/libqhtml.a
  LIBS+= -L./libqhtml -lqhtml
  OBJS+= html.o docbook.o
  ifndef CONFIG_WIN32
    TARGETLIBS+= libqhtml
    TARGETS+= html2png$(EXE)
  endif
endif

ifdef CONFIG_FFMPEG
  OBJS+= video.o image.o
  DEP_LIBS+= $(FFMPEG_LIBDIR)/libavcodec/libavcodec.a $(FFMPEG_LIBDIR)/libavformat/libavformat.a
  LIBS+= -L$(FFMPEG_LIBDIR)/libavcodec -L$(FFMPEG_LIBDIR)/libavformat -lavformat -lavcodec -lz -lpthread
  DEFINES+= -I$(FFMPEG_SRCDIR)/libavcodec -I$(FFMPEG_SRCDIR)/libavformat
  TARGETS+= ffplay$(EXE)
endif

ifdef CONFIG_INIT_CALLS
  # must be the last object
  OBJS+= qeend.o
  TOBJS+= qeend.o
endif

SRCS:= $(OBJS:.o=.c)
TSRCS:= $(TOBJS:.o=.c)

DEPENDS:= qe.h config.h cutils.h display.h qestyles.h config.mak
DEPENDS:= $(addprefix $(DEPTH)/, $(DEPENDS))

OBJS_DIR:= $(DEPTH)/.objs
TOBJS_DIR:= $(DEPTH)/.tobjs
OBJS:= $(addprefix $(OBJS_DIR)/, $(OBJS))
TOBJS:= $(addprefix $(TOBJS_DIR)/, $(TOBJS))

$(shell mkdir -p $(OBJS_DIR) $(TOBJS_DIR))

#
# Dependencies
#
all: $(TARGETLIBS) $(TARGETS)

libqhtml: force
	$(MAKE) -C libqhtml all

qe_g$(EXE): $(OBJS) $(DEP_LIBS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

qe$(EXE): qe_g$(EXE) Makefile
	rm -f $@
	cp $< $@
	-$(STRIP) $@
	@ls -l $@
	echo `size $@` `wc -c $@` qe $(OPTIONS) \
		| cut -d ' ' -f 7-10,13,15-40 >> STATS

#
# Tiny version of QEmacs
#
tqe_g$(EXE): $(TOBJS)
	$(CC) $(TLDFLAGS) -o $@ $^ $(TLIBS)

tqe$(EXE): tqe_g$(EXE) Makefile
	rm -f $@
	cp $< $@
	-$(STRIP) $@
	@ls -l $@
	echo `size $@` `wc -c $@` tqe $(OPTIONS) \
		| cut -d ' ' -f 7-10,13,15-40 >> STATS

ffplay$(EXE): qe$(EXE) Makefile
	ln -sf $< $@

ifndef CONFIG_INIT_CALLS
$(OBJS_DIR)/qe.o: allmodules.txt
$(TOBJS_DIR)/qe.o: basemodules.txt
endif

allmodules.txt: $(SRCS) Makefile
	@echo creating $@
	@echo '/* This file was generated automatically */' > $@
	@grep -h ^qe_module_init $(SRCS)                    >> $@

basemodules.txt: $(TSRCS) Makefile
	@echo creating $@
	@echo '/* This file was generated automatically */' > $@
	@grep -h ^qe_module_init $(TSRCS)                   >> $@

$(OBJS_DIR)/cfb.o: cfb.c cfb.h fbfrender.h
$(OBJS_DIR)/charsetjis.o: charsetjis.c charsetjis.def
$(OBJS_DIR)/fbfrender.o: fbfrender.c fbfrender.h libfbf.h
$(OBJS_DIR)/qe.o: qe.c parser.c qeconfig.h qfribidi.h variables.h
$(OBJS_DIR)/qfribidi.o: qfribidi.c qfribidi.h

$(TOBJS_DIR)/cfb.o: cfb.c cfb.h fbfrender.h
$(TOBJS_DIR)/charsetjis.o: charsetjis.c charsetjis.def
$(TOBJS_DIR)/fbfrender.o: fbfrender.c fbfrender.h libfbf.h
$(TOBJS_DIR)/qe.o: qe.c parser.c qeconfig.h qfribidi.h variables.h
$(TOBJS_DIR)/qfribidi.o: qfribidi.c qfribidi.h

$(OBJS_DIR)/%.o: %.c $(DEPENDS) Makefile
	$(CC) $(DEFINES) $(CFLAGS) -o $@ -c $<

$(TOBJS_DIR)/%.o: %.c $(DEPENDS) Makefile
	$(CC) $(DEFINES) -DCONFIG_TINY $(CFLAGS) -o $@ -c $<

$(OBJS_DIR)/haiku.o: haiku.cpp $(DEPENDS) Makefile
	g++ $(DEFINES) $(CFLAGS) -Wno-multichar -o $@ -c $<

$(TOBJS_DIR)/haiku.o: haiku.cpp $(DEPENDS) Makefile
	g++ $(DEFINES) -DCONFIG_TINY $(CFLAGS) -Wno-multichar -o $@ -c $<

%.s: %.c $(DEPENDS) Makefile
	$(CC) $(DEFINES) $(CFLAGS) -o $@ -S $<

%.s: %.cpp $(DEPENDS) Makefile
	g++ $(DEFINES) $(CFLAGS) -Wno-multichar -o $@ -S $<

#
# Test for bidir algorithm
#
qfribidi$(EXE): qfribidi.c cutils.c
	$(HOST_CC) $(CFLAGS) -DTEST -o $@ $^

#
# build ligature table
#
ligtoqe$(EXE): ligtoqe.c cutils.c
	$(HOST_CC) $(CFLAGS) -o $@ $^

ifdef BUILD_ALL
ligatures: ligtoqe$(EXE) unifont.lig
	./ligtoqe unifont.lig $@
endif

#
# Key maps build (Only useful if you want to build your own maps from yudit maps)
#
KMAPS=Arabic.kmap ArmenianEast.kmap ArmenianWest.kmap Chinese-CJ.kmap       \
      Cyrillic.kmap Czech.kmap DE-RU.kmap Danish.kmap Dutch.kmap            \
      Esperanto.kmap Ethiopic.kmap French.kmap Georgian.kmap German.kmap    \
      Greek.kmap GreekMono.kmap Guarani.kmap HebrewP.kmap Hungarian.kmap    \
      KOI8_R.kmap Lithuanian.kmap Mnemonic.kmap Polish.kmap Russian.kmap    \
      SGML.kmap TeX.kmap Troff.kmap VNtelex.kmap                            \
      Vietnamese.kmap XKB_iso8859-4.kmap                                    \
      DanishAlternate.kmap GreekBible.kmap Polytonic.kmap Spanish.kmap      \
      Thai.kmap VietnameseTelex.kmap Welsh.kmap                             \
      Hebrew.kmap HebrewIsraeli.kmap HebrewP.kmap Israeli.kmap Yiddish.kmap \
      Kana.kmap
#     Hangul.kmap Hangul2.kmap Hangul3.kmap Unicode2.kmap
#KMAPS_DIR=$(datadir)/yudit/data
KMAPS_DIR=kmap
KMAPS:=$(addprefix $(KMAPS_DIR)/, $(KMAPS))

kmaptoqe$(EXE): kmaptoqe.c cutils.c
	$(HOST_CC) $(CFLAGS) -o $@ $^

ifdef BUILD_ALL
kmaps: kmaptoqe$(EXE) $(KMAPS) Makefile
	./kmaptoqe $@ $(KMAPS)
endif

#
# Code pages (only useful to add your own code pages)
#
CP=  8859-2.TXT   8859-3.TXT   8859-4.TXT   8859-5.TXT   8859-6.TXT  \
     8859-7.TXT   8859-8.TXT   8859-9.TXT   8859-10.TXT  8859-11.TXT \
     8859-13.TXT  8859-14.TXT  8859-15.TXT  8859-16.TXT              \
     CP437.TXT    CP737.TXT    CP850.TXT    CP852.TXT    CP866.TXT   \
     CP1125.TXT   CP1250.TXT   CP1251.TXT   CP1252.TXT   CP1256.TXT  \
     CP1257.TXT   MAC-LATIN2.TXT MAC-ROMAN.TXT                       \
     kamen.cp     KOI8-R.TXT   koi8_u.cp    TCVN.TXT     VISCII.TXT  \
     CP037.TXT    CP424.TXT    CP500.TXT    CP875.TXT    CP1026.TXT  \
     ATARIST.TXT

CP:=$(addprefix cp/,$(CP))

JIS= JIS0208.TXT JIS0212.TXT
JIS:=$(addprefix cp/,$(JIS))

cptoqe$(EXE): cptoqe.c cutils.c
	$(HOST_CC) $(CFLAGS) -o $@ $^

jistoqe$(EXE): jistoqe.c cutils.c
	$(HOST_CC) $(CFLAGS) -o $@ $^

ifdef BUILD_ALL
charsetmore.c: cp/cpdata.txt $(CP) cptoqe$(EXE) Makefile
	./cptoqe -i cp/cpdata.txt $(CP) > $@

charsetjis.def: $(JIS) jistoqe$(EXE) Makefile
	./jistoqe $(JIS) > $@
endif

#
# fonts (only needed for html2png)
#
FONTS=fixed10.fbf fixed12.fbf fixed13.fbf fixed14.fbf \
      helv8.fbf helv10.fbf helv12.fbf helv14.fbf helv18.fbf helv24.fbf \
      times8.fbf times10.fbf times12.fbf times14.fbf times18.fbf times24.fbf \
      unifont.fbf
FONTS:=$(addprefix fonts/,$(FONTS))

fbftoqe$(EXE): fbftoqe.c cutils.c
	$(HOST_CC) $(CFLAGS) -o $@ $^

fbffonts.c: fbftoqe$(EXE) $(FONTS)
	./fbftoqe $(FONTS) > $@

#
# html2png tool (XML/HTML/CSS2 renderer test tool)
#
OBJS1=html2png.o util.o cutils.o \
      arabic.o indic.o qfribidi.o display.o unicode_join.o \
      charset.o charsetmore.o charsetjis.o \
      libfbf.o fbfrender.o cfb.o fbffonts.o

OBJS1:=$(addprefix $(OBJS_DIR)/, $(OBJS1))

html2png$(EXE): $(OBJS1) libqhtml/libqhtml.a
	$(CC) $(LDFLAGS) -o $@ $(OBJS1) \
                   -L./libqhtml -lqhtml $(HTMLTOPPM_LIBS)

# autotest target
test:
	$(MAKE) -C tests test

# documentation
qe-doc.html: qe-doc.texi Makefile
	LANGUAGE=en_US LC_ALL=en_US.UTF-8 texi2html -monolithic $<
	mv $@ $@.tmp
	sed "s/This document was generated on.*//"     < $@.tmp | \
	sed "s/<!-- Created on .* by/<!-- Created by/" > $@
	rm $@.tmp

#
# Maintenance targets
#
clean:
	$(MAKE) -C libqhtml clean
	rm -rf *.dSYM
	rm -f *~ *.o *.a *.exe *_g TAGS gmon.out core *.exe.stackdump   \
           qe tqe qfribidi kmaptoqe ligtoqe html2png fbftoqe fbffonts.c \
           cptoqe jistoqe allmodules.txt basemodules.txt '.#'*[0-9] \
	   $(OBJS_DIR)/*.o

distclean: clean
	rm -rf config.h config.mak $(OBJS_DIR) $(TOBJS_DIR)

install: $(TARGETS) qe.1
	$(INSTALL) -m 755 -d $(DESTDIR)$(prefix)/bin
	$(INSTALL) -m 755 -d $(DESTDIR)$(mandir)/man1
	$(INSTALL) -m 755 -d $(DESTDIR)$(datadir)/qe
	$(INSTALL) -m 755 -s qe$(EXE) $(DESTDIR)$(prefix)/bin/qemacs$(EXE)
	ln -sf qemacs $(DESTDIR)$(prefix)/bin/qe$(EXE)
ifdef CONFIG_FFMPEG
	ln -sf qemacs$(EXE) $(DESTDIR)$(prefix)/bin/ffplay$(EXE)
endif
	$(INSTALL) -m 644 kmaps ligatures $(DESTDIR)$(datadir)/qe
	$(INSTALL) -m 644 qe.1 $(DESTDIR)$(mandir)/man1
ifdef CONFIG_HTML
	$(INSTALL) -m 755 -s html2png$(EXE) $(DESTDIR)$(prefix)/bin
endif

uninstall:
	rm -f $(DESTDIR)$(prefix)/bin/qemacs$(EXE)   \
	      $(DESTDIR)$(prefix)/bin/qe$(EXE)       \
	      $(DESTDIR)$(prefix)/bin/ffplay$(EXE)   \
	      $(DESTDIR)$(mandir)/man1/qe.1          \
	      $(DESTDIR)$(datadir)/qe/kmaps          \
	      $(DESTDIR)$(datadir)/qe/ligatures      \
	      $(DESTDIR)$(prefix)/bin/html2png$(EXE)

rebuild:
	./configure && $(MAKE) clean all

TAGS: force
	etags *.[ch]

force:

#
# tar archive for distribution
#
FILES:=COPYING Changelog Makefile README TODO VERSION               \
       arabic.c bufed.c buffer.c cfb.c cfb.h charset.c charsetjis.c \
       charsetjis.def charsetmore.c clang.c config.eg config.h      \
       configure cptoqe.c cutils.c cutils.h dired.c display.c       \
       display.h docbook.c extras.c fbffonts.c fbfrender.c          \
       fbfrender.h fbftoqe.c haiku.cpp haiku-pe2qe.sh hex.c html.c  \
       html2png.c htmlsrc.c lisp.c                                  \
       image.c indic.c input.c jistoqe.c kmap.c kmaptoqe.c          \
       latex-mode.c libfbf.c libfbf.h ligtoqe.c list.c makemode.c   \
       mpeg.c perl.c qe-doc.html qe-doc.texi qe.1 qe.c qe.h qe.tcc  \
       qeconfig.h qeend.c qemacs.spec qestyles.h qfribidi.c         \
       qfribidi.h script.c shell.c tty.c unicode_join.c unihex.c    \
       unix.c util.c variables.c variables.h video.c win32.c x11.c  \
       xml.c

FILES+=plugins/Makefile  plugins/my_plugin.c

TESTS= HELLO.txt TestPage.txt colours.txt lattrs.txt scocols.txt    \
       test-capital-rtl test-capital-rtl.ref test-hebrew testbidi.html \
       utf8.txt vt100.txt
TESTS:=$(addprefix tests/,$(TESTS))

# qhtml library
FILES+=libqhtml/Makefile libqhtml/css.c libqhtml/css.h libqhtml/cssid.h \
       libqhtml/cssparse.c libqhtml/csstoqe.c libqhtml/docbook.css      \
       libqhtml/html.css libqhtml/htmlent.h libqhtml/xmlparse.c

# keyboard maps, code pages, fonts
FILES+=unifont.lig ligatures kmaps $(KMAPS) $(CP) $(JIS) $(FONTS) $(TESTS)

FILE=qemacs-$(shell cat VERSION)

tar: $(FILES)
	rm -f $(HOME)/$(FILE).tar.gz
	rm -rf /tmp/$(FILE)
	mkdir -p /tmp/$(FILE)
	tar cf - $(FILES) | (cd /tmp/$(FILE) ; tar xf - )
	( cd /tmp ; tar cfz $(HOME)/$(FILE).tar.gz $(FILE) )
	rm -rf /tmp/$(FILE)

SPLINTOPTS := +posixlib -nestcomment +boolint +charintliteral -mayaliasunique
SPLINTOPTS += -nullstate -unqualifiedtrans +charint
# extra options that will be removed later
SPLINTOPTS += -mustfreeonly -temptrans -kepttrans

splint: allmodules.txt basemodules.txt
	splint $(SPLINTOPTS) -I. -Ilibqhtml $(SRCS)
