#include "EasyTcpServer.hpp"
#include"CellMsgStream.hpp"

// !<<<NOTE>>>
// formater IDE
// ctrl+shift+f:find;ctrl+k,ctrl+f:Format alignment  ctrl+M :layer

// complier tools
// g++ -std=c++11 -pthread server.cpp -o server

// common libiary
// properties -> c/c++ ->general -> additoinal include directories

bool g_bRun = true;
void cmdThread()
{
    while (true)
    {
        char cmdBuf[256] = {};
        scanf("%s", cmdBuf);
        if (0 == strcmp(cmdBuf, "exit"))
        {
            g_bRun = false;
            CellLog::Info("退出cmdThread线程\n");
            break;
        }
        else {
            CellLog::Info("不支持的命令。\n");
        }
    }
}

class MyServer : public EasyTcpServer
{
public:
    //只会被一个线程触发 安全
    virtual void OnNetJoin(CellClient* pClient)
    {
        EasyTcpServer::OnNetJoin(pClient);
        // CellLog::Info("client<%d> join\n", pClient->sockfd());
    }
    //cellServer 多个线程触发 不安全
    virtual void OnNetLeave(CellClient* pClient)
    {
        EasyTcpServer::OnNetLeave(pClient);
        // CellLog::Info("client<%d> leave\n", pClient->sockfd());
    }
    
    //cellServer 多个线程触发 不安全
    virtual void OnNetMsg(CellServer* pServer, CellClient *pClient, netmsg_DataHeader* header)
    {
        EasyTcpServer::OnNetMsg(pServer,pClient,header);
        switch (header->cmd)
        {
            case CMD_LOGIN:
            {
                pClient->resetDTHeart();
                netmsg_Login* login = (netmsg_Login*)header;
                //CellLog::Info("收到客户端<Socket=%d>请求：CMD_LOGIN,数据长度：%d,userName=%s PassWord=%s\n",pClient->sockfd(), login->dataLength, login->userName, login->PassWord);
                //忽略判断用户密码是否正确的过程
                netmsg_LoginResult ret;
                if (SOCKET_ERROR == pClient->SendData(&ret))
                {
                    CellLog::Info("<Socket=%d> Send Full \n",pClient->sockfd());
                    // cache is full
                }
                
                //netmsg_LoginResult *ret = new netmsg_LoginResult();
                //auto ret = std::make_shared<netmsg_LoginResult>();
                //pCellServer->addSendTask(pClient,(DataHeaderPtr)ret);
            }
                break;
            case CMD_LOGOUT:
            {
                pClient->resetDTHeart();
                CellReadStream r(header);
                //读取消息长度
                r.ReadInt16();
                //读取消息命令
                r.getNetCmd();
                auto n1 = r.ReadInt8();
                auto n2 = r.ReadInt16();
                auto n3 = r.ReadInt32();
                auto n4 = r.ReadFloat();
                auto n5 = r.ReadDouble();
                uint32_t n = 0;
                r.onlyRead(n);
                char name[32] = {};
                auto n6 = r.ReadArray(name, 32);
                char pwd[32] = {};
                auto n7 = r.ReadArray(pwd, 32);
                int arr[10] = {};
                auto n8 = r.ReadArray(arr, 10);
                ///
                CellWriteStream s(128);
                s.setNetCmd(CMD_LOGOUT_RESULT);
                s.WriteInt8(n1);
                s.WriteInt16(n2);
                s.WriteInt32(n3);
                s.WriteFloat(n4);
                s.WriteDouble(n5);
                s.WriteArray(name, strlen(name));
                s.WriteArray(pwd, strlen(pwd));
                s.WriteArray(arr, n8);
                s.finsh();
                pClient->SendData(s.data(), s.length());
            }
                break;
                
            case CMD_C2S_HEART:
            {
                pClient->resetDTHeart();
                //    netmsg_c2s_Heart ret;
                //    pClient->SendData(&ret);
                
                //    auto ret = std::make_shared<netmsg_c2s_Heart>();
                //    pCellServer->addSendTask(pClient, (DataHeaderPtr)ret);
                
                netmsg_s2c_Heart ret;
                pClient->SendData(&ret);
            }
                break;
                
            default:
            {
                CellLog::Info("recv <socket=%d> undefine msgType,dataLen：%d\n", pClient->sockfd(), header->dataLength);
                netmsg_DataHeader ret;
                //    pClient->SendData(&ret);
            }
                break;
        }
    }
};

int main()
{
    CellLog::Instance().setLogPath("serverLog.txt","w");
    MyServer server;
    server.InitSocket();
    server.Bind(nullptr, 4567);
    server.Listen(5);
    server.Start(4);
    
    while (true)
    {
        char cmdBuf[256] = {};
        scanf("%s", cmdBuf);
        if (0 == strcmp(cmdBuf, "exit"))
        {
            server.Close();
            break;
        }
        else {
            CellLog::Info("undefine cmd\n");
        }
    }
    
    CellLog::Info("exit.\n");
    getchar();
    return 0;
}
