#include"ProcessServer.hpp"
#include<unordered_set>

size_t CProcessServer::CClientSocketForProcServer::OnRead(size_t bytesReserved, uint8_t* buf, size_t newBytes) {
    size_t lastZero = 0;
    for (size_t i = bytesReserved;i < bytesReserved + newBytes;i++)
        if (buf[i] == 0) {
            unsigned bytes = i - lastZero;
            unsigned errorsFound = 0;
            for (unsigned j = lastZero;j < lastZero + bytes;j++)
                if (buf[j] < 32 || buf[j]>126) {
                    buf[j] = '?';
                    errorsFound++;
                }
            Send(&bytes, sizeof(unsigned));
            
            printf("server received a message: \"%.*s\"", (int)bytes, buf + lastZero);
            errorsFound ? printf(" and found %u errors\n", errorsFound) : printf(" with no errors\n");
            
            {//removing duplicates of words
                std::string str(buf + lastZero, buf + lastZero + bytes);
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
                ((CProcessServer*)Serv)->Send(resStr.data(), resStr.size());//yes, no +1 for size since we explicitly placed 0 at last space
            }

            lastZero = i + 1;
        }
    return bytesReserved + newBytes - lastZero;
}