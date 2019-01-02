AM_CXXFLAGS = $(PTHREAD_CFLAGS) 
EXTRA_LIBRARIES =

BTCLITE_CONFIG_INCLUDES = -I$(builddir)/config
BTCLITE_INCLUDES_PATH = $(srcdir)/include
BTCLITE_INCLUDES = -I$(BTCLITE_INCLUDES_PATH)

STDCPP_FILESYSTEM_LIBS = -lstdc++fs
LIBBTCLITE_UTIL = libbtclite_util.a

EXTRA_LIBRARIES += \
				   $(LIBBTCLITE_UTIL)

bin_PROGRAMS = btclited

BTCLITE_HEADER_FILES = \
					   $(BTCLITE_INCLUDES_PATH)/init.h \
					   $(BTCLITE_INCLUDES_PATH)/clientversion.h \
					   $(BTCLITE_INCLUDES_PATH)/net.h \
					   $(BTCLITE_INCLUDES_PATH)/serialize.h \
					   $(BTCLITE_INCLUDES_PATH)/sync.h \
					   $(BTCLITE_INCLUDES_PATH)/tinyformat.h \
					   $(BTCLITE_INCLUDES_PATH)/util.h \
					   $(BTCLITE_INCLUDES_PATH)/utilstrencodings.h \
					   $(BTCLITE_INCLUDES_PATH)/utiltime.h

libbtclite_util_a_CPPFLAGS = $(AM_CXXFLAGS) $(BTCLITE_INCLUDES)
libbtclite_util_a_SOURCES = sync.cpp \
							util.cpp \
							utiltime.cpp \
							$(BTCLITE_HEADER_FILES)



btclited_CPPFLAGS = $(AM_CXXFLAGS) $(BTCLITE_INCLUDES)
btclited_SOURCES = btclited.cpp \
				   init.cpp \
				   validation.cpp \
				   $(BTCLITE_HEADER_FILES)
btclited_LDADD = \
				 $(LIBBTCLITE_UTIL)
btclited_LDADD += $(PTHREAD_LIBS) \
				  $(STDCPP_FILESYSTEM_LIBS)
