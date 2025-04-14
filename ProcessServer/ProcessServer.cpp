#include"ProcessServer.hpp"
#include<unordered_set>
#include"httpparser/include/response.h"
#include"httpparser/include/httpresponseparser.h"

static std::string RemoveDuplicateWords(std::string const& str) {
    std::string resStr; resStr.reserve(str.size());
    std::unordered_set<std::string> words;
    size_t prevWordBeg = 0;
    while (prevWordBeg != str.size() + 1) {
        size_t end = str.find_first_of(' ', prevWordBeg);
        if (end == str.npos) end = str.size();
        auto subStr = str.substr(prevWordBeg, end - prevWordBeg);
        if (!words.count(subStr)) {
            resStr += subStr + ' ';
            words.emplace(std::move(subStr));
        }
        prevWordBeg = end + 1;
    }
    if (resStr.size()) resStr[resStr.size() - 1] = 0;
    return resStr;
}

size_t CProcessServer::CClientSocketForProcServer::OnRead(size_t bytesReserved, uint8_t* buf, size_t newBytes) {
    return StandartParser(bytesReserved, buf, newBytes, [this](uint8_t* buf, size_t len) {
        httpparser::Response clientResp;
        httpparser::HttpResponseParser parser;

        httpparser::HttpResponseParser::ParseResult res =
            parser.parse(clientResp, (const char*)buf, (const char*)(buf + len));
        if (res == httpparser::HttpResponseParser::ParsingCompleted) {
            unsigned errorsFound = 0;
            for (unsigned i = 0;i < clientResp.content.size();i++)
                if (clientResp.content[i] < 32 || clientResp.content[i]>126) {
                    clientResp.content[i] = '?';
                    errorsFound++;
                }
            {
                std::string json = std::to_string(clientResp.content.size());
                std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                    "Content-Length: " + std::to_string(json.size()) + "\r\n\r\n" + json;
                unsigned num = response.size(); Send(&num, sizeof(unsigned));
                Send(response.data(), response.size());
            }

            printf("server received a message: \"%.*s\"", (int)clientResp.content.size(), clientResp.content.data());
            errorsFound ? printf(" and found %u errors\n", errorsFound) : printf(" with no errors\n");

            std::string resStr = RemoveDuplicateWords(std::string(clientResp.content.begin(), clientResp.content.end()));
            {
                std::string json = resStr;
                std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                    "Content-Length: " + std::to_string(json.size()) + "\r\n\r\n" + json;
                unsigned num = response.size(); ((CProcessServer*)Serv)->Send(&num, sizeof(unsigned));
                ((CProcessServer*)Serv)->Send(response.data(), response.size());
            }

        }
        else
            fprintf(stderr, "cant parse http, it contains error\n");

        });
}