AM_CXXFLAGS = $(PTHREAD_CFLAGS) 
EXTRA_LIBRARIES =

BTCLITE_CONFIG_INCLUDES = -I$(builddir)/config
BTCLITE_FULLNODE_INCLUDES = -I$(srcdir)/fullnode/include
BTCLITE_COMMON_INCLUDES = -I$(srcdir)/common/include
BTCLITE_CONSENSUS_INCLUDES = -I$(srcdir)/consensus/include
BTCLITE_CHAIN_PROTO_INCLUDES = -I$(srcdir)/chain/proto
BTCLITE_CRYPTO_INCLUDES = -I$(srcdir)/crypto/include
BTCLITE_NET_INCLUDES = -I$(srcdir)/net/include
BTCLITE_UTIL_INCLUDES = -I$(srcdir)/utility/include


BTCLITE_INCLUDES = $(BTCLITE_FULLNODE_INCLUDES) \
				   $(BTCLITE_COMMON_INCLUDES) \
				   $(BTCLITE_CONSENSUS_INCLUDES) \
				   $(BTCLITE_CHAIN_PROTO_INCLUDES) \
				   $(BTCLITE_CRYPTO_INCLUDES) \
				   $(BTCLITE_NET_INCLUDES) \
				   $(BTCLITE_UTIL_INCLUDES) 


STDCPP_FILESYSTEM_LIBS = -lstdc++fs
LIBBTCLITE_COMMON = common/src/libbtclite_common.a
LIBBTCLITE_CONSENSUS = consensus/src/libbtclite_consensus.a
LIBBTCLITE_CRYPTO = crypto/src/libbtclite_crypto.a
LIBBTCLITE_UTIL = utility/src/libbtclite_util.a

EXTRA_LIBRARIES += \
				   $(LIBBTCLITE_COMMON) \
				   $(LIBBTCLITE_CONSENSUS) \
				   $(LIBBTCLITE_CRYPTO) \
				   $(LIBBTCLITE_UTIL)

bin_PROGRAMS = btc-fullnode

BTCLITE_HEADER_FILES = \
					   common/include/block.h \
					   common/include/params.h \
					   common/include/script.h \
					   common/include/script_witness.h \
					   common/include/transaction.h \
					   config/environment.h \
					   fullnode/include/clientversion.h \
					   net/include/net.h \
					   consensus/include/params.h \
					   crypto/include/hash.h \
					   utility/include/blob.h \
					   utility/include/Endian.h \
					   utility/include/arithmetic.h \
					   utility/include/constants.h \
					   utility/include/logging.h \
					   utility/include/serialize.h \
					   utility/include/sync.h \
					   utility/include/tinyformat.h \
					   utility/include/util.h \
					   utility/include/utilstrencodings.h \
					   utility/include/utiltime.h


common_src_libbtclite_common_a_CPPFLAGS = $(AM_CXXFLAGS) \
										  $(BOTAN_CFLAGS) \
										  $(GLOG_CFLAGS) \
										  $(BTCLITE_COMMON_INCLUDES) \
										  $(BTCLITE_CRYPTO_INCLUDES) \
										  $(BTCLITE_UTIL_INCLUDES) 
common_src_libbtclite_common_a_SOURCES = common/src/block.cpp \
										 common/src/params.cpp \
										 common/src/script.cpp \
										 common/src/script_witness.cpp \
										 common/src/transaction.cpp


consensus_src_libbtclite_consensus_a_CPPFLAGS = $(BOTAN_CFLAGS) \
												$(GLOG_CFLAGS) \
												$(BTCLITE_COMMON_INCLUDES) \
												$(BTCLITE_CONSENSUS_INCLUDES) \
												$(BTCLITE_CRYPTO_INCLUDES) \
												$(BTCLITE_UTIL_INCLUDES)
consensus_src_libbtclite_consensus_a_SOURCES = consensus/src/params.cpp

crypto_src_libbtclite_crypto_a_CPPFLAGS = $(AM_CXXFLAGS) \
										  $(BOTAN_CFLAGS) \
										  $(GLOG_CFLAGS) \
										  $(BTCLITE_CRYPTO_INCLUDES) \
										  $(BTCLITE_UTIL_INCLUDES)
crypto_src_libbtclite_crypto_a_SOURCES = crypto/src/hash.cpp



utility_src_libbtclite_util_a_CPPFLAGS = $(AM_CXXFLAGS) \
										 $(GLOG_CFLAGS) \
										 $(BOTAN_CFLAGS) \
										 $(BTCLITE_UTIL_INCLUDES) \
										 $(BTCLITE_CRYPTO_INCLUDES) \
										 $(BTCLITE_FULLNODE_INCLUDES)
utility_src_libbtclite_util_a_SOURCES = utility/src/sync.cpp \
										utility/src/logging.cpp \
										utility/src/util.cpp \
										utility/src/utiltime.cpp \
										utility/src/utilstrencodings.cpp 


btc_fullnode_CPPFLAGS = $(AM_CXXFLAGS) \
						$(BOTAN_CFLAGS) \
						$(GLOG_CFLAGS) \
						$(BTCLITE_INCLUDES)
btc_fullnode_SOURCES = fullnode/src/main.cpp \
					   fullnode/src/executor.cpp \
					   fullnode/src/validation.cpp \
					   $(BTCLITE_HEADER_FILES)
btc_fullnode_LDADD = \
					 $(LIBBTCLITE_COMMON) \
					 $(LIBBTCLITE_CONSENSUS) \
					 $(LIBBTCLITE_CRYPTO) \
					 $(LIBBTCLITE_UTIL)
btc_fullnode_LDADD += $(PTHREAD_LIBS) \
					  $(STDCPP_FILESYSTEM_LIBS) \
					  $(PROTOBUF_LIBS) \
					  $(BOTAN_LIBS) \
					  $(GLOG_LIBS)

#chain/proto/%.pb.cc chain/proto/%.pb.h: chain/proto/%.proto
#	$(PROTOC) --proto_path=chain/proto --cpp_out=chain/proto $^


MOSTLYCLEANFILES = utility/src/*.a \
				   common/src/*.a \
				   consensus/src/*.a \
				   crypto/src/*.a
