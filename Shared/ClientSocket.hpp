#pragma once
#include<string>
#include<vector>
#include<cstdint>
#include<thread>
#include<mutex>
#include<condition_variable>
#include"SocketShared.hpp"

//can connect to one endpoint and transfer data with it
class CClientSocket {
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
    void _ReadThreadFunc();
public:

    CClientSocket();
    CClientSocket(int linuxSocketHandle, size_t readBuffSize);
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
