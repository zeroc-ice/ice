// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

import com.zeroc.Ice.Instrumentation.CollocatedObserver;

class CollocatedObserverI extends ChildInvocationObserverI
    implements CollocatedObserver {}
