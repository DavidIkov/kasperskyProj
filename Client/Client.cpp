#include"Client.hpp"
#include<cstring>

void CClient::OnRead(uint8_t* buf, size_t bytesAmount) {
    while (bytesAmount) {
        size_t bytesToRead = std::min(bytesAmount, (size_t)BytesLeftInUnsignedBuf);
        std::memcpy(ReadUnsignedBuf + (sizeof(ReadUnsignedBuf) - BytesLeftInUnsignedBuf), buf, bytesToRead);
        buf += bytesToRead; bytesAmount -= bytesToRead;
        if (BytesLeftInUnsignedBuf == 0) printf("server received %u bytes!\n", *((unsigned*)ReadUnsignedBuf));
    }
}