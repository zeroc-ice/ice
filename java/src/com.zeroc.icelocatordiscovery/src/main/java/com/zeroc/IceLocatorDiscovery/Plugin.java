// Copyright (c) ZeroC, Inc.

package com.zeroc.IceLocatorDiscovery;

import com.zeroc.Ice.LocatorPrx;

import java.util.List;

public interface Plugin extends com.zeroc.Ice.Plugin {
    List<LocatorPrx> getLocators(String instanceName, int waitTime);
}
