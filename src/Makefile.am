AM_CXXFLAGS = $(PTHREAD_CFLAGS) 
EXTRA_LIBRARIES =

BTCLITE_CONFIG_INCLUDES = -I$(builddir)/config
BTCLITE_MAIN_INCLUDES = -I$(srcdir)/main/include
BTCLITE_COMMON_INCLUDES = -I$(srcdir)/common/include
BTCLITE_CHAIN_PROTO_INCLUDES = -I$(srcdir)/chain/proto
BTCLITE_CRYPTO_INCLUDES = -I$(srcdir)/crypto/include
BTCLITE_NET_INCLUDES = -I$(srcdir)/net/include
BTCLITE_UTIL_INCLUDES = -I$(srcdir)/utility/include


BTCLITE_INCLUDES = $(BTCLITE_MAIN_INCLUDES) \
				   $(BTCLITE_COMMON_INCLUDES) \
				   $(BTCLITE_CHAIN_PROTO_INCLUDES) \
				   $(BTCLITE_CRYPTO_INCLUDES) \
				   $(BTCLITE_NET_INCLUDES) \
				   $(BTCLITE_UTIL_INCLUDES) 


STDCPP_FILESYSTEM_LIBS = -lstdc++fs
LIBBTCLITE_COMMON = common/src/libbtclite_common.a
LIBBTCLITE_CRYPTO = crypto/src/libbtclite_crypto.a
LIBBTCLITE_UTIL = utility/src/libbtclite_util.a

EXTRA_LIBRARIES += \
				   $(LIBBTCLITE_COMMON) \
				   $(LIBBTCLITE_CRYPTO) \
				   $(LIBBTCLITE_UTIL)

bin_PROGRAMS = btclited

BTCLITE_HEADER_FILES = \
					   common/include/block.h \
					   common/include/params.h \
					   common/include/script.h \
					   common/include/script_witness.h \
					   common/include/transaction.h \
					   config/environment.h \
					   main/include/clientversion.h \
					   main/include/init.h \
					   net/include/net.h \
					   consensus/include/params.h \
					   crypto/include/hash.h \
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
										 $(BTCLITE_MAIN_INCLUDES)
utility_src_libbtclite_util_a_SOURCES = utility/src/sync.cpp \
										utility/src/util.cpp \
										utility/src/utiltime.cpp \
										utility/src/utilstrencodings.cpp 


btclited_CPPFLAGS = $(AM_CXXFLAGS) \
					$(BOTAN_CFLAGS) \
					$(GLOG_CFLAGS) \
					$(BTCLITE_INCLUDES)
btclited_SOURCES = main/src/btclited.cpp \
				   main/src/init.cpp \
				   main/src/validation.cpp 
				   $(BTCLITE_HEADER_FILES)
btclited_LDADD = \
				 $(LIBBTCLITE_COMMON) \
				 $(LIBBTCLITE_CRYPTO) \
				 $(LIBBTCLITE_UTIL)
btclited_LDADD += $(PTHREAD_LIBS) \
				  $(STDCPP_FILESYSTEM_LIBS) \
				  $(PROTOBUF_LIBS) \
				  $(BOTAN_LIBS) \
				  $(GLOG_LIBS)

#chain/proto/%.pb.cc chain/proto/%.pb.h: chain/proto/%.proto
#	$(PROTOC) --proto_path=chain/proto --cpp_out=chain/proto $^


MOSTLYCLEANFILES = utility/src/*.a \
				   common/src/*.a \
				   crypto/src/*.a
