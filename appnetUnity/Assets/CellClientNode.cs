using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using AOT;
using System;

public class CellClientNode : CellTcpClient
{
    //[DllImport("CppNet100")]
    //private static extern int Add(int a, int b);

    //public delegate void CallBack1(string s);

    //[DllImport("CppNet100")]
    //private static extern void TestCall1(string s, CallBack1 cb);

    //[MonoPInvokeCallback(typeof(CallBack1))]
    //public static  void CallBackFun1(string s)
    //{
    //    Debug.Log(s);
    //}
    public string IP = "192.168.0.107";
    public short PORT = 4567;
    // Use this for initialization
    void Start()
    {
        //Debug.Log(Add(4,5));
        //TestCall1("Hei Hei", CallBackFun1);

        this.Create();
        this.Connect(IP, PORT);

        CellSendStream s = new CellSendStream(64);
        s.setNetCmd(NetCMD.LOGOUT);
        s.WriteInt8(1);
        s.WriteInt16(2);
        s.WriteInt32(3);
        s.WriteFloat(4.5f);
        s.WriteDouble(6.7);
        s.WriteString("client");
        s.WriteString("hello");
        int [] b = { 1, 2, 3, 4, 5 };
        s.WriteInts(b);
        s.finsh();
        this.SendData(s.Array);
    }

    // Update is called once per frame
    void Update()
    {
        this.OnRun();
    }

    void OnDestroy()
    {
        this.Close();
    }

    public override void OnNetMsgBytes(byte[] data)
    {

    }
}
