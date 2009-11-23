// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Timers;

public class StateChangerI : Test.StateChangerDisp_
{
    public StateChangerI(Ice.ObjectAdapter adapter)
    {
        _otherAdapter = adapter;
    }

    public override void hold(int milliSeconds, Ice.Current current)
    {
        if(milliSeconds <= 0)
        {
            _otherAdapter.hold();
            _otherAdapter.waitForHold();
        }
        else
        {
            Timer timer = new Timer();
            timer.AutoReset = false;
            timer.Enabled = true;
            timer.Interval = milliSeconds;
            timer.Elapsed += new ElapsedEventHandler(
                delegate(object source, ElapsedEventArgs e)
                {
                    _otherAdapter.hold();
                });
        }
    }

    public override void activate(int milliSeconds, Ice.Current current)
    {
        if(milliSeconds <= 0)
        {
            _otherAdapter.activate();
        }
        else
        {
            Timer timer = new Timer();
            timer.AutoReset = false;
            timer.Enabled = true;
            timer.Interval = milliSeconds;
            timer.Elapsed += new ElapsedEventHandler(
                delegate(object source, ElapsedEventArgs e)
                {
                    _otherAdapter.activate();
                });
        }
    }

    private Ice.ObjectAdapter _otherAdapter;
}
