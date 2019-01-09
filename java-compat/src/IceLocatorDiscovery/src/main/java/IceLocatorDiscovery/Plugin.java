// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceLocatorDiscovery;

import java.util.List;

interface Plugin extends Ice.Plugin
{
    List<Ice.LocatorPrx> getLocators(String instanceName, int waitTime);
}
