// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceMX;

public class ThreadObserverI extends Observer<ThreadMetrics> implements Ice.Instrumentation.ThreadObserver
{
    public void
    stateChanged(final Ice.Instrumentation.ThreadState oldState, final Ice.Instrumentation.ThreadState newState)
    {
        _oldState = oldState;
        _newState = newState;
        forEach(_threadStateUpdate);
    }

    private MetricsUpdate<ThreadMetrics> _threadStateUpdate = new MetricsUpdate<ThreadMetrics>()
        {
            public void
            update(ThreadMetrics v)
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