using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TestPacket : AnalyticsPacket
{
    public TestPacket(float t, string ts, int a)
    {
        PacketName = "TestPacket";
        test = t;
        test_string = ts;
        a_test = a;
    }

    public TimeStamp time;
    public string test_string;
    public int a_test;
    public float test;
    public Vector2 vector_test;
}
