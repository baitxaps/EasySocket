using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System;
using System.Text;

public class CellWriteStream {

    [DllImport("CppNetworkDll")]
    private static extern IntPtr CellWriteStream_Create(int nSize);

    // int
    [DllImport("CppNetworkDll")]
    private static extern bool CellWriteStream_WriteInt8(IntPtr cppStreamObj,sbyte n);

    [DllImport("CppNetworkDll")]
    private static extern bool CellWriteStream_WriteInt16(IntPtr cppStreamObj, Int16 n);

    [DllImport("CppNetworkDll")]
    private static extern bool CellWriteStream_WriteInt32(IntPtr cppStreamObj, Int32 n);

    [DllImport("CppNetworkDll")]
    private static extern bool CellWriteStream_WriteInt64(IntPtr cppStreamObj, Int64 n);

    //uint
    [DllImport("CppNetworkDll")]
    private static extern bool CellWriteStream_WriteUInt8(IntPtr cppStreamObj, byte n);

    [DllImport("CppNetworkDll")]
    private static extern bool CellWriteStream_WriteUInt16(IntPtr cppStreamObj, UInt16 n);

    [DllImport("CppNetworkDll")]
    private static extern bool CellWriteStream_WriteUInt32(IntPtr cppStreamObj, UInt32 n);

    [DllImport("CppNetworkDll")]
    private static extern bool CellWriteStream_WriteUInt64(IntPtr cppStreamObj, UInt64 n);

    // float
    [DllImport("CppNetworkDll")]
    private static extern bool CellWriteStream_WriteFloat(IntPtr cppStreamObj, float n);

    [DllImport("CppNetworkDll")]
    private static extern bool CellWriteStream_WriteDouble(IntPtr cppStreamObj, double n);

    // string
    [DllImport("CppNetworkDll")]
    private static extern bool CellWriteStream_WriteString(IntPtr cppStreamObj, string s);
    //------------------------------------------------------------------------------------------

    private IntPtr cppStreamObj = IntPtr.Zero;
    public CellWriteStream(int nSize = 128)
    {
        cppStreamObj = CellWriteStream_Create(nSize);
    }

    public IntPtr cppObj
    {
        get
        {
            return cppStreamObj;
        }
    }

    public void setNetCmd(NetCMD cmd)
    {
        WriteUInt16((UInt16)cmd);
    }

    public void finsh()
    {
  
    }

    // int
    public void WriteInt8(sbyte n)
    {
        CellWriteStream_WriteInt8(cppStreamObj, n);
    }

    public void WriteInt16(Int16 n)
    {
        CellWriteStream_WriteInt16(cppStreamObj, n);
    }

    public void WriteInt32(Int32 n)
    {
        CellWriteStream_WriteInt32(cppStreamObj, n);
    }

    public void WriteInt64(Int64 n)
    {
        CellWriteStream_WriteInt64(cppStreamObj, n);
    }

    // uint 
    public void WriteUInt8(byte n)
    {
        CellWriteStream_WriteUInt8(cppStreamObj, n);
    }

    public void WriteUInt16(UInt16 n)
    {
        CellWriteStream_WriteUInt16(cppStreamObj, n);
    }

    public void WriteUInt32(UInt32 n)
    {
        CellWriteStream_WriteUInt32(cppStreamObj, n);
    }

    public void WriteUInt64(UInt64 n)
    {
        CellWriteStream_WriteUInt64(cppStreamObj, n);
    }

    // float
    public void WriteFloat(float n)
    {
        CellWriteStream_WriteFloat(cppStreamObj, n);
    }

    public void WriteDouble(double n)
    {
        CellWriteStream_WriteDouble(cppStreamObj, n);
    }

    //  string
    public void WriteString(string s)
    {
        byte[] buffer = Encoding.UTF8.GetBytes(s);
        WriteUInt32((UInt32)buffer.Length + 1);
        for(int n = 0; n<buffer.Length; n++)
        {
            WriteUInt8(buffer[n]);
        }
        WriteUInt8(0);
    }

    // array
    public void WriteByte(byte[] data)
    {
        WriteUInt32((UInt32)data.Length + 1);
        for (int n = 0; n < data.Length; n++)
        {
            WriteUInt8(data[n]);
        }
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
