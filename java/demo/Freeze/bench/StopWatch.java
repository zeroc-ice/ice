// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
