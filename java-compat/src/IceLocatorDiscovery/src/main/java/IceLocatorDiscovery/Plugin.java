// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceLocatorDiscovery;

import java.util.List;

interface Plugin extends Ice.Plugin
{
    List<Ice.LocatorPrx> getLocators(String instanceName, int waitTime);
}
