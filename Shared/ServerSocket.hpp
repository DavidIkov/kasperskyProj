#pragma once
#include"ClientSocket.hpp"

//aka "server socket", can accept connections and produce CClientSocketfor each new connection.
//by itself cant transfer data since it dosent have an endpoint
class CServerSocket {
private:
    int Handle = -1;

protected:
    mutable std::recursive_mutex Mutex;
    std::condition_variable_any CV;
private:

    bool StopAccepting = false;
    std::thread AcceptingThread;

    void _CloseSocket() noexcept;
public:

    CServerSocket();
    CServerSocket(CServerSocket const&) = delete;
    CServerSocket(CServerSocket&&) = delete;
    CServerSocket& operator=(CServerSocket const&) = delete;
    CServerSocket& operator=(CServerSocket&&) = delete;
    virtual ~CServerSocket();
protected:
    inline virtual void OnAccept(std::shared_ptr<CClientSocket> ptr) { printf("accepted\n"); }
    inline virtual void OnOpen() {}
    inline virtual void OnClose() {}
    inline virtual std::shared_ptr<CClientSocket> ClientSocketFactory(int Handle, size_t bufSize) {
        return std::shared_ptr<CClientSocket>(new CClientSocket(Handle, bufSize));
    };
public:
    inline bool GetIsOpened() const noexcept { std::lock_guard LG(Mutex); return Handle != -1; }
    uint16_t GetLocalPort() const;
    //todo send http request to api.ipify.org to get ip
    //SEndpoint GetLocalEndpoint() const;

public:
    //if port is 0, then OS will decide which port to use
    void Open(uint16_t port = 0);
    void Close();
};