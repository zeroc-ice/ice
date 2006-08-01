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

    RegistryObserverI(String instanceName, Coordinator coordinator)
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
	    _coordinator.registryInit(_instanceName,
				      _serial, _applications, _adapters, _objects);
	}
	else
	{
	    throw new Ice.TimeoutException();
	}
    }


    public synchronized void init(int serial, java.util.List applications, AdapterInfo[] adapters, 
				  ObjectInfo[] objects, Ice.Current current)
    {
	_initialized = true;
	_serial = serial;

	_applications = new java.util.LinkedList();
	java.util.Iterator p = applications.iterator();
	while(p.hasNext())
	{
	    _applications.add(((ApplicationInfo)p.next()).descriptor); // TODO: Use ApplicationInfo directly.
	}

	_adapters = adapters;
	_objects = objects;
	notify();
    }

    public void applicationAdded(final int serial, final ApplicationInfo info, 
				 Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.applicationAdded(serial, info.descriptor); // TODO: Use ApplicationInfo directly.
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
		    _coordinator.applicationUpdated(serial, info.descriptor); // TODO: Use ApplicationUpdateInfo
		}
	    });
    }

    public void adapterAdded(final int serial, final AdapterInfo info, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.adapterAdded(serial, info);
		}
	    });
    }    

    public void adapterUpdated(final int serial, final AdapterInfo info, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.adapterUpdated(serial, info);
		}
	    });
    }    

    public void adapterRemoved(final int serial, final String id, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.adapterRemoved(serial, id);
		}
	    });
    }    

    public void objectAdded(final int serial, final ObjectInfo info, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.objectAdded(serial, info);
		}
	    });
    }    

    public void objectUpdated(final int serial, final ObjectInfo info, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.objectUpdated(serial, info);
		}
	    });
    }    

    public void objectRemoved(final int serial, final Ice.Identity id, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.objectRemoved(serial, id);
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
    private AdapterInfo[] _adapters;
    private ObjectInfo[] _objects;
};
