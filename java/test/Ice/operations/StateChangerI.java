// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import test.Ice.operations.Test._StateChangerDisp;

class StateChangerI extends _StateChangerDisp
{
    public StateChangerI(java.util.Timer timer, Ice.ObjectAdapter adapter)
    {
        _timer = timer;
        _otherAdapter = adapter;
    }

    public void hold(int milliSeconds, Ice.Current current)
    {
        if(milliSeconds <= 0)
        {
            _otherAdapter.hold();
            _otherAdapter.waitForHold();
        }
        else
        {
            _timer.schedule(new java.util.TimerTask()
                            {
                                public void
                                run()
                                {
                                    _otherAdapter.hold();
                                }
                            }, milliSeconds);
        }
    }

    public void activate(int milliSeconds, Ice.Current current)
    {
        if(milliSeconds <= 0)
        {
            _otherAdapter.activate();
        }
        else
        {
            _timer.schedule(new java.util.TimerTask()
                            {
                                public void
                                run()
                                {
                                    _otherAdapter.activate();
                                }
                            }, milliSeconds);
        }
    }

    private final java.util.Timer _timer;
    private final Ice.ObjectAdapter _otherAdapter;
}
