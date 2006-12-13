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

class ApplicationObserverI extends _ApplicationObserverDisp
{

    ApplicationObserverI(String instanceName, Coordinator coordinator)
    {
	_instanceName = instanceName;
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

	if(!_initialized)
	{
	    try
	    {
		wait(timeout);
	    }
	    catch(InterruptedException e)
	    {
	    }   
	}

	if(_initialized)
	{
	    _coordinator.applicationInit(_instanceName, _serial, _applications);
	}
	else
	{
	    throw new Ice.TimeoutException();
	}
    }

    public synchronized void applicationInit(int serial, java.util.List applications, Ice.Current current)
    {
	_initialized = true;
	_serial = serial;

	_applications = new java.util.LinkedList();
	java.util.Iterator p = applications.iterator();
	while(p.hasNext())
	{
	    _applications.add(((ApplicationInfo)p.next()).descriptor); // TODO: XXX: Use ApplicationInfo directly.
	}

	notify();
    }

    public void applicationAdded(final int serial, final ApplicationInfo info, 
				 Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.applicationAdded(serial, info.descriptor); // TODO: XXX: Use ApplicationInfo directly.
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

    public void applicationUpdated(final int serial, final ApplicationUpdateInfo info, 
				   Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.applicationUpdated(serial, info.descriptor); // TODO: XXX: Use ApplicationUpdateInfo
		}
	    });
    }

    private final Coordinator _coordinator;
 
    private boolean _initialized = false;
    
    //
    // Values given to init
    //
    private final String _instanceName;
    private int _serial;
    private java.util.List _applications;
};
