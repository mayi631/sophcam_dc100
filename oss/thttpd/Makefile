CUR_DIR      := $(shell pwd)
TARGET_DIR    := $(CUR_DIR)/../..
prefix = /usr/local
exec_prefix = ${prefix}
BINDIR = ${exec_prefix}/sbin
MANDIR = ${prefix}/man
WEBDIR = $(prefix)/www
WEBGROUP = www
CGIBINDIR = $(WEBDIR)/cgi-bin

CC = $(CROSS_COMPILE_3RD)gcc
CCOPT = -O2 -g
DEFS = -DHAVE__PROGNAME=1 -DHAVE_FCNTL_H=1 -DHAVE_GRP_H=1 -DHAVE_MEMORY_H=1 -DHAVE_PATHS_H=1 -DHAVE_POLL_H=1 -DHAVE_SYS_POLL_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_DIRENT_H=1 -DHAVE_LIBCRYPT=1 -DHAVE_STRERROR=1 -DHAVE_WAITPID=1 -DHAVE_VSNPRINTF=1 -DHAVE_DAEMON=1 -DHAVE_SETSID=1 -DHAVE_GETADDRINFO=1 -DHAVE_GETNAMEINFO=1 -DHAVE_GAI_STRERROR=1 -DHAVE_SIGSET=1 -DHAVE_ATOLL=1 -DHAVE_UNISTD_H=1 -DHAVE_GETPAGESIZE=1 -DHAVE_MMAP=1 -DHAVE_SELECT=1 -DHAVE_POLL=1 -DHAVE_TM_GMTOFF=1 -DHAVE_INT64T=1 -DHAVE_SOCKLENT=1
INCLS = -I.
CFLAGS += $(CCOPT) $(DEFS) $(INCLS) -Wno-error=unused-but-set-variable -fPIC
LDFLAGS =
LIBS = -lcrypt
NETLIBS =
LIB_DIR := $(CUR_DIR)
BUILD_LIB_DIR := $(TARGET_DIR)/build/lib

$(shell mkdir -p $(BUILD_LIB_DIR))

$(BUILD_LIB_DIR)/%.o: $(TARGET_DIR)/%.c
	@echo "Compiling $< to $@"
	@rm -f $@
	$(CC) $(CFLAGS) -c $< -o $@

SRC = $(TARGET_DIR)/thttpd.c $(TARGET_DIR)/libhttpd.c $(TARGET_DIR)/fdwatch.c $(TARGET_DIR)/mmc.c $(TARGET_DIR)/timers.c $(TARGET_DIR)/match.c $(TARGET_DIR)/tdate_parse.c
OBJ = $(patsubst $(TARGET_DIR)/%.c,$(BUILD_LIB_DIR)/%.o,$(SRC))
TARGET_A = $(BUILD_LIB_DIR)/libthttpd.a
TARGET_SO = $(BUILD_LIB_DIR)/libthttpd.so

ALL = $(TARGET_A) $(TARGET_SO)
GENHDR = mime_encodings.h mime_types.h
CLEANFILES = $(ALL) $(OBJ) $(GENHDR)
SUBDIRS = cgi-src extras
LDFLAGS_SO = -shared

all: $(ALL)

$(TARGET_A): $(OBJ)
	@echo "Creating static library $@ from $(OBJ)"
	@rm -f $@
	$(AR) -rv $@ $^

$(TARGET_SO): $(OBJ)
	@echo "Creating shared library $@ from $(OBJ)"
	$(CC) $(LDFLAGS_SO) $(LIBS) -o $@ $^

mime_encodings.h: mime_encodings.txt
	rm -f mime_encodings.h
	sed < mime_encodings.txt > mime_encodings.h \
	  -e 's/#.*//' -e 's/[ 	]*$//' -e '/^$/d' \
	  -e 's/[ 	][ 	]*/", 0, "/' -e 's/^/{ "/' -e 's/$/", 0 },/'

mime_types.h: mime_types.txt
	rm -f mime_types.h
	sed < mime_types.txt > mime_types.h \
	  -e 's/#.*//' -e 's/[ 	]*$//' -e '/^$/d' \
	  -e 's/[ 	][ 	]*/", 0, "/' -e 's/^/{ "/' -e 's/$/", 0 },/'

subdirs:
	for i in $(SUBDIRS); do ( \
	    cd $$i; \
	    pwd; \
	    $(MAKE) $(MFLAGS) \
		WEBDIR=$(WEBDIR) \
		CGIBINDIR=$(CGIBINDIR) \
		MANDIR=$(MANDIR) \
		WEBGROUP=$(WEBGROUP); \
	) done

clean:
	rm -f $(OBJ) $(TARGET_A) $(TARGET_SO)

install:
	@echo "Installing header files..."
	cp $(TARGET_DIR)/*.h $(TARGET_DIR)/include/
	@echo "Installing lib files..."
	cp $(LIB_DIR)/*.a $(TARGET_DIR)/lib/
	cp $(LIB_DIR)/*.so $(TARGET_DIR)/lib/

thttpd.o: config.h version.h libhttpd.h fdwatch.h mmc.h timers.h match.h
libhttpd.o: config.h version.h libhttpd.h mime_encodings.h mime_types.h mmc.h timers.h match.h tdate_parse.h
fdwatch.o: fdwatch.h
mmc.o: mmc.h libhttpd.h
timers.o: timers.h
match.o: match.h
tdate_parse.o: tdate_parse.h
