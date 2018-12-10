AM_CXXFLAGS = 
EXTRA_LIBRARIES =

BTCDEMO_CONFIG_INCLUDES = -I$(builddir)/config
BTCDEMO_INCLUDES_PATH = $(srcdir)/include
BTCDEMO_INCLUDES = -I$(BTCDEMO_INCLUDES_PATH)

LIBBTCDEMO_UTIL = libbtcdemo_util.a

EXTRA_LIBRARIES += \
				   $(LIBBTCDEMO_UTIL)

bin_PROGRAMS = btcdemod

BTCDEMO_HEADER_FILES = \
					   $(BTCDEMO_INCLUDES_PATH)/init.h \
					   $(BTCDEMO_INCLUDES_PATH)/clientversion.h \
					   $(BTCDEMO_INCLUDES_PATH)/serialize.h \
					   $(BTCDEMO_INCLUDES_PATH)/sync.h \
					   $(BTCDEMO_INCLUDES_PATH)/tinyformat.h \
					   $(BTCDEMO_INCLUDES_PATH)/util.h \
					   $(BTCDEMO_INCLUDES_PATH)/utilstrencodings.h \
					   $(BTCDEMO_INCLUDES_PATH)/utiltime.h

libbtcdemo_util_a_CPPFLAGS = $(BTCDEMO_INCLUDES)
libbtcdemo_util_a_SOURCES = sync.cpp \
							util.cpp \
							$(BTCDEMO_HEADER_FILES)



btcdemod_CPPFLAGS = $(BTCDEMO_INCLUDES)
btcdemod_SOURCES = btcdemod.cpp \
				   init.cpp \
				   validation.cpp \
				   $(BTCDEMO_HEADER_FILES)
btcdemod_LDADD = \
				 $(LIBBTCDEMO_UTIL)
btcdemod_LDADD += $(BOOST_LIBS)
