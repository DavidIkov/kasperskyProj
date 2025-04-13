#pragma once
#include"ClientSocket.hpp"

class CClient :public CClientSocket {
public:
    using CClientSocket::CClientSocket;
protected:
    virtual size_t OnRead(size_t bytesReserved, uint8_t* buf, size_t newBytes) override final;
};