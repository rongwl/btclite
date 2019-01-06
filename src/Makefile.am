AM_CXXFLAGS = $(PTHREAD_CFLAGS) 
EXTRA_LIBRARIES =

BTCLITE_CONFIG_INCLUDES = -I$(builddir)/config
BTCLITE_INCLUDES_PATH = $(srcdir)/include
BTCLITE_INCLUDES = -I$(BTCLITE_INCLUDES_PATH)

STDCPP_FILESYSTEM_LIBS = -lstdc++fs
LIBBTCLITE_CONSENSUS = consensus/libbtclite_consensus.a
LIBBTCLITE_UTIL = utility/libbtclite_util.a

EXTRA_LIBRARIES += \
				   $(LIBBTCLITE_COMMON) \
				   $(LIBBTCLITE_CONSENSUS) \
				   $(LIBBTCLITE_UTIL)

bin_PROGRAMS = btclited

BTCLITE_HEADER_FILES = \
					   $(BTCLITE_INCLUDES_PATH)/block.h \
					   $(BTCLITE_INCLUDES_PATH)/chainparams.h \
					   $(BTCLITE_INCLUDES_PATH)/clientversion.h \
					   $(BTCLITE_INCLUDES_PATH)/init.h \
					   $(BTCLITE_INCLUDES_PATH)/net.h \
					   $(BTCLITE_INCLUDES_PATH)/serialize.h \
					   $(BTCLITE_INCLUDES_PATH)/sync.h \
					   $(BTCLITE_INCLUDES_PATH)/tinyformat.h \
					   $(BTCLITE_INCLUDES_PATH)/uint256.h \
					   $(BTCLITE_INCLUDES_PATH)/util.h \
					   $(BTCLITE_INCLUDES_PATH)/utilstrencodings.h \
					   $(BTCLITE_INCLUDES_PATH)/utiltime.h


consensus_libbtclite_consensus_a_CPPFLAGS = $(AM_CXXFLAGS) $(BTCLITE_INCLUDES)
consensus_libbtclite_consensus_a_SOURCES = consensus/uint256.cpp \
										   utility/utilstrencodings.cpp

utility_libbtclite_util_a_CPPFLAGS = $(AM_CXXFLAGS) $(BTCLITE_INCLUDES)
utility_libbtclite_util_a_SOURCES = utility/sync.cpp \
									utility/util.cpp \
									utility/utiltime.cpp \
									utility/utilstrencodings.cpp 


btclited_CPPFLAGS = $(AM_CXXFLAGS) $(BTCLITE_INCLUDES)
btclited_SOURCES = main/btclited.cpp \
				   main/init.cpp \
				   main/validation.cpp \
				   $(BTCLITE_HEADER_FILES)
btclited_LDADD = \
				 $(LIBBTCLITE_COMMON) \
				 $(LIBBTCLITE_CONSENSUS) \
				 $(LIBBTCLITE_UTIL)
btclited_LDADD += $(PTHREAD_LIBS) \
				  $(STDCPP_FILESYSTEM_LIBS)

MOSTLYCLEANFILES = utility/*.a consensus/*.a
