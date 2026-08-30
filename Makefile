USE_DEBUG = NO
USE_STATIC = NO
USE_WINMSGS = YES

#BASE_PATH=c:/mingw.tdm461/bin/
#BASE_PATH=c:/mingw.v4.8.1/bin/
BASE_PATH=d:/tdm32/bin/

ifeq ($(USE_DEBUG),YES)
CFLAGS=-Wall -O -ggdb -mwindows 
LFLAGS=
else
CFLAGS=-Wall -O2 -c
LFLAGS=-s -mwindows
endif
CFLAGS += -Weffc++
CFLAGS += -Wno-write-strings
CFLAGS += -Wno-stringop-truncation
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

BASE :=wToolTipTest
BINX :=$(BASE).exe

# Automatically parse the latest version block
VERSION := $(shell grep -oE '\[[0-9]+\.[0-9]+\]' CHANGELOG.md | head -n 1 | tr -d '[]')
DIST_ZIP := $(BASE)V$(VERSION).zip

.PHONY: dist release update
#************************************************************
%.o: %.cpp
	$(BASE_PATH)g++ $(CFLAGS) $< -o $@

all: $(BINX)

clean:
	rm -f $(BINX).exe $(OBJS) *.zip *.bak *~

dist:
	rm -f *.zip
	zip $(DIST_ZIP) $(BINX) README.md LICENSE.txt CHANGELOG.md

# Your new automated release workflow
release:
	cmd /C "@echo Preparing GitHub release for v$(VERSION)..."
	sed -n '/## \['$(VERSION)'\]/,/## \[/p' CHANGELOG.md | sed '$$d' > temp_notes.md
	gh release create v$(VERSION) ./$(DIST_ZIP) ./CHANGELOG.md --notes-file temp_notes.md
	rm temp_notes.md
	cmd /C "@echo Release v$(VERSION) successfully uploaded to GitHub!"
	
# Your new update-in-place pipeline
update: dist
	cmd /C "@echo Updating assets for existing release v$(VERSION)..."
	@# Uploads and overwrites the .zip file and CHANGELOG.md on GitHub
	gh release upload v$(VERSION) ./$(DIST_ZIP) ./CHANGELOG.md --clobber
	cmd /C "@echo Release v$(VERSION) assets successfully updated on GitHub!"

wc:
	wc -l *.cpp *.rc

check:
	cmd /C "d:\llvm\bin\clang-tidy.exe $(CSRC)"

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) lintdefs.cpp *.rc $(CSRC)"

depend:
	makedepend $(CFLAGS) $(CSRC)

#************************************************************
$(BINX).exe: $(OBJS)
	$(TOOLS)/$(GNAME) $(OBJS) $(LFLAGS) -o $(BINX) $(LIBS) 

rc.o: $(BASE).rc 
	$(TOOLS)\windres -O COFF $^ -o $@

# DO NOT DELETE

wToolTipTest.o: resource.h common.h tooltips.h
tooltips.o: common.h tooltips.h
common_funcs.o: common.h
