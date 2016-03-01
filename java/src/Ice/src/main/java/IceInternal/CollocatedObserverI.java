// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class CollocatedObserverI
    extends IceMX.ObserverWithDelegate<IceMX.CollocatedMetrics, Ice.Instrumentation.CollocatedObserver>
    implements Ice.Instrumentation.CollocatedObserver
{
    @Override
    public void
    reply(final int size)
    {
        forEach(new MetricsUpdate<IceMX.CollocatedMetrics>()
                {
                    @Override
                    public void
                    update(IceMX.CollocatedMetrics v)
                    {
                        v.replySize += size;
                    }
                });
        if(_delegate != null)
        {
            _delegate.reply(size);
        }
    }
}
