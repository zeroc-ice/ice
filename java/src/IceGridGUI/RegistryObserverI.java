// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI;

import javax.swing.SwingUtilities;
import IceGrid.*;

class RegistryObserverI extends _RegistryObserverDisp
{

    RegistryObserverI(Coordinator coordinator)
    {
	_coordinator = coordinator;
    }

    public void registryInit(final RegistryInfo[] registries, Ice.Current current)
    {
	// TODO: XXX
    }

    public void registryUp(final RegistryInfo registryInfo, Ice.Current current)
    {
	// TODO: XXX
// 	SwingUtilities.invokeLater(new Runnable() 
// 	    {
// 		public void run() 
// 		{
// 		    _coordinator.registryUp(registryInfo);
// 		}
// 	    });			   
    }

    public void registryDown(final String registryName, Ice.Current current)
    {
	// TODO: XXX
// 	SwingUtilities.invokeLater(new Runnable() 
// 	    {
// 		public void run() 
// 		{
// 		    _coordinator.registryDown(registryName);
// 		}
// 	    });			   
    }

    private final Coordinator _coordinator;
};
