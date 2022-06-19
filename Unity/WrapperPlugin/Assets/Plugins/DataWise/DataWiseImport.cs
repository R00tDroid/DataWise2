using System;
using System.Runtime.InteropServices;
using UnityEngine;

public class DataWiseImport
{
    [DllImport("core", EntryPoint = "BeginSession")]
    public static extern int BeginSession(string storage, string address, int port, int mode);

    [DllImport("core", EntryPoint = "EndSession")]
    public static extern void EndSession(int session);

    [DllImport("core", EntryPoint = "CleanupSession")]
    public static extern void CleanupSession(int session);

    [DllImport("core", EntryPoint = "HasSessionFinished")]
    public static extern bool HasSessionFinished(int session);

    [DllImport("core", EntryPoint = "ReportEvent")]
    public static extern void ReportEvent(int session, string data);

    [DllImport("core", EntryPoint = "SetLogCallback")]
    public static extern void SetLogCallback(IntPtr callback);

    [DllImport("core", EntryPoint = "SetMetaInt")]
    public static extern void SetMetaInt(int session, string name, int value);

    [DllImport("core", EntryPoint = "SetMetaFloat")]
    public static extern void SetMetaFloat(int session, string name, float value);

    [DllImport("core", EntryPoint = "SetMetaString")]
    public static extern void SetMetaString(int session, string name, string value);


    [DllImport("core", EntryPoint = "CreatePacketBuilder")]
    public static extern int CreatePacketBuilder(int session, string packet_name);

    [DllImport("core", EntryPoint = "AddInt")]
    public static extern void AddInt(int builder, int value);

    [DllImport("core", EntryPoint = "AddFloat")]
    public static extern void AddFloat(int builder, float value);

    [DllImport("core", EntryPoint = "AddString")]
    public static extern void AddString(int builder, string value);

    [DllImport("core", EntryPoint = "AddTime")]
    public static extern void AddTime(int builder);

    [DllImport("core", EntryPoint = "SendPacket")]
    public static extern void SendPacket(int builder);




    public delegate void LoggingDelegate(int type, string text);

    public static void EnableLogging(bool Enable)
    {
        if (Enable)
        {
            LoggingDelegate callback = new LoggingDelegate(DataWiseImport.LoggingCallback);
            SetLogCallback(Marshal.GetFunctionPointerForDelegate(callback));
        }
        else
        {
            SetLogCallback(IntPtr.Zero);
        }
    }

    public static void LoggingCallback(int type, string text)
    {
        switch (type)
        {
            case 0: { Debug.Log(text); break; }
            case 1: { Debug.LogWarning(text); break; }
            case 2: { Debug.LogError(text); break; }
        }
    }
}
