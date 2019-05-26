using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using AOT;
using System;

public class cSharpTest : CellTcpClient
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
    public string IP = "127.0.0.1";//192.168.0.107
    public short PORT = 4567;
    // Use this for initialization
    void Start()
    {
        //Debug.Log(Add(4,5));
        //TestCall1("Hei Hei", CallBackFun1);

        this.Create();
        this.Connect(IP, PORT);

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
}
