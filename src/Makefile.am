AM_CXXFLAGS = $(PTHREAD_CFLAGS) 
EXTRA_LIBRARIES =

BTCLITE_CONFIG_INCLUDES = -I$(builddir)/config
BTCLITE_FULLNODE_INCLUDES = -I$(srcdir)/fullnode/include
BTCLITE_CHAIN_INCLUDES = -I$(srcdir)/chain/include
BTCLITE_CHAIN_PROTO_INCLUDES = -I$(srcdir)/chain/proto
BTCLITE_CRYPTO_INCLUDES = -I$(srcdir)/crypto/include
BTCLITE_NETWORK_INCLUDES = -I$(srcdir)/network/include
BTCLITE_NETWORK_PROTO_INCLUDES = -I$(srcdir)/network/proto
BTCLITE_UTIL_INCLUDES = -I$(srcdir)/utility/include


BTCLITE_INCLUDES = $(BTCLITE_FULLNODE_INCLUDES) \
                   $(BTCLITE_CHAIN_INCLUDES) \
                   $(BTCLITE_CHAIN_PROTO_INCLUDES) \
                   $(BTCLITE_CRYPTO_INCLUDES) \
                   $(BTCLITE_NETWORK_INCLUDES) \
                   $(BTCLITE_NETWORK_PROTO_INCLUDES) \
                   $(BTCLITE_UTIL_INCLUDES) 


STDCPP_FILESYSTEM_LIBS = -lstdc++fs
LIBBTCLITE_FULLNODE = fullnode/src/libbtclite_fullnode.a
LIBBTCLITE_CHAIN = chain/src/libbtclite_chain.a
LIBBTCLITE_CRYPTO = crypto/src/libbtclite_crypto.a
LIBBTCLITE_NETWORK = network/src/libbtclite_network.a
LIBBTCLITE_UTIL = utility/src/libbtclite_util.a

EXTRA_LIBRARIES += \
                   $(LIBBTCLITE_FULLNODE) \
                   $(LIBBTCLITE_CHAIN) \
                   $(LIBBTCLITE_CRYPTO) \
                   $(LIBBTCLITE_NETWORK) \
                   $(LIBBTCLITE_UTIL)

bin_PROGRAMS = btc-fullnode

BTCLITE_HEADER_FILES = \
                       chain/include/block.h \
                       chain/include/chain.h \
                       chain/include/compact.h \
                       chain/include/params.h \
                       chain/include/script.h \
                       chain/include/script_witness.h \
                       chain/include/transaction.h \
                       config/environment.h \
                       fullnode/include/clientversion.h \
                       fullnode/include/config.h \
                       network/include/acceptor.h \
                       network/include/bandb.h \
                       network/include/block_sync.h \
                       network/include/bloom.h \
                       network/include/connector.h \
                       network/include/libevent.h \
                       network/include/protocol/message.h \
                       network/include/protocol/address.h \
                       network/include/protocol/inventory.h \
                       network/include/protocol/inventory_vector.h \
                       network/include/protocol/getaddr.h \
                       network/include/protocol/ping.h \
                       network/include/protocol/pong.h \
                       network/include/protocol/reject.h \
                       network/include/protocol/send_headers.h \
                       network/include/protocol/send_compact.h \
                       network/include/protocol/verack.h \
                       network/include/protocol/version.h \
                       network/include/msg_process.h \
                       network/include/node.h \
                       network/include/p2p.h \
                       network/include/params.h \
                       network/include/peers.h \
                       network/include/net.h \
                       network/include/netbase.h \
                       network/include/network_address.h \
                       network/include/socket.h \
                       crypto/include/hash.h \
                       utility/include/arithmetic.h \
                       utility/include/blob.h \
                       utility/include/circular_buffer.h \
                       utility/include/constants.h \
                       utility/include/Endian.h \
                       utility/include/error.h \
                       utility/include/fs.h \
                       utility/include/logging.h \
                       utility/include/random.h \
                       utility/include/serialize.h \
                       utility/include/sync.h \
                       utility/include/thread.h \
                       utility/include/tinyformat.h \
                       utility/include/util.h \
                       utility/include/stream.h \
                       utility/include/string_encoding.h \
                       utility/include/timer.h \
                       utility/include/utiltime.h 


dist_noinst_DATA = network/proto/network_address.proto \
                   network/proto/peers.proto



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
                                         $(BTCLITE_NETWORK_INCLUDES) \
                                         $(BTCLITE_NETWORK_PROTO_INCLUDES) \
                                         $(BTCLITE_FULLNODE_INCLUDES)
utility_src_libbtclite_util_a_SOURCES = utility/src/error.cpp \
                                        utility/src/logging.cpp \
                                        utility/src/random.cpp \
                                        utility/src/serialize.cpp \
                                        utility/src/string_encoding.cpp \
                                        utility/src/sync.cpp \
                                        utility/src/thread.cpp \
                                        utility/src/timer.cpp \
                                        utility/src/util.cpp \
                                        utility/src/utiltime.cpp 


chain_src_libbtclite_chain_a_CPPFLAGS = $(AM_CXXFLAGS) \
                                        $(BOTAN_CFLAGS) \
                                        $(GLOG_CFLAGS) \
                                        $(BTCLITE_CHAIN_INCLUDES) \
                                        $(BTCLITE_CRYPTO_INCLUDES) \
                                        $(BTCLITE_NETWORK_INCLUDES) \
                                        $(BTCLITE_UTIL_INCLUDES)
chain_src_libbtclite_chain_a_SOURCES = chain/src/block.cpp \
                                       chain/src/chain.cpp \
                                       chain/src/compact.cpp \
                                       chain/src/script.cpp \
                                       chain/src/script_witness.cpp \
                                       chain/src/transaction.cpp \
                                       chain/src/params.cpp


network_src_libbtclite_network_a_CPPFLAGS = $(AM_CXXFLAGS) \
                                            $(BOTAN_CFLAGS) \
                                            $(GLOG_CFLAGS) \
                                            $(BTCLITE_CHAIN_INCLUDES) \
                                            $(BTCLITE_CONFIG_INCLUDES) \
                                            $(BTCLITE_CRYPTO_INCLUDES) \
                                            $(BTCLITE_NETWORK_INCLUDES) \
                                            $(BTCLITE_NETWORK_PROTO_INCLUDES) \
                                            $(BTCLITE_UTIL_INCLUDES) \
                                            $(BTCLITE_FULLNODE_INCLUDES)
network_src_libbtclite_network_a_SOURCES = network/proto/banmap.pb.cc \
                                           network/proto/network_address.pb.cc \
                                           network/proto/peers.pb.cc \
                                           network/src/acceptor.cpp \
                                           network/src/bandb.cpp \
                                           network/src/block_sync.cpp \
                                           network/src/connector.cpp \
                                           network/src/libevent.cpp \
                                           network/src/protocol/message.cpp \
                                           network/src/protocol/address.cpp \
                                           network/src/protocol/inventory.cpp \
                                           network/src/protocol/getaddr.cpp \
                                           network/src/protocol/ping.cpp \
                                           network/src/protocol/pong.cpp \
                                           network/src/protocol/reject.cpp \
                                           network/src/protocol/send_headers.cpp \
                                           network/src/protocol/send_compact.cpp \
                                           network/src/protocol/verack.cpp \
                                           network/src/protocol/version.cpp \
                                           network/src/msg_process.cpp \
                                           network/src/netbase.cpp \
                                           network/src/network_address.cpp \
                                           network/src/params.cpp \
                                           network/src/peers.cpp \
                                           network/src/p2p.cpp \
                                           network/src/net.cpp \
                                           network/src/node.cpp \
                                           network/src/socket.cpp


fullnode_src_libbtclite_fullnode_a_CPPFLAGS = $(AM_CXXFLAGS) \
                                              $(BOTAN_CFLAGS) \
                                              $(GLOG_CFLAGS) \
                                              $(BTCLITE_CRYPTO_INCLUDES) \
                                              $(BTCLITE_CHAIN_INCLUDES) \
                                              $(BTCLITE_FULLNODE_INCLUDES) \
                                              $(BTCLITE_NETWORK_INCLUDES) \
                                              $(BTCLITE_NETWORK_PROTO_INCLUDES) \
                                              $(BTCLITE_UTIL_INCLUDES)
fullnode_src_libbtclite_fullnode_a_SOURCES = fullnode/src/config.cpp \
                                             fullnode/src/executor.cpp \
                                             fullnode/src/validation.cpp


btc_fullnode_CPPFLAGS = $(AM_CXXFLAGS) \
                        $(BOTAN_CFLAGS) \
                        $(GLOG_CFLAGS) \
                        $(BTCLITE_INCLUDES)
btc_fullnode_SOURCES = fullnode/src/main.cpp \
                       $(BTCLITE_HEADER_FILES)
btc_fullnode_LDADD = \
                     $(LIBBTCLITE_FULLNODE) \
                     $(LIBBTCLITE_NETWORK) \
                     $(LIBBTCLITE_CHAIN) \
                     $(LIBBTCLITE_CRYPTO) \
                     $(LIBBTCLITE_UTIL)
btc_fullnode_LDADD += $(PTHREAD_LIBS) \
                      $(STDCPP_FILESYSTEM_LIBS) \
                      $(PROTOBUF_LIBS) \
                      $(BOTAN_LIBS) \
                      $(GLOG_LIBS) \
                      $(EVENT_LIBS) \
                      $(EVENT_PTHREADS_LIBS)


network/proto/%.pb.cc network/proto/%.pb.h: network/proto/%.proto
	$(PROTOC) --proto_path=network/proto --cpp_out=network/proto $^


if ENABLE_UNIT_TESTS
include Makefile.test.include
endif


MOSTLYCLEANFILES = fullnode/src/*.a \
                   utility/src/*.a \
                   chain/src/*.a \
                   crypto/src/*.a \
                   network/src/*.a \
                   network/proto/*.pb.*
