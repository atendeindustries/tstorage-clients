SRCPATH = src/
OBJPATH = obj/
LIBPATH = lib/
INCLUDEPATH = include/tstorageclient++/

VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_PATCH = 0
VERSION = $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

BINARYNAME = libtstorageclient++

STATICLIB = $(BINARYNAME).a.$(VERSION)
STATICLINK_MINOR = $(basename $(STATICLIB))
STATICLINK_MAJOR = $(basename $(STATICLINK_MINOR))
STATICLINK = $(basename $(STATICLINK_MAJOR))

SHAREDLIB = $(BINARYNAME).so.$(VERSION)
SHAREDLINK_MINOR = $(basename $(SHAREDLIB))
SHAREDLINK_MAJOR = $(basename $(SHAREDLINK_MINOR))
SHAREDLINK = $(basename $(SHAREDLINK_MAJOR))

BINARY = $(LIBPATH)$(SHAREDLIB)
BINARYEXT = so
ifdef STATIC
	BINARY := $(LIBPATH)$(STATICLIB)
	BINARYEXT = a
endif
BINARYFILE = $(BINARYNAME).$(BINARYEXT)

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif


SRCFILES = \
	BatchSerializer.cpp \
	Buffer.cpp \
	ChannelBase.cpp \
	ChannelImpl.cpp \
	DataTypes.cpp \
	Serializer.cpp \
	Socket.cpp \
	Timestamp.cpp \
	Version.cpp

CC = g++
STDCPP = -std=c++14

DEFINES = \
-DD_LIBTSCLIENT_VERSION=$(VERSION)\
-DD_LIBTSCLIENT_VERSION_MAJOR=$(VERSION_MAJOR)\
-DD_LIBTSCLIENT_VERSION_MINOR=$(VERSION_MINOR)\
-DD_LIBTSCLIENT_VERSION_PATCH=$(VERSION_PATCH)\

CFLAGS = $(STDCPP) -fPIC -MD -MP $(DEFINES) \
	-Wall -Werror -pedantic
DBGFLAGS = -O0 -g
LDFLAGS = -Wl,-soname,$(BINARYFILE).$(VERSION_MAJOR)

SRCS = $(addprefix $(SRCPATH), $(SRCFILES))
OBJS = $(addprefix $(OBJPATH), $(subst .cpp,.o, $(SRCFILES)))
INCLUDES = -Iinclude

release: DBGFLAGS:=-O2 -flto
release: CFLAGS+=-fvisibility=hidden
release: $(BINARY)

debug: DBGFLAGS:=-O0 -g
debug: DEFINES+=-DDEBUG
debug: $(BINARY)

$(LIBPATH)$(STATICLIB): $(OBJS) | $(LIBPATH)
	gcc-ar -crs $@ $(OBJS)
	ln -sf $(STATICLIB) $(LIBPATH)$(STATICLINK_MAJOR)
	ln -sf $(STATICLINK_MAJOR) $(LIBPATH)$(STATICLINK)

$(LIBPATH)$(SHAREDLIB): $(OBJS) | $(LIBPATH)
	g++ -shared $(DBGFLAGS) $(LDFLAGS) -o $@ $(OBJS)
	ln -sf $(SHAREDLIB) $(LIBPATH)$(SHAREDLINK_MAJOR)
	ln -sf $(SHAREDLINK_MAJOR) $(LIBPATH)$(SHAREDLINK)

$(OBJPATH)%.o: $(SRCPATH)%.cpp | $(OBJPATH)
	$(CC) $(CFLAGS) $(DBGFLAGS) $(INCLUDES) -c $< -o $@

$(LIBPATH):
	mkdir $(LIBPATH)
$(OBJPATH):
	mkdir $(OBJPATH)

clean:
	$(RM) -r $(LIBPATH)
	$(RM) -r $(OBJPATH)

install: $(BINARY)
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(BINARY) $(DESTDIR)$(PREFIX)/lib/
	ln -s $(BINARYFILE).$(VERSION) $(DESTDIR)$(PREFIX)/lib/$(BINARYFILE).$(VERSION_MAJOR)
	ln -s $(BINARYFILE).$(VERSION_MAJOR) $(DESTDIR)$(PREFIX)/lib/$(BINARYFILE)
	install -d $(DESTDIR)$(PREFIX)/include/tstorageclient++/
	install -m 644 $(INCLUDEPATH)* $(DESTDIR)$(PREFIX)/include/tstorageclient++/

.PHONY: clean debug release test install

-include $(OBJS:.o=.d)
