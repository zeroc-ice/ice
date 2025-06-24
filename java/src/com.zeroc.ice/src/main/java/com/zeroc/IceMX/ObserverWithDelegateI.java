// Copyright (c) ZeroC, Inc.

package com.zeroc.IceMX;

import com.zeroc.Ice.Instrumentation.Observer;

/**
 * Observer implementation that supports delegation to another observer.
 */
public class ObserverWithDelegateI
    extends ObserverWithDelegate<Metrics, Observer> {}
