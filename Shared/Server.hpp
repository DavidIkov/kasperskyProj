#pragma once
#include"ServerSocket.hpp"

class CServer :public CServerSocket {
protected:
    virtual void OnAccept(std::shared_ptr<CClientSocket> ptr) override;
    inline virtual std::shared_ptr<CClientSocket> ClientSocketFactory(int handle, size_t bufSize) override {
        return std::shared_ptr<CClientSocket>(new CClientSocketForServer(this, handle, bufSize));
    };
private:
    friend class CClientSocketForServer;
    class CClientSocketForServer :public CClientSocket {
    protected:
        CServer* Serv;
        inline virtual void OnDisconnect() override {
            CClientSocket::OnDisconnect();
            std::lock_guard LG(Serv->Mutex);
            Serv->Work.push_back(this);
            Serv->CV.notify_all();
        }
        inline virtual void OnRead(uint8_t* buf, size_t bytesAmount) override {
            CClientSocket::OnRead(buf, bytesAmount);
            std::lock_guard LG(Serv->Mutex);
            Serv->_ProcessClientInput(buf, bytesAmount);
        }
    public:
        CClientSocketForServer() = delete;
        CClientSocketForServer(CServer* serv, int handle, size_t bufSize) :CClientSocket(handle, bufSize), Serv(serv) {}
    };
    std::vector<std::shared_ptr<CClientSocket>> Clients;

    //this is a vector of sockets that need to be removed
    //when socket reading thread will encounter disconnection message
    //it will call the callback provided by server and the callback
    //will add this client to Work vector, to remove it later
    std::vector<CClientSocket*> Work;

    bool StopWorkThread = false;
    std::thread WorkThread;

    void _ProcessClientInput(uint8_t* buf, size_t bytesAmount);
public:
    CServer();
    ~CServer();

};