#pragma once
#include"Server.hpp"

class CVisualServer :public CServer {
public:
    using CServer::CServer;
protected:
    class CClientSocketForVisualServer :public CClientSocketForServer {
    public:
        using CClientSocketForServer::CClientSocketForServer;
    protected:
        virtual size_t OnRead(size_t bytesReserved, uint8_t* buf, size_t newBytes) override final;
    };

    inline virtual std::shared_ptr<CClientSocket> ClientSocketFactory(int handle, size_t bufSize) override {
        return std::shared_ptr<CClientSocket>(new CClientSocketForVisualServer(this, handle, bufSize));
    };
};