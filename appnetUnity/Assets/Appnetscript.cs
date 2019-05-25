using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using AOT;

public class Appnetscript : MonoBehaviour
{
    [DllImport("CppNetworkDll")]
    public static extern int Add(int a, int b);

    public delegate void CallBack1(string s);

    [DllImport("CppNetworkDll")]
    public static extern void TestCall(string str1, CallBack1 cb);

    [MonoPInvokeCallback(typeof(CallBack1))]
    public void CallBackFun1(string s)
    {
        Debug.Log(s);
    }

    // Start is called before the first frame update
    void Start()
    {
        Debug.Log(Add(5, 4));
        TestCall("hello,Call me", CallBackFun1);
    }

    // Update is called once per frame
    void Update()
    {

    }
}
