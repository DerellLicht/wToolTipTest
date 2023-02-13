SHELL=cmd.exe
USE_DEBUG = NO
USE_STATIC = NO
USE_WINMSGS = YES

#BASE_PATH=c:/mingw.tdm461/bin/
#BASE_PATH=c:/mingw.v4.8.1/bin/
BASE_PATH=c:/mingw/bin/

ifeq ($(USE_DEBUG),YES)
CFLAGS=-Wall -O -ggdb -mwindows 
LFLAGS=
else
CFLAGS=-Wall -O2 -mwindows 
LFLAGS=-s
endif
CFLAGS += -Wno-write-strings
CFLAGS += -Weffc++
LiFLAGS += -DWINVER=0x0500
LiFLAGS += -D_WIN32_IE=0x0501
#LiFLAGS += _WIN32_WINNT=0x0501
ifeq ($(USE_STATIC),YES)
LIBS=-lgdi32 -lcomctl32 -static
else
LIBS=-lgdi32 -lcomctl32
endif

CSRC+=wToolTipTest.cpp tooltips.cpp common_funcs.cpp

ifeq ($(USE_WINMSGS),YES)
CFLAGS += -DUSE_WINMSGS
endif

OBJS = $(CSRC:.cpp=.o) rc.o

BIN=wToolTipTest

#************************************************************
%.o: %.cpp
	$(BASE_PATH)g++ $(CFLAGS) -c $< -o $@

all: $(BIN).exe

clean:
	rm -f $(BIN).exe $(OBJS) *.zip *.bak *~

dist:
	rm -f $(BIN).zip
	zip -r $(BIN).zip $(BIN).exe *.f* fntcol\* readme.txt

wc:
	wc -l *.cpp *.rc

source:
	rm -f $(BIN).src.zip
	zip $(BIN).src.zip *

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) lintdefs.cpp *.rc $(CSRC)"

depend:
	makedepend $(CFLAGS) $(CSRC)

#************************************************************
$(BIN).exe: $(OBJS)
	$(BASE_PATH)g++ $(CFLAGS) $(LFLAGS) $(OBJS) -o $@ $(LIBS)

rc.o: $(BIN).rc 
	windres $< -O COFF -o $@

# DO NOT DELETE

wToolTipTest.o: resource.h common.h
tooltips.o: resource.h common.h
common_funcs.o: common.h
