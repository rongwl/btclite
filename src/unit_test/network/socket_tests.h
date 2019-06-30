#include <gmock/gmock.h>
#include "socket.h"


class MockSocket : public SocketInterface {
public:
    MOCK_METHOD3(Create, bool(int domain, int type, int protocol));
    MOCK_METHOD2(Bind, bool(const struct sockaddr *addr, socklen_t addr_len));
    MOCK_METHOD1(Listen, bool(int back_log));
    MOCK_METHOD2(Accept, int(struct sockaddr_storage *addr, socklen_t *addr_len));
    MOCK_METHOD2(Connect, bool(const struct sockaddr *addr, socklen_t addr_len));
    MOCK_METHOD0(Close, bool());
    MOCK_CONST_METHOD0(sock_fd, int());
};
