AM_CXXFLAGS = $(PTHREAD_CFLAGS) 
EXTRA_LIBRARIES =

BTCLITE_CONFIG_INCLUDES = -I$(builddir)/config
BTCLITE_MAIN_INCLUDES = -I$(srcdir)/main/include
BTCLITE_CHAIN_INCLUDES = -I$(srcdir)/chain/include
BTCLITE_CHAIN_PROTO_INCLUDES = -I$(srcdir)/chain/proto
BTCLITE_CONSENSUS_INCLUDES = -I$(srcdir)/consensus/include
BTCLITE_NET_INCLUDES = -I$(srcdir)/net/include
BTCLITE_TRANSACTION_INCLUDES = -I$(srcdir)/transaction/include
BTCLITE_UTIL_INCLUDES = -I$(srcdir)/utility/include

BTCLITE_INCLUDES = $(BTCLITE_MAIN_INCLUDES) \
				   $(BTCLITE_CHAIN_INCLUDES) \
				   $(BTCLITE_CHAIN_PROTO_INCLUDES) \
				   $(BTCLITE_CONSENSUS_INCLUDES) \
				   $(BTCLITE_NET_INCLUDES) \
				   $(BTCLITE_TRANSACTION_INCLUDES) \
				   $(BTCLITE_UTIL_INCLUDES)


STDCPP_FILESYSTEM_LIBS = -lstdc++fs
LIBBTCLITE_CHAIN = chain/src/libbtclite_chain.a
LIBBTCLITE_CONSENSUS = consensus/src/libbtclite_consensus.a
LIBBTCLITE_TRANSACTION = transaction/src/libbtclite_transaction.a
LIBBTCLITE_UTIL = utility/src/libbtclite_util.a

EXTRA_LIBRARIES += \
				   $(LIBBTCLITE_CHAIN) \
				   $(LIBBTCLITE_CONSENSUS) \
				   $(LIBBTCLITE_TRANSACTION) \
				   $(LIBBTCLITE_UTIL)

bin_PROGRAMS = btclited

BTCLITE_HEADER_FILES = \
					   chain/include/block.h \
					   chain/include/chainparams.h \
					   main/include/clientversion.h \
					   main/include/init.h \
					   net/include/net.h \
					   consensus/include/hash.h \
					   transaction/include/script.h \
					   transaction/include/transaction.h \
					   utility/include/blob.h \
					   utility/include/Endian.h \
					   utility/include/arithmetic.h \
					   utility/include/serialize.h \
					   utility/include/constants.h \
					   utility/include/sync.h \
					   utility/include/tinyformat.h \
					   utility/include/util.h \
					   utility/include/utilstrencodings.h \
					   utility/include/utiltime.h


chain_src_libbtclite_chain_a_CPPFLAGS = $(AM_CXXFLAGS) \
										$(BOTAN_CFLAGS) \
										$(BTCLITE_CHAIN_INCLUDES) \
										$(BTCLITE_CONSENSUS_INCLUDES) \
										$(BTCLITE_TRANSACTION_INCLUDES) \
										$(BTCLITE_UTIL_INCLUDES) 
chain_src_libbtclite_chain_a_SOURCES = chain/src/block.cpp \
									   chain/src/chainparams.cpp


consensus_src_libbtclite_consensus_a_CPPFLAGS = $(AM_CXXFLAGS) \
											$(BOTAN_CFLAGS) \
											$(BTCLITE_CONSENSUS_INCLUDES) \
											$(BTCLITE_UTIL_INCLUDES)
consensus_src_libbtclite_consensus_a_SOURCES = consensus/src/hash.cpp



transaction_src_libbtclite_transaction_a_CPPFLAGS = $(AM_CXXFLAGS) \
													$(BOTAN_CFLAGS) \
													$(BTCLITE_TRANSACTION_INCLUDES) \
													$(BTCLITE_CONSENSUS_INCLUDES) \
													$(BTCLITE_UTIL_INCLUDES) \
													$(BTCLITE_CHAIN_PROTO_INCLUDES)
transaction_src_libbtclite_transaction_a_SOURCES = transaction/src/script.cpp \
												   transaction/src/transaction.cpp


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
				   main/src/validation.cpp 
				   $(BTCLITE_HEADER_FILES)
btclited_LDADD = \
				 $(LIBBTCLITE_CHAIN) \
				 $(LIBBTCLITE_CONSENSUS) \
				 $(LIBBTCLITE_TRANSACTION) \
				 $(LIBBTCLITE_UTIL)
btclited_LDADD += $(PTHREAD_LIBS) \
				  $(STDCPP_FILESYSTEM_LIBS) \
				  $(PROTOBUF_LIBS)

#chain/proto/%.pb.cc chain/proto/%.pb.h: chain/proto/%.proto
#	$(PROTOC) --proto_path=chain/proto --cpp_out=chain/proto $^


MOSTLYCLEANFILES = utility/src/*.a \
				   transaction/src/*.a \
				   chain/src/*.a \
				   consensus/src/*.a
