using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TestController : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        DataWiseImport.EnableLogging(true);

        session = AnalyticsSession.Begin("DW_Test", "127.0.0.1", 5438, AnalyticsSession.ERecordingMode.Streamed);

        session.ReportEvent(new TestPacket(.24f, "test", 9));

        session.SetMeta("Players", 5);
        session.SetMeta("Duration", 126.8f);

        session.End();
    }

    // Update is called once per frame
    void Update()
    {
        if (session != null)
        {
            if (session.HasFinished())
            {
                session.Cleanup();
                session = null;
            }
        }
    }

    private AnalyticsSession session;
}
