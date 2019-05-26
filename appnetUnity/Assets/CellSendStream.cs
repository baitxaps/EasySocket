using System.Collections;
using System.Collections.Generic;
using System;
using System.Text;

public class CellSendStream {

    private List<byte> byteList = null;
    public CellSendStream(int nSize = 128)
    {
        byteList = new List<byte>(nSize);
    }

    public byte[] Array
    {
        get
        {
            return byteList.ToArray();
        }
    }

    public int Size
    {
        get
        {
            return byteList.Count;
        }
    }

    public void setNetCmd(NetCMD cmd)
    {
        WriteUInt16((UInt16)cmd);
    }

    public void finsh()
    {
        if(byteList.Count > UInt16.MaxValue)
        {
            // Error ,分包传送
        }

        UInt16 len = (UInt16)byteList.Count;
        len += 2; // 插入一个Uint16后增加2字节
        byteList.InsertRange(0, BitConverter.GetBytes(len));
    }

    public void Write(byte[] data)
    {
        byteList.AddRange(data);
    }

    // int
    public void WriteInt8(sbyte n)
    {
        byteList.Add((byte)n);
    }

    public void WriteInt16(Int16 n)
    {
        byteList.AddRange(BitConverter.GetBytes(n));
    }

    public void WriteInt32(Int32 n)
    {
        byteList.AddRange(BitConverter.GetBytes(n));
    }

    public void WriteInt64(Int64 n)
    {
        byte[] data = BitConverter.GetBytes(n);
        byteList.AddRange(data);
    }

    // unit 
    public void WriteUInt8(byte n)
    {
        byteList.Add(n);
    }

    public void WriteUInt16(UInt16 n)
    {
        byteList.AddRange(BitConverter.GetBytes(n));
    }

    public void WriteUInt32(UInt32 n)
    {
        byteList.AddRange(BitConverter.GetBytes(n));
    }

    public void WriteUInt64(UInt64 n)
    {
        byte[] data = BitConverter.GetBytes(n);
        byteList.AddRange(data);
    }

    // float
    public void WriteFloat(float n)
    {
        byteList.AddRange(BitConverter.GetBytes(n));
    }

    public void WriteDouble(double n)
    {
        byteList.AddRange(BitConverter.GetBytes(n));
    }

    //  string
    public void WriteString(string s)
    {
        byte[] buffer = Encoding.UTF8.GetBytes(s);
        WriteUInt32((UInt32)buffer.Length + 1);
        Write(buffer);
        WriteUInt8(0);
    }

    // array
    public void WriteStrings(byte[] data)
    {
        WriteUInt32((UInt32)data.Length + 1);
        Write(data);
    }

    public void WriteInts(int[] data)
    {
        WriteUInt32((UInt32)data.Length);
        for(int n = 0;n < data.Length;n++)
        {
            WriteInt32(data[n]);
        }
    }
}
