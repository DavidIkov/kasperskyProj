#pragma once
#include<string>
#include<cstdint>

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

