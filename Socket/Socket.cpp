#include"Socket.hpp"
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdexcept>

std::string GetIPFromDomen(std::string const& domen) {
    //todo
}

void CClientSocket::_CloseSocket() noexcept {
    if (Handle != -1) {
        shutdown(Handle, SHUT_RDWR); close(Handle);
        Handle = -1;
    }
}
CClientSocket::CClientSocket() :ReadingThread([this]() {
    while (1) {
        std::unique_lock LG(Mutex);
        if (StopReading) return;
        if (Handle == -1)
            CV.wait(LG, [&]()->bool {return StopReading || Handle != -1;});
        else {
            if (ReadBuffer.size() == 0) fprintf(stderr, "read buffer is empty\n");
            //todo i feel that this is not thread safe
            LG.unlock(); ssize_t bytes = read(Handle, ReadBuffer.data(), ReadBuffer.size()); LG.lock();
            printf("read %i\n", (int)bytes);
            if (bytes == -1) {
                fprintf(stderr, "error while reading %i\n", errno);
                Disconnect();
            }
            else if (bytes == 0)
                Disconnect();
            else
                OnRead(ReadBuffer.data(), bytes);
        }
    }
    }) {

}
CClientSocket::~CClientSocket() {
    std::unique_lock LG(Mutex);
    if (GetIsConnected()) Disconnect();
    StopReading = true;
    CV.notify_all();
    LG.unlock(); ReadingThread.join(); LG.lock();
}
void CClientSocket::Connect(std::string const& endpointIP, uint16_t endpointPort, uint16_t localPort) {
    std::lock_guard LG(Mutex);
    if (GetIsConnected()) throw SocketError("Socket is already connected");
    if ((Handle = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        throw SocketError("Socket creation error");
    }

    sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(localPort);
    if (bind(Handle, (sockaddr*)&localAddr, sizeof(localAddr)) == -1) {
        close(Handle); Handle = -1;
        throw SocketError("Invalid port");
    }

    //convert ip string to binary
    sockaddr_in endpointAddr; endpointAddr.sin_family = AF_INET; endpointAddr.sin_port = htons(endpointPort);
    if (inet_pton(AF_INET, endpointIP.data(), &endpointAddr.sin_addr) <= 0) {
        close(Handle); Handle = -1;
        throw SocketError("Invalid address/Address not supported");
    }

    if (connect(Handle, (sockaddr*)&endpointAddr, sizeof(endpointAddr)) == -1) {
        close(Handle); Handle = -1;
        throw SocketError("Connection failed");
    }

    CV.notify_all();
    OnConnect();
}
void CClientSocket::Disconnect() {
    std::lock_guard LG(Mutex);
    if (!GetIsConnected()) throw SocketError("Socket is not connected to anything");
    _CloseSocket();
    OnDisconnect();
}
void CClientSocket::Send(void const* data, size_t len) {
    std::lock_guard LG(Mutex);
    if (!GetIsConnected()) throw SocketError("Socket is not connected to anything");
    size_t bytesSend = 0;
    while (bytesSend < len) {
        ssize_t sended = send(Handle, (uint8_t*)data + bytesSend, len - bytesSend, 0);
        bytesSend += sended;
        if (sended == -1) throw SocketError("Failed sending with error " + std::to_string(errno));
    }
}
auto CClientSocket::GetRemoteEndpoint() const -> SEndpoint {
    std::lock_guard LG(Mutex);
    sockaddr_in endp; socklen_t _ = sizeof(endp);
    if (getpeername(Handle, (sockaddr*)&endp, &_) == -1) throw SocketError("Failed getting remote socket info");
    SEndpoint end; end.Port = ntohs(endp.sin_port);
    end.IP = "000.000.000.000";
    inet_ntop(AF_INET, &endp.sin_addr, end.IP.data(), end.IP.size() + 1);
    return end;
}
uint16_t CClientSocket::GetLocalPort() const {
    std::lock_guard LG(Mutex);
    sockaddr_in endp; socklen_t _ = sizeof(endp);
    if (getsockname(Handle, (sockaddr*)&endp, &_) == -1) throw SocketError("Failed getting local socket info");
    return ntohs(endp.sin_port);
}





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

            if (sock == -1)
                fprintf(stderr, "failed accepting client, %i\n", errno);
            else {
                auto ptr = ClientSocketFactory();
                std::lock_guard LG(ptr->Mutex);
                ptr->Handle = sock;
                ptr->SetReadBufferSize(128);//todo add custom size
                CV.notify_all();
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
