//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public class ThreadObserverI
    extends com.zeroc.Ice.MX.ObserverWithDelegate<com.zeroc.Ice.MX.ThreadMetrics,
                                                 com.zeroc.Ice.Instrumentation.ThreadObserver>
    implements com.zeroc.Ice.Instrumentation.ThreadObserver
{
    @Override
    public void stateChanged(final com.zeroc.Ice.Instrumentation.ThreadState oldState,
                             final com.zeroc.Ice.Instrumentation.ThreadState newState)
    {
        _oldState = oldState;
        _newState = newState;
        forEach(_threadStateUpdate);
        if(_delegate != null)
        {
            _delegate.stateChanged(oldState, newState);
        }
    }

    private com.zeroc.Ice.MX.Observer.MetricsUpdate<com.zeroc.Ice.MX.ThreadMetrics> _threadStateUpdate =
        new com.zeroc.Ice.MX.Observer.MetricsUpdate<com.zeroc.Ice.MX.ThreadMetrics>()
        {
            @Override
            public void update(com.zeroc.Ice.MX.ThreadMetrics v)
            {
                switch(_oldState)
                {
                case ThreadStateInUseForIO:
                    --v.inUseForIO;
                    break;
                case ThreadStateInUseForUser:
                    --v.inUseForUser;
                    break;
                case ThreadStateInUseForOther:
                    --v.inUseForOther;
                    break;
                default:
                    break;
                }
                switch(_newState)
                {
                case ThreadStateInUseForIO:
                    ++v.inUseForIO;
                    break;
                case ThreadStateInUseForUser:
                    ++v.inUseForUser;
                    break;
                case ThreadStateInUseForOther:
                    ++v.inUseForOther;
                    break;
                default:
                    break;
                }
            }
        };

    private com.zeroc.Ice.Instrumentation.ThreadState _oldState;
    private com.zeroc.Ice.Instrumentation.ThreadState _newState;
}
