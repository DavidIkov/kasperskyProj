#pragma once
#include<string>
#include<cstdint>
#include<unordered_map>

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

std::string GetIPFromDomen(std::string_view const& domen);

template<typename T>
size_t StandartParser(size_t bytesReserved, uint8_t* buf, size_t newBytes, T&& func){

    size_t end = bytesReserved + newBytes;

    for (size_t i = 0;i < end;) {
        if (end - i < sizeof(unsigned)) return end - i;
        else {
            unsigned len = *((unsigned*)(buf + i));
            if (i + sizeof(unsigned) + len > end) return end - i;
            else {
                func((uint8_t*)(buf + i + sizeof(unsigned)), (size_t)len);
                i += sizeof(unsigned) + len;
            }
        }
    }
    return 0;
}