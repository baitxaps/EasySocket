//
//  ViewController.m
//  EasyTcpDemo
//
//  Created by William on 2019/6/3.
//  Copyright © 2019 技术研发-IOS. All rights reserved.
//

#import "ViewController.h"
#include"EasyTcpClient.hpp"
#include"CellMsgStream.hpp"
#include<iostream>
#include<functional>


class MyClient : public EasyTcpClient
{
public:
    //响应网络消息
    virtual void OnNetMsg(netmsg_DataHeader* header)
    {
        switch (header->cmd)
        {
            case CMD_LOGOUT_RESULT:
            {
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
                //CellLog::Info("<socket=%d> recv msgType：CMD_LOGOUT_RESULT\n", (int)_pClient->sockfd());
                printf("n1= %d,n2= %d,n3= %d,n4= %f,n5= %f, pwd = %s ,name = %s ", n1,n2,n3,n4,n5,pwd,name);
                printf(",arr =");
                for (int i = 0; i<5; i++) {
                    printf("%d ", *(arr+i));
                }
                printf("\n");
            }
                break;
            case CMD_ERROR:
            {
                CellLog::Info("<socket=%d> recv msgType：CMD_ERROR\n", (int)_pClient->sockfd());
            }
                break;
            default:
            {
                CellLog::Info("error, <socket=%d> recv undefine msgType\n", (int)_pClient->sockfd());
            }
        }
    }
private:
    
};


/*
请先运行EasyTcpServer端，再把SocketExec函数中的Connect,改成要连接服务器的IP
*/
void SocketExec()
{
    CellWriteStream s(256);
    s.setNetCmd(CMD_LOGOUT);
    s.WriteInt8(1);
    s.WriteInt16(2);
    s.WriteInt32(3);
    s.WriteFloat(4.5f);
    s.WriteDouble(6.7);
    s.WriteString("socket engine...");
    char str[] = "client.";
    s.WriteArray(str, strlen(str));
    int arr[] = {1,2,3,4,5};
    s.WriteArray(arr, 5);
    s.finsh();
    MyClient client;
    client.Connect("127.0.0.1", 4567);
    
    while (client.OnRun())
    {
        client.SendData(s.data(), s.length());
        CellThread::Sleep(10);
    }
}


@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    SocketExec();
}



@end
