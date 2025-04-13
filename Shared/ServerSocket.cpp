#include"ServerSocket.hpp"
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdexcept>


void CServerSocket::_CloseSocket() noexcept {
    if (Handle != -1) {
        shutdown(Handle, SHUT_RDWR);close(Handle); Handle = -1;
    }
}
CServerSocket::CServerSocket() :AcceptingThread([this]() {
    while (1) {
        std::unique_lock LG(Mutex);
        if (StopAccepting) return;
        if (Handle == -1)
            CV.wait(LG, [&]()->bool {return StopAccepting || Handle != -1;});
        else {
            sockaddr_in localEndpoint; socklen_t _;
            //todo i feel that this is not thread safe
            LG.unlock(); int sock = accept(Handle, (sockaddr*)&localEndpoint, &_); LG.lock();
            if (StopAccepting) return;

            if (sock == -1) {
                if (errno == EINVAL) {
                    printf("stopped accepting clients\n"); return;
                }
                else fprintf(stderr, "failed accepting client, %i\n", errno);
            }
            else {
                auto ptr = ClientSocketFactory(sock, 128);//todo add custom size
                OnAccept(ptr);
            }
        }
    }
    }) {

}
CServerSocket::~CServerSocket() {
    std::unique_lock LG(Mutex);
    if (GetIsOpened()) Close();
    StopAccepting = true;
    CV.notify_all();
    LG.unlock(); AcceptingThread.join(); LG.lock();
}
void CServerSocket::Open(uint16_t port) {
    std::unique_lock LG(Mutex);
    if (GetIsOpened()) throw SocketError("Socket is already opened");

    if ((Handle = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        throw SocketError("Socket creation error");
    }

    sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(port);
    if (bind(Handle, (sockaddr*)&localAddr, sizeof(localAddr)) == -1) {
        close(Handle); Handle = -1;
        throw SocketError("Invalid port");
    }

    if (listen(Handle, 5) == -1) {
        close(Handle); Handle = -1;
        throw SocketError("Failed at creating listening queue");
    }

    CV.notify_all();
    OnOpen();
}
void CServerSocket::Close() {
    std::lock_guard LG(Mutex);
    if (!GetIsOpened()) throw SocketError("Socket is not opened");
    _CloseSocket();
    OnClose();
}
uint16_t CServerSocket::GetLocalPort() const {
    std::lock_guard LG(Mutex);
    sockaddr_in endp; socklen_t _ = sizeof(endp);
    if (getsockname(Handle, (sockaddr*)&endp, &_) == -1) throw SocketError("Failed getting local socket info");
    return ntohs(endp.sin_port);
}
