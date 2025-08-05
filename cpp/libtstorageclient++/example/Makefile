SRCPATH = src/
TESTPATH = tests/
OBJPATH = obj/

BINPATH = bin/
BINNAME = tstorageclient-upload-csv
TESTNAME = tests

MAINFILE = Main.cpp
SRCFILES = \
	CsvRecord.cpp \
	CsvToRecordParser.cpp \
	File.cpp \
	CommandLineOptions.cpp \
	Log.cpp \
	Utils.cpp \

TESTFILES = \
	CsvRecord.cpp \
	CsvToRecordParser.cpp \
	File.cpp \
	Utils.cpp \

LIBPATH = ../lib/
LIBNAME = tstorageclient++
LIBFULLPATH = $(LIBPATH)lib$(LIBNAME).so

CC = g++
STDCPP = -std=c++14

CFLAGS = $(STDCPP) -MD -MP -Wall -Werror $(DEFINES) -pedantic
LDFLAGS = -Wl,-z,origin '-Wl,-rpath,$$ORIGIN/../'$(LIBPATH)
DBGFLAGS = -O0 -g

OBJS = $(addprefix $(OBJPATH), $(subst .cpp,.o,$(SRCFILES)))
MAINOBJ = $(addprefix $(OBJPATH), $(subst .cpp,.o,$(MAINFILE)))
TESTOBJS = $(addprefix $(OBJPATH)$(TESTPATH), $(subst .cpp,.o,$(TESTFILES)))

INCLUDES = -I../include
LPATH = -L$(LIBPATH)
LIBS = $(addprefix -l, $(LIBNAME))


release: DBGFLAGS:=-O2 -flto
release: all

debug: DBGFLAGS:=-O0 -g
debug: all

all: main tests


main: $(BINPATH)$(BINNAME)

$(BINPATH)$(BINNAME) : $(OBJS) $(MAINOBJ) | $(BINPATH)
	$(CC) $(CFLAGS) $(DBGFLAGS) $(LDFLAGS) $(LPATH) $^ -o $@ $(LIBS)


tests: LIBS+=-lCatch2Main -lCatch2
tests: $(BINPATH)$(TESTNAME)

$(BINPATH)$(TESTNAME) : $(OBJS) $(TESTOBJS) $(LIBFULLPATH) | $(BINPATH)
	$(CC) $(CFLAGS) $(DBGFLAGS) $(LDFLAGS) $(LPATH) $^ -o $@ $(LIBS)


$(OBJPATH)%.o: $(SRCPATH)%.cpp | $(OBJPATH) $(OBJPATH)$(TESTPATH)
	$(CC) $(CFLAGS) $(DBGFLAGS) $(INCLUDES) -c $< -o $@

$(OBJPATH):
	mkdir -p $(OBJPATH)
$(OBJPATH)$(TESTPATH):
	mkdir -p $(OBJPATH)$(TESTPATH)
$(BINPATH):
	mkdir -p $(BINPATH)

clean:
	$(RM) $(BINPATH)$(BINNAME)
	$(RM) $(BINPATH)$(TESTNAME)
	$(RM) -r $(OBJPATH)

.PHONY: all clean debug release main tests

-include $(OBJS:.o=.d)
