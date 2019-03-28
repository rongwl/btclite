AM_CXXFLAGS = $(PTHREAD_CFLAGS) 
EXTRA_LIBRARIES =

BTCLITE_CONFIG_INCLUDES = -I$(builddir)/config
BTCLITE_FULLNODE_INCLUDES = -I$(srcdir)/fullnode/include
BTCLITE_CHAIN_INCLUDES = -I$(srcdir)/chain/include
BTCLITE_CHAIN_PROTO_INCLUDES = -I$(srcdir)/chain/proto
BTCLITE_CRYPTO_INCLUDES = -I$(srcdir)/crypto/include
BTCLITE_NETWORK_INCLUDES = -I$(srcdir)/network/include
BTCLITE_UTIL_INCLUDES = -I$(srcdir)/utility/include


BTCLITE_INCLUDES = $(BTCLITE_FULLNODE_INCLUDES) \
				   $(BTCLITE_CHAIN_INCLUDES) \
				   $(BTCLITE_CHAIN_PROTO_INCLUDES) \
				   $(BTCLITE_CRYPTO_INCLUDES) \
				   $(BTCLITE_NETWORK_INCLUDES) \
				   $(BTCLITE_UTIL_INCLUDES) 


STDCPP_FILESYSTEM_LIBS = -lstdc++fs
LIBBTCLITE_CHAIN = chain/src/libbtclite_chain.a
LIBBTCLITE_CRYPTO = crypto/src/libbtclite_crypto.a
LIBBTCLITE_NETWORK = network/src/libbtclite_network.a
LIBBTCLITE_UTIL = utility/src/libbtclite_util.a

EXTRA_LIBRARIES += \
				   $(LIBBTCLITE_CHAIN) \
				   $(LIBBTCLITE_CRYPTO) \
				   $(LIBBTCLITE_NETWORK) \
				   $(LIBBTCLITE_UTIL)

bin_PROGRAMS = btc-fullnode

BTCLITE_HEADER_FILES = \
					   chain/include/block.h \
					   chain/include/params.h \
					   chain/include/script.h \
					   chain/include/script_witness.h \
					   chain/include/transaction.h \
					   config/environment.h \
					   fullnode/include/clientversion.h \
					   fullnode/include/config.h \
					   network/include/params.h \
					   network/include/net.h \
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


chain_src_libbtclite_chain_a_CPPFLAGS = $(AM_CXXFLAGS) \ 
                                        $(BOTAN_CFLAGS) \
										$(GLOG_CFLAGS) \
										$(BTCLITE_CHAIN_INCLUDES) \
										$(BTCLITE_CRYPTO_INCLUDES) \
										$(BTCLITE_UTIL_INCLUDES)
chain_src_libbtclite_chain_a_SOURCES = chain/src/block.cpp \
									   chain/src/script.cpp \
									   chain/src/script_witness.cpp \
									   chain/src/transaction.cpp \ 
                                       chain/src/params.cpp


network_src_libbtclite_network_a_CPPFLAGS = $(AM_CXXFLAGS) \
											$(BOTAN_CFLAGS) \
                                            $(GLOG_CFLAGS) \
											$(BTCLITE_CRYPTO_INCLUDES) \
											$(BTCLITE_NETWORK_INCLUDES) \
											$(BTCLITE_UTIL_INCLUDES)
network_src_libbtclite_network_a_SOURCES = network/src/params.cpp \ 
                                           network/src/net.cpp


btc_fullnode_CPPFLAGS = $(AM_CXXFLAGS) \
						$(BOTAN_CFLAGS) \
						$(GLOG_CFLAGS) \
						$(BTCLITE_INCLUDES)
btc_fullnode_SOURCES = fullnode/src/main.cpp \
					   fullnode/src/config.cpp \
					   fullnode/src/executor.cpp \
					   fullnode/src/validation.cpp \
					   $(BTCLITE_HEADER_FILES)
btc_fullnode_LDADD = \
					 $(LIBBTCLITE_CHAIN) \
					 $(LIBBTCLITE_CRYPTO) \
					 $(LIBBTCLITE_NETWORK) \
					 $(LIBBTCLITE_UTIL)
btc_fullnode_LDADD += $(PTHREAD_LIBS) \
					  $(STDCPP_FILESYSTEM_LIBS) \
					  $(PROTOBUF_LIBS) \
					  $(BOTAN_LIBS) \
					  $(GLOG_LIBS)

#chain/proto/%.pb.cc chain/proto/%.pb.h: chain/proto/%.proto
#	$(PROTOC) --proto_path=chain/proto --cpp_out=chain/proto $^


MOSTLYCLEANFILES = utility/src/*.a \
				   chain/src/*.a \
				   crypto/src/*.a \
				   network/src/*.a
