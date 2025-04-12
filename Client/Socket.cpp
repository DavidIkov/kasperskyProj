#include"Socket.hpp"
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdexcept>

void CClientSocket::_CloseSocket() noexcept {
    if (Handle != -1) {
        close(Handle);
        Handle = -1;
    }
}
CClientSocket::CClientSocket() :ReadingThread([this]() {
    while (1) {
        std::unique_lock LG(Mutex);
        if (StopReading) return;
        if (Handle == -1) {
            CV.wait(LG, [&]()->bool {return StopReading || Handle != -1;});
        }
        else {
            //todo i feel that this is not thread safe
            LG.unlock(); ssize_t bytes = read(Handle, &*ReadBuffer.begin(), ReadBuffer.size()); LG.lock();
            printf("wfbro1\n");
            if (bytes == -1) {
                OnReadError(errno); return;//todo dont just return
            }
            else if (bytes == 0)
                Disconnect();
            else
                OnRead(bytes);
        }
    }
    }) {

}
CClientSocket::~CClientSocket() {
    std::unique_lock LG(Mutex);
    printf("wfbro2\n");
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
        printf("%i\n", errno);
        throw SocketError("Connection failed");
    }

    CV.notify_all();
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
        if(sended==-1) throw SocketError("Failed sending with error " + std::to_string(errno));
    }
}
void CClientSocket::OnRead(size_t bytesAmount) {
    printf("%.*s", (int)bytesAmount, &*ReadBuffer.begin());
}
auto CClientSocket::GetRemoteEndpoint() const -> SEndpoint {
    std::lock_guard LG(Mutex);
    printf("getting pop %i\n", Handle);
    sockaddr_in endp; socklen_t _ = sizeof(endp);
    if (getpeername(Handle, (sockaddr*)&endp, &_) == -1) throw SocketError("Failed getting remote socket info" + std::to_string(Handle));
    SEndpoint end; end.Port = ntohs(endp.sin_port);
    end.IP = "000.000.000.000";
    inet_ntop(AF_INET, &endp.sin_addr, end.IP.data(), end.IP.size() + 1);
    printf("ending pop %i\n", Handle);
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
        if (Handle == -1) {
            CV.wait(LG, [&]()->bool {return StopAccepting || Handle != -1;});
        }
        else {
            sockaddr_in localEndpoint; socklen_t _;
            //todo i feel that this is not thread safe
            LG.unlock(); int sock = accept(Handle, (sockaddr*)&localEndpoint, &_); LG.lock();
            if (StopAccepting) return;

            if (sock == -1)
                OnAcceptError(errno);
            else {
                std::shared_ptr<CClientSocket> ptr(new CClientSocket());
                ptr->Mutex.lock();
                ptr->Handle = sock;
                CV.notify_all();
                auto endp = ptr->GetRemoteEndpoint();
                printf("huh %i\n", ptr->Handle);
                printf("works! %s\n", endp.IP.c_str());
                printf("huh %i\n", ptr->Handle);
                printf("huh %i, %i\n", (int)GetLocalPort(), (int)ptr->GetRemoteEndpoint().Port);
                printf("just for fun %i\n", (int)ptr->GetLocalPort());
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

    {
        //dirty
        sockaddr_in newLocAddr; socklen_t _ = sizeof(newLocAddr); getsockname(Handle, (sockaddr*)&newLocAddr, &_);
        LastLocalPort = ntohs(newLocAddr.sin_port);
    }

    CV.notify_all();
}
void CServerSocket::Close() {
    std::lock_guard LG(Mutex);
    if (!GetIsOpened()) throw SocketError("Socket is not opened");
    _CloseSocket();
}
uint16_t CServerSocket::GetLocalPort() const {
    std::lock_guard LG(Mutex);
    sockaddr_in endp; socklen_t _ = sizeof(endp);
    if (getsockname(Handle, (sockaddr*)&endp, &_) == -1) throw SocketError("Failed getting local socket info");
    return ntohs(endp.sin_port);
}
