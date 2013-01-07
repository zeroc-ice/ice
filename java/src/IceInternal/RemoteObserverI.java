// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class RemoteObserverI extends IceMX.Observer<IceMX.RemoteMetrics> 
    implements Ice.Instrumentation.RemoteObserver
{
    public void
    reply(final int size)
    {
        forEach(new MetricsUpdate<IceMX.RemoteMetrics>()
                {
                    public void
                    update(IceMX.RemoteMetrics v)
                    {
                        v.replySize += size;
                    }
                });
    }
}