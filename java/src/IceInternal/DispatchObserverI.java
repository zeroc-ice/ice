// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class DispatchObserverI extends IceMX.Observer<IceMX.DispatchMetrics> 
    implements Ice.Instrumentation.DispatchObserver
{
    public void
    userException()
    {
        forEach(_userException);
    }

    public void
    reply(final int size)
    {
        forEach(new MetricsUpdate<IceMX.DispatchMetrics>()
                {
                    public void
                    update(IceMX.DispatchMetrics v)
                    {
                        v.replySize += size;
                    }
                });
    }

    final MetricsUpdate<IceMX.DispatchMetrics> _userException = new MetricsUpdate<IceMX.DispatchMetrics>()
        {
            public void
            update(IceMX.DispatchMetrics v)
            {
                ++v.userException;
            }
        };
}