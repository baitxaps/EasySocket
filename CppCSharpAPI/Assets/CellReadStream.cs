using System;
using System.Text;
using System.Runtime.InteropServices;

public class CellReadStream
{

#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("CppNetworkDll")]
#endif
    private static extern IntPtr CellReadStream_Create(IntPtr data, int le);

    // int
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("CppNetworkDll")]
#endif
    private static extern sbyte CellReadStream_ReadInt8(IntPtr cppStreamObj);

#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("CppNetworkDll")]
#endif
    private static extern Int16 CellReadStream_ReadInt16(IntPtr cppStreamObj);

#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("CppNetworkDll")]
#endif
    private static extern Int32 CellReadStream_ReadInt32(IntPtr cppStreamObj);

#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("CppNetworkDll")]
#endif
    private static extern Int64 CellReadStream_ReadInt64(IntPtr cppStreamObj);

    // uint
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("CppNetworkDll")]
#endif
    private static extern byte CellReadStream_ReadUInt8(IntPtr cppStreamObj);

#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("CppNetworkDll")]
#endif
    private static extern UInt16 CellReadStream_ReadUInt16(IntPtr cppStreamObj);

#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("CppNetworkDll")]
#endif
    private static extern UInt32 CellReadStream_ReadUInt32(IntPtr cppStreamObj);

#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("CppNetworkDll")]
#endif
    private static extern UInt64 CellReadStream_ReadUInt64(IntPtr cppStreamObj);

    // float
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("CppNetworkDll")]
#endif
    private static extern float CellReadStream_ReadFloat(IntPtr cppStreamObj);

#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("CppNetworkDll")]
#endif
    private static extern double CellReadStream_ReadDouble(IntPtr cppStreamObj);

#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("CppNetworkDll")]
#endif
    private static extern UInt32 CellReadStream_OnlyReadUInt32(IntPtr cppStreamObj);

    // string
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("CppNetworkDll")]
#endif
    private static extern bool CellReadStream_ReadString(IntPtr cppStreamObj, StringBuilder buffer, int len);

    // release
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("CppNetworkDll")]
#endif
    private static extern void CellReadStream_Release(IntPtr cppStreamObj);

    //------------------------------------------------------------------------------------------
    private IntPtr cppStreamObj = IntPtr.Zero;
    /// <summary>
    /// /
    /// </summary>
    /// <param name="data">C++消息回调传入的消息数据 指针</param>
    /// <param name="len">数据字节长度</param>
    public CellReadStream(IntPtr data, int len)
    {
        cppStreamObj = CellReadStream_Create(data, len);
    }

    public NetCMD ReadNetCmd()
    {
        return (NetCMD)ReadUInt16();
    }

    // int
    public sbyte ReadInt8(sbyte n = 0)
    {
        return CellReadStream_ReadInt8(cppStreamObj);
    }

    public Int16 ReadInt16(Int16 n = 0)
    {
        return CellReadStream_ReadInt16(cppStreamObj);
    }

    public Int32 ReadInt32(Int32 n = 0)
    {
        return CellReadStream_ReadInt32(cppStreamObj);
    }

    public Int64 ReadInt64(Int64 n = 0)
    {
        return CellReadStream_ReadInt64(cppStreamObj);
    }

    // uint
    public byte ReadUInt8(byte n = 0)
    {
        return CellReadStream_ReadUInt8(cppStreamObj);
    }

    public UInt16 ReadUInt16(UInt16 n = 0)
    {
        return CellReadStream_ReadUInt16(cppStreamObj);
    }

    public UInt32 ReadUInt32(UInt32 n = 0)
    {
        return CellReadStream_ReadUInt32(cppStreamObj);
    }

    public UInt64 ReadUInt64(UInt64 n = 0)
    {
        return CellReadStream_ReadUInt64(cppStreamObj);
    }

    // float
    public float ReadFloat(float n = 0)
    {
        return CellReadStream_ReadFloat(cppStreamObj);
    }


    public double ReadDouble(double n = 0)
    {
        return CellReadStream_ReadDouble(cppStreamObj);
    }


    public UInt32 OnlyReadUInt32(UInt32 n = 0)
    {
        return CellReadStream_OnlyReadUInt32(cppStreamObj);
    }


    // string
    public string ReadString()
    {
        Int32 len = (Int32)ReadUInt32();
        byte[] buffer = new byte[len];
        for (int n = 0; n < buffer.Length; n++)
        {
            buffer[n] = ReadUInt8();
        }
        return Encoding.UTF8.GetString(buffer, 0, len);

        //StringBuilder sb = new StringBuilder(len);
        //CellReadStream_ReadString(cppStreamObj, sb, len);
        //return sb.ToString();
    }

    // array
    public Int32[] ReadInt32s()
    {
        int len = ReadInt32();
        if (len < 1)
        {
            // Error 处理  (try catch)
            return null;
        }

        Int32[] data = new Int32[len];
        for (int n = 0; n < len; n++)
        {
            data[n] = ReadInt32();
        }
        return data;
    }

    public  void Release()
    {
        CellReadStream_Release(cppStreamObj);
    }
}