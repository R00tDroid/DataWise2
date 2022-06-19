using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using UnityEngine;

public class AnalyticsSession
{
    public enum ERecordingMode
    {
        Buffered,
        Streamed
    };

    public static AnalyticsSession Begin(string storage, string address, int port = 5438, ERecordingMode mode = ERecordingMode.Streamed)
    {
        AnalyticsSession session = new AnalyticsSession();
        session.session_id_ = DataWiseImport.BeginSession(storage, address, port, (int)mode);
        return session;
    }

    public void End()
    {
        DataWiseImport.EndSession(session_id_);
    }

    public void Cleanup()
    {
        DataWiseImport.CleanupSession(session_id_);
    }

    public bool HasFinished()
    {
        return DataWiseImport.HasSessionFinished(session_id_);
    }

    public void ReportEvent(AnalyticsPacket packet)
    {
        FieldInfo[] base_properties = typeof(AnalyticsPacket).GetFields();

        int builder = DataWiseImport.CreatePacketBuilder(session_id_, packet.PacketName);

        foreach (FieldInfo property in packet.GetType().GetFields())
        {
            bool found = false;
            foreach (FieldInfo base_prop in base_properties)
            {
                if (base_prop.Name.Equals(property.Name))
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                if (property.FieldType == typeof(int))
                {
                    DataWiseImport.AddInt(builder, (int)property.GetValue(packet));
                }
                else if (property.FieldType == typeof(float))
                {
                    DataWiseImport.AddFloat(builder, (float)property.GetValue(packet));
                }
                else if (property.FieldType == typeof(string))
                {
                    DataWiseImport.AddString(builder, (string)property.GetValue(packet));
                }
                else if (property.FieldType == typeof(AnalyticsPacket.TimeStamp))
                {
                    DataWiseImport.AddTime(builder);
                }
                else if (property.FieldType == typeof(Vector2))
                {
                    DataWiseImport.AddFloat(builder, ((Vector2)property.GetValue(packet)).x);
                    DataWiseImport.AddFloat(builder, ((Vector2)property.GetValue(packet)).y);
                }
                else if (property.FieldType == typeof(Vector3))
                {
                    DataWiseImport.AddFloat(builder, ((Vector3)property.GetValue(packet)).x);
                    DataWiseImport.AddFloat(builder, ((Vector3)property.GetValue(packet)).y);
                    DataWiseImport.AddFloat(builder, ((Vector3)property.GetValue(packet)).z);
                }
                else if (property.FieldType == typeof(Vector4))
                {
                    DataWiseImport.AddFloat(builder, ((Vector4)property.GetValue(packet)).x);
                    DataWiseImport.AddFloat(builder, ((Vector4)property.GetValue(packet)).y);
                    DataWiseImport.AddFloat(builder, ((Vector4)property.GetValue(packet)).z);
                    DataWiseImport.AddFloat(builder, ((Vector4)property.GetValue(packet)).w);
                }
                else
                {
                    Debug.LogError("Use of not supported packet member type: " + property.FieldType.FullName);
                }
            }
        }

        DataWiseImport.SendPacket(builder);
    }

    public void SetMeta(string name, int value) 
    {
        DataWiseImport.SetMetaInt(session_id_, name, value);
    }

    public void SetMeta(string name, float value)
    {
        DataWiseImport.SetMetaFloat(session_id_, name, value);
    }

    public void SetMeta(string name, string value)
    {
        DataWiseImport.SetMetaString(session_id_, name, value);
    }

    private int session_id_;
}
