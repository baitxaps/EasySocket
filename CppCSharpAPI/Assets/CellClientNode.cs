using UnityEngine;
using System;

public class CellClientNode : CellTcpClient
{

    public string IP = "192.168.0.107";
    public short PORT = 4567;
    // Use this for initialization
    void Start()
    {
        this.Create();
        this.Connect(IP, PORT);
        Debug.Log(IP);
        Debug.Log(PORT);
    }

    // Update is called once per frame
    void Update()
    {
        this.OnRun();
        CellWriteStream s = new CellWriteStream(256);
        // CellSendStream s = new CellSendStream(256);
        s.setNetCmd(NetCMD.LOGOUT);
        s.WriteInt8(1);
        s.WriteInt16(2);
        s.WriteInt32(3);
        s.WriteFloat(4.5f);
        s.WriteDouble(6.7);
        s.WriteString("引擎 client...");
        s.WriteString("hello 引擎...");
        int[] b = { 1, 2, 3, 4, 5 };
        s.WriteInt32s(b);
        s.finsh();
        this.SendData(s);
    }

    void OnDestroy()
    {
        this.Close();
    }

    public override void OnNetMsgBytes(IntPtr data, int len)
    {
        // CellRecvStream r = new CellRecvStream(data, len);
        CellReadStream r = new CellReadStream(data, len);
        // msg length
        Debug.Log(r.ReadUInt16());
        // msg Type
        Debug.Log(r.ReadNetCmd());

        Debug.Log(r.ReadInt8());
        Debug.Log(r.ReadInt16());
        Debug.Log(r.ReadInt32());
        Debug.Log(r.ReadFloat());
        Debug.Log(r.ReadDouble());
        Debug.Log(r.ReadString());
        Debug.Log(r.ReadString());

        Int32[] arr = r.ReadInt32s();
        for (int n = 0; n < arr.Length; n++)
        {
            Debug.Log(arr[n]);
        }
    }
}
