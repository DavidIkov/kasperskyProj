#include"Client.hpp"
#include<cstring>
#include"httpparser/include/response.h"
#include"httpparser/include/httpresponseparser.h"


size_t CClient::OnRead(size_t bytesReserved, uint8_t* buf, size_t newBytes) {
    return StandartParser(bytesReserved, buf, newBytes, [this](uint8_t* buf, size_t len) {
        httpparser::Response response;
        httpparser::HttpResponseParser parser;

        httpparser::HttpResponseParser::ParseResult res =
            parser.parse(response, (const char*)buf, (const char*)(buf + len));
        if (res == httpparser::HttpResponseParser::ParsingCompleted)
            printf("server received \"%.*s\" bytes\n", (int)response.content.size(), response.content.data());
        else
            fprintf(stderr, "cant parse http, it contains error\n");
        });
}