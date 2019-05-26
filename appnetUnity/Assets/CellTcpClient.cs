using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using System;
using AOT;

public class CellTcpClient : MonoBehaviour
{
    public delegate void OnNetMsgCallBack(IntPtr csObj, IntPtr data, int len);

    [MonoPInvokeCallback(typeof(OnNetMsgCallBack))]
    private static void OnNetMsgCallBack1(IntPtr csObj, IntPtr data, int len)
    {
        Debug.Log("OnNetMsgCallBack1:" + len);
    }

    [DllImport("CppNetworkDll")]
    private static extern IntPtr CellClient_Create(IntPtr csObj, OnNetMsgCallBack cb);

    [DllImport("CppNetworkDll")]
    private static extern bool CellClient_Connect(IntPtr cppClientObj, string ip, short port);

    [DllImport("CppNetworkDll")]
    private static extern bool CellClient_OnRun(IntPtr cppClientObj);

    [DllImport("CppNetworkDll")]
    private static extern void CellClient_Close(IntPtr cppClientObj);

    [DllImport("CppNetworkDll")]
    private static extern int CellClient_SendData(IntPtr cppClientObj, byte[] data, int len);

    ////////////
    private GCHandle _handleThis;
    // this对象的指针 在C++消息回调中传回
    IntPtr _csThisObj = IntPtr.Zero;
    //C++ NativeTCPClient 对象的指针
    IntPtr _cppClientObj = IntPtr.Zero;
    //
    public void Create()
    {
        _handleThis = GCHandle.Alloc(this);
        _csThisObj = GCHandle.ToIntPtr(_handleThis);
        _cppClientObj = CellClient_Create(_csThisObj, OnNetMsgCallBack1);
    }

    public bool Connect(string ip, short port)
    {
        if (_cppClientObj == IntPtr.Zero)
            return false;
        return CellClient_Connect(_cppClientObj, ip, port);
    }

    public bool OnRun()
    {
        Debug.Log("OnRun");
        if (_cppClientObj == IntPtr.Zero)
            return false;

        return CellClient_OnRun(_cppClientObj);
    }

    public void Close()
    {
        if (_cppClientObj == IntPtr.Zero)
            return;

        CellClient_Close(_cppClientObj);
        _cppClientObj = IntPtr.Zero;
        Debug.Log("Close");
    }

    public int SendData(byte[] data)
    {
        if (_cppClientObj == IntPtr.Zero)
            return 0;
        Debug.Log("SendData");
        return CellClient_SendData(_cppClientObj, data, data.Length);
    }

    ////////////

    // Use this for initialization
    void Start()
    {
        //this.Create();
        //this.Connect(IP, PORT);
    }

    // Update is called once per frame
    void Update()
    {

    }
}
