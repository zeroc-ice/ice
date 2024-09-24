//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

import com.zeroc.Ice.IceMX.ThreadMetrics;
import com.zeroc.Ice.Instrumentation.ThreadObserver;
import com.zeroc.Ice.Instrumentation.ThreadState;

/**
 * @hidden Public because it's used by IceMX (via reflection).
 */
public class ThreadObserverI
        extends com.zeroc.Ice.IceMX.ObserverWithDelegate<ThreadMetrics, ThreadObserver>
        implements ThreadObserver {
    @Override
    public void stateChanged(final ThreadState oldState, final ThreadState newState) {
        _oldState = oldState;
        _newState = newState;
        forEach(_threadStateUpdate);
        if (_delegate != null) {
            _delegate.stateChanged(oldState, newState);
        }
    }

    private com.zeroc.Ice.IceMX.Observer.MetricsUpdate<ThreadMetrics> _threadStateUpdate =
            new com.zeroc.Ice.IceMX.Observer.MetricsUpdate<ThreadMetrics>() {
                @Override
                public void update(ThreadMetrics v) {
                    switch (_oldState) {
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
                    switch (_newState) {
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

    private ThreadState _oldState;
    private ThreadState _newState;
}
