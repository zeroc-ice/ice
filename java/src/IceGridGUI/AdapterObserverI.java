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

class AdapterObserverI extends _AdapterObserverDisp
{

    AdapterObserverI(Coordinator coordinator)
    {
	_coordinator = coordinator;
    }

    public synchronized void adapterInit(final AdapterInfo[] adapters, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.adapterInit(adapters);
		}
	    });
    }

    public void adapterAdded(final AdapterInfo info, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.adapterAdded(info);
		}
	    });
    }    

    public void adapterUpdated(final AdapterInfo info, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.adapterUpdated(info);
		}
	    });
    }    

    public void adapterRemoved(final String id, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.adapterRemoved(id);
		}
	    });
    }

    private final Coordinator _coordinator;
};
