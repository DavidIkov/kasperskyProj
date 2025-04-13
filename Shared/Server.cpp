#include"Server.hpp"
#include<algorithm>


void CServer::OnAccept(std::shared_ptr<CClientSocket> ptr) {
    CServerSocket::OnAccept(ptr);
    Clients.push_back(ptr);
    auto data = ptr->GetRemoteEndpoint();
    printf("client joined from %s:%i\n", data.IP.c_str(), data.Port);
}
CServer::CServer() :WorkThread([this]() {
    while(1) {
        std::unique_lock LG(Mutex);
        if (StopWorkThread) return;
        if (Work.size() == 0)
            CV.wait(LG, [&]()->bool {return StopWorkThread || Work.size() != 0;});
        else {
            for (auto ptr : Work) {
                Clients.erase(std::find_if(Clients.begin(), Clients.end(),
                    [&](std::shared_ptr<CClientSocket>& _p)->bool { return _p.get() == ptr;}));
                printf("removed client\n");
            }
            Work.clear();
        }
    }
    }) {

}
CServer::~CServer() {
    std::unique_lock LG(Mutex);
    for (auto& ptr : Clients) {
        ptr->Disconnect();
    }
    CServerSocket::Close();
    StopWorkThread = true;
    CV.notify_all();
    LG.unlock(); WorkThread.join(); LG.lock();

}
