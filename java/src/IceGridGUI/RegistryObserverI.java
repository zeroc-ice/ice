// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

    //
    // Runs in the UI thread
    //
    synchronized void waitForInit()
    {
	//
	// TODO: configurable timeout
	//
	long timeout = 10000;

	long start = System.currentTimeMillis();

	while(!_initialized)
	{
	    try
	    {
		wait(timeout);
	    }
	    catch(InterruptedException e)
	    {
	    }

	    if((start + timeout) < System.currentTimeMillis())
	    {
		break;
	    }
	}
	
	if(_initialized)
	{
	    _coordinator.registryInit(_serial, _applications);
	}
	else
	{
	    throw new Ice.TimeoutException();
	}
    }


    public synchronized void init(int serial, java.util.LinkedList applications, 
				  Ice.Current current)
    {
	_initialized = true;
	_serial = serial;
	_applications = applications;
	notify();
    }

    public void applicationAdded(final int serial, final ApplicationDescriptor desc, 
				 Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.applicationAdded(serial, desc);
		}
	    });
    }

    public void applicationRemoved(final int serial, final String name, 
				   final Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.applicationRemoved(serial, name);
		}
	    });
    }

    public void applicationUpdated(final int serial, final ApplicationUpdateDescriptor desc, 
				   Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.applicationUpdated(serial, desc);
		}
	    });
    }
    

    private Coordinator _coordinator;
 
    private boolean _initialized = false;
    
    //
    // Values given to init
    //
    private int _serial;
    private java.util.LinkedList _applications;
};
