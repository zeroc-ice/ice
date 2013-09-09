// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class DispatchObserverI 
    extends IceMX.ObserverWithDelegate<IceMX.DispatchMetrics, Ice.Instrumentation.DispatchObserver> 
    implements Ice.Instrumentation.DispatchObserver
{
    public void
    userException()
    {
        forEach(_userException);
        if(_delegate != null)
        {
            _delegate.userException();
        }
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
        if(_delegate != null)
        {
            _delegate.reply(size);
        }
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