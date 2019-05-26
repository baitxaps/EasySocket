using System.Collections;
using System.Collections.Generic;
using System;
using System.Text;

public class CellSendStream {

    private List<byte> _byteList = null;
    public CellSendStream(int nSize = 128)
    {
        _byteList = new List<byte>(nSize);
    }

    public byte[] Array
    {
        get
        {
            return _byteList.ToArray();
        }
    }

    public int Size
    {
        get
        {
            return _byteList.Count;
        }
    }

    public void setNetCmd(NetCMD cmd)
    {
        WriteUInt16((UInt16)cmd);
    }

    public void finsh()
    {
        if(_byteList.Count > UInt16.MaxValue)
        {
            // Error ,分包传送
        }

        UInt16 len = (UInt16)_byteList.Count;
        len += 2; // 插入一个Uint16后增加2字节
        _byteList.InsertRange(0, BitConverter.GetBytes(len));
    }

    public void Write(byte[] data)
    {
        _byteList.AddRange(data);
    }

    // int
    public void WriteInt8(sbyte n)
    {
        _byteList.Add((byte)n);
    }

    public void WriteInt16(Int16 n)
    {
        _byteList.AddRange(BitConverter.GetBytes(n));
    }

    public void WriteInt32(Int32 n)
    {
        _byteList.AddRange(BitConverter.GetBytes(n));
    }

    public void WriteInt64(Int64 n)
    {
        byte[] data = BitConverter.GetBytes(n);
        _byteList.AddRange(data);
    }

    // uint 
    public void WriteUInt8(byte n)
    {
        _byteList.Add(n);
    }

    public void WriteUInt16(UInt16 n)
    {
        _byteList.AddRange(BitConverter.GetBytes(n));
    }

    public void WriteUInt32(UInt32 n)
    {
        _byteList.AddRange(BitConverter.GetBytes(n));
    }

    public void WriteUInt64(UInt64 n)
    {
        byte[] data = BitConverter.GetBytes(n);
        _byteList.AddRange(data);
    }

    // float
    public void WriteFloat(float n)
    {
        _byteList.AddRange(BitConverter.GetBytes(n));
    }

    public void WriteDouble(double n)
    {
        _byteList.AddRange(BitConverter.GetBytes(n));
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

    public void WriteInt32s(Int32[] data)
    {
        WriteUInt32((UInt32)data.Length);
        for(int n = 0;n < data.Length;n++)
        {
            WriteInt32(data[n]);
        }
    }
}
