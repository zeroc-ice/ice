// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceMX;

public class DispatchObserverI extends Observer<DispatchMetrics> implements Ice.Instrumentation.DispatchObserver
{
    public void
    userException()
    {
        forEach(_userException);
    }

    final MetricsUpdate<DispatchMetrics> _userException = new MetricsUpdate<DispatchMetrics>()
        {
            public void
            update(DispatchMetrics v)
            {
                ++v.userException;
            }
        };
}