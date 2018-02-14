// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class ThreadObserverI
    extends IceMX.ObserverWithDelegate<IceMX.ThreadMetrics, Ice.Instrumentation.ThreadObserver>
    implements Ice.Instrumentation.ThreadObserver
{
    @Override
    public void
    stateChanged(final Ice.Instrumentation.ThreadState oldState, final Ice.Instrumentation.ThreadState newState)
    {
        _oldState = oldState;
        _newState = newState;
        forEach(_threadStateUpdate);
        if(_delegate != null)
        {
            _delegate.stateChanged(oldState, newState);
        }
    }

    private MetricsUpdate<IceMX.ThreadMetrics> _threadStateUpdate = new MetricsUpdate<IceMX.ThreadMetrics>()
        {
            @Override
            public void
            update(IceMX.ThreadMetrics v)
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

    private Ice.Instrumentation.ThreadState _oldState;
    private Ice.Instrumentation.ThreadState _newState;
}