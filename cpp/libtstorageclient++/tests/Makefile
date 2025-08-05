SRCPATH = src/
OBJPATH = obj/

BINPATH = bin/
BINNAME = test
SRVNAME = pystorage

SRCFILES = \
	Test.cpp \
	Asserts.cpp \
	FloatPayload.cpp \
	StringPayload.cpp \
	BufferTests.cpp \
	ChannelTests.cpp \
	SerializerTests.cpp \
	SocketTests.cpp \

LIBPATH = ../lib/
LIBNAME = tstorageclient++
LIBFULLPATH = $(LIBPATH)lib$(LIBNAME).so

CC = g++
STDCPP = -std=c++14

CFLAGS = $(STDCPP) -MD -MP -Wall -Werror $(DEFINES) -pedantic
DBGFLAGS = -O0 -g
LDFLAGS = -Wl,-z,origin '-Wl,-rpath,$$ORIGIN/../'$(LIBPATH)

SRCS = $(addprefix $(SRCPATH), $(SRCFILES))
OBJS = $(addprefix $(OBJPATH), $(subst .cpp,.o,$(SRCFILES)))
INCLUDES = -I../include -iquote../src
LIBRARIES = -L$(LIBPATH)
LIBS = -l$(LIBNAME)

release: DBGFLAGS:=-O2 -flto
release: all

debug: DBGFLAGS:=-O0 -g
debug: all

all: $(BINPATH)$(BINNAME) $(BINPATH)$(SRVNAME)

$(BINPATH)$(BINNAME) : $(OBJS) $(LIBFULLPATH) | $(BINPATH)
	$(CC) $(CFLAGS) $(DBGFLAGS) $(LDFLAGS) $(LIBRARIES) $(filter-out $(LIBFULLPATH),$^) -o $@ $(LIBS)

$(BINPATH)$(SRVNAME) : ./pystorage.py | $(BINPATH)
	ln -fs ../pystorage.py $@

$(OBJPATH)%.o: $(SRCPATH)%.cpp | $(OBJPATH)
	$(CC) $(CFLAGS) $(DBGFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) -r $(BINPATH)
	$(RM) -r $(OBJPATH)

$(BINPATH):
	mkdir $(BINPATH)
$(OBJPATH):
	mkdir $(OBJPATH)

rm:
	$(RM) $(OBJPATH)Test.o

.PHONY: all clean debug release

-include $(OBJS:.o=.d)
