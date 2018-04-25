// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public class ThreadObserverI
    extends com.zeroc.IceMX.ObserverWithDelegate<com.zeroc.IceMX.ThreadMetrics,
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

    private com.zeroc.IceMX.Observer.MetricsUpdate<com.zeroc.IceMX.ThreadMetrics> _threadStateUpdate =
        new com.zeroc.IceMX.Observer.MetricsUpdate<com.zeroc.IceMX.ThreadMetrics>()
        {
            @Override
            public void update(com.zeroc.IceMX.ThreadMetrics v)
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
