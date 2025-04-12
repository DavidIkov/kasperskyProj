#pragma once
#include<string>
#include<vector>
#include<cstdint>
#include<thread>
#include<mutex>
#include<condition_variable>

class SocketError : public std::exception {
private:
    std::string err;
public:
    SocketError(std::string const& str) :err(str) {}
    virtual const char* what() const noexcept override { return err.c_str(); }
};
struct SEndpoint{
    std::string IP = "";
    uint16_t Port = 0;
};

std::string GetIPFromDomen(std::string const& domen);

//can connect to one endpoint and transfer data with it
class CClientSocket {
    friend class CServerSocket;
private:
    int Handle = -1;
    std::vector<uint8_t> ReadBuffer;

protected:
    mutable std::recursive_mutex Mutex;
    std::condition_variable_any CV;
private:

    bool StopReading = false;
    std::thread ReadingThread;

    //right now im just dropping connection and not doing gracefull disconnect
    void _CloseSocket() noexcept;
public:

    CClientSocket();
    CClientSocket(CClientSocket const&) = delete;
    CClientSocket(CClientSocket&&) = delete;
    CClientSocket& operator=(CClientSocket const&) = delete;
    CClientSocket& operator=(CClientSocket&&) = delete;
    virtual ~CClientSocket();
protected:
    inline virtual void OnConnect() { printf("Connected\n"); }
    inline virtual void OnDisconnect() { printf("Disconnected\n"); }
    inline virtual void OnRead(uint8_t* buf, size_t bytesAmount) {}
public:
    //dosent take in account that socket may be created and not yet connected(in process of connecting)
    inline bool GetIsConnected() const noexcept { std::lock_guard LG(Mutex); return Handle != -1; }
    SEndpoint GetRemoteEndpoint() const;
    uint16_t GetLocalPort() const;
    //todo send http request to api.ipify.org to get ip
    //SEndpoint GetLocalEndpoint() const;

    inline void SetReadBufferSize(size_t size) { std::lock_guard LG(Mutex); ReadBuffer.resize(size, 0); }
public:
    //if localPort is 0 OS will decide which port to use
    void Connect(std::string const& endpointIP, uint16_t endpointPort, uint16_t localPort = 0);
    void Disconnect();
    void Send(void const* data, size_t len);
};

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
    inline virtual std::shared_ptr<CClientSocket> ClientSocketFactory() {
        return std::shared_ptr<CClientSocket>(new CClientSocket());
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