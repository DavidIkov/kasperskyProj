#pragma once
#include"ClientSocket.hpp"

class CClient :public CClientSocket {
private:
    uint8_t BytesLeftInUnsignedBuf = sizeof(unsigned);
    uint8_t ReadUnsignedBuf[sizeof(unsigned)];
protected:
    virtual void OnRead(uint8_t* buf, size_t bytesAmount) override;
};