#include"ProcessServer.hpp"
#include<unordered_set>
#include"httpparser/include/response.h"
#include"httpparser/include/httpresponseparser.h"
#include"nlohmannjson/json.hpp"

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
    resStr.resize(resStr.size() - 1);
    return resStr;
}

size_t CProcessServer::CClientSocketForProcServer::OnRead(size_t bytesReserved, uint8_t* buf, size_t newBytes) {
    return StandartParser(bytesReserved, buf, newBytes, [this](uint8_t* buf, size_t len) {
        httpparser::Response clientResp;
        httpparser::HttpResponseParser parser;

        httpparser::HttpResponseParser::ParseResult res =
            parser.parse(clientResp, (const char*)buf, (const char*)(buf + len));
        if (res == httpparser::HttpResponseParser::ParsingCompleted) {

            nlohmann::json clientJson = nlohmann::json::parse(clientResp.content);
            std::string clientInput = clientJson["text"];
            
            unsigned errorsFound = 0;
            for (unsigned i = 0;i < clientInput.size();i++)
                if (clientInput[i] < 32 || clientInput[i]>126) {
                    clientInput[i] = '?';
                    errorsFound++;
                }
            {
                std::string json = "{\"bytes\":\"" + std::to_string(clientInput.size()) + "\"}";
                std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                    "Content-Length: " + std::to_string(json.size()) + "\r\n\r\n" + json;
                unsigned num = response.size(); Send(&num, sizeof(unsigned));
                Send(response.data(), response.size());
            }

            printf("server received a message: \"%.*s\"", (int)clientInput.size(), clientInput.data());
            errorsFound ? printf(" and found %u errors\n", errorsFound) : printf(" with no errors\n");

            std::string resStr = RemoveDuplicateWords(std::string(clientInput.begin(), clientInput.end()));
            {
                std::string json = "{\"text\":\"" + resStr + "\"}";
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