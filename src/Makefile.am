AM_CXXFLAGS = $(PTHREAD_CFLAGS) 
EXTRA_LIBRARIES =

BTCLITE_CONFIG_INCLUDES = -I$(builddir)/config
BTCLITE_MAIN_INCLUDES = -I$(srcdir)/main/include
BTCLITE_CHAIN_INCLUDES = -I$(srcdir)/chain/include
BTCLITE_CONSENSUS_INCLUDES = -I$(srcdir)/consensus/include
BTCLITE_NET_INCLUDES = -I$(srcdir)/net/include
BTCLITE_UTIL_INCLUDES = -I$(srcdir)/utility/include

BTCLITE_INCLUDES = $(BTCLITE_MAIN_INCLUDES) \
				   $(BTCLITE_CHAIN_INCLUDES) \
				   $(BTCLITE_CONSENSUS_INCLUDES) \
				   $(BTCLITE_NET_INCLUDES) \
				   $(BTCLITE_UTIL_INCLUDES)


STDCPP_FILESYSTEM_LIBS = -lstdc++fs
LIBBTCLITE_CONSENSUS = consensus/src/libbtclite_consensus.a
LIBBTCLITE_UTIL = utility/src/libbtclite_util.a

EXTRA_LIBRARIES += \
				   $(LIBBTCLITE_COMMON) \
				   $(LIBBTCLITE_CONSENSUS) \
				   $(LIBBTCLITE_UTIL)

bin_PROGRAMS = btclited

BTCLITE_HEADER_FILES = \
					   chain/include/block.h \
					   chain/include/chainparams.h \
					   main/include/clientversion.h \
					   main/include/init.h \
					   net/include/net.h \
					   consensus/includ/uint256.h \
					   utility/includ/serialize.h \
					   utility/includ/sync.h \
					   utility/includ/tinyformat.h \
					   utility/includ/util.h \
					   utility/includ/utilstrencodings.h \
					   utility/includ/utiltime.h


consensus_src_libbtclite_consensus_a_CPPFLAGS = $(AM_CXXFLAGS) \
												$(BTCLITE_CONSENSUS_INCLUDES) \
												$(BTCLITE_UTIL_INCLUDES)
consensus_src_libbtclite_consensus_a_SOURCES = consensus/src/uint256.cpp \
											   utility/src/utilstrencodings.cpp

utility_src_libbtclite_util_a_CPPFLAGS = $(AM_CXXFLAGS) \
										 $(BTCLITE_UTIL_INCLUDES) \
										 $(BTCLITE_MAIN_INCLUDES)
utility_src_libbtclite_util_a_SOURCES = utility/src/sync.cpp \
										utility/src/util.cpp \
										utility/src/utiltime.cpp \
										utility/src/utilstrencodings.cpp 


btclited_CPPFLAGS = $(AM_CXXFLAGS) \
					$(BTCLITE_INCLUDES)
btclited_SOURCES = main/src/btclited.cpp \
				   main/src/init.cpp \
				   main/src/validation.cpp \
				   $(BTCLITE_HEADER_FILES)
btclited_LDADD = \
				 $(LIBBTCLITE_COMMON) \
				 $(LIBBTCLITE_CONSENSUS) \
				 $(LIBBTCLITE_UTIL)
btclited_LDADD += $(PTHREAD_LIBS) \
				  $(STDCPP_FILESYSTEM_LIBS)

MOSTLYCLEANFILES = utility/src/*.a consensus/src/*.a
