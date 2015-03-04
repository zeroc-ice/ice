// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class StopWatch
{
    StopWatch()
    {
    }

    public void
    start()
    {
        _stopped = false;
        _start = System.currentTimeMillis();
    }

    public long
    stop()
    {
        if(!_stopped)
        {
            _stop = System.currentTimeMillis();
            _stopped = true;
        }
        return _stop - _start;
    }

    private boolean _stopped;
    private long _start;
    private long _stop;
}
