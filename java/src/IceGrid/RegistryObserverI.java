// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;

import javax.swing.SwingUtilities;

class RegistryObserverI extends _RegistryObserverDisp
{

    RegistryObserverI(SessionKeeper sessionKeeper, Model model)
    {
	_sessionKeeper = sessionKeeper;
	_model = model;
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
	    _model.registryInit(_serial, _applications, _nodesUp);
	}
	else
	{
	    throw new Ice.TimeoutException();
	}
    }


    public synchronized void init(int serial, java.util.LinkedList applications, 
				  String[] nodesUp, Ice.Current current)
    {
	_initialized = true;
	_serial = serial;
	_applications = applications;
	_nodesUp = nodesUp;
	notify();
    }

    public void applicationAdded(final int serial, final ApplicationDescriptor desc, 
				 Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    if(_model.updateSerial(serial))
		    {
			_model.applicationAdded(desc);
		    }
		    else
		    {
			_sessionKeeper.sessionLost(
			    "Received application update (new application) out of sequence");
		    }
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
		    if(_model.updateSerial(serial))
		    {
			_model.applicationRemoved(name);
		    }
		    else
		    {
			_sessionKeeper.sessionLost(
			    "Received application update (application removed) out of sequence");
		    }
		}
	    });
    }

    public void applicationSynced(final int serial, final ApplicationDescriptor desc, 
				  Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    if(_model.updateSerial(serial))
		    {
			_model.applicationSynced(desc);
		    }
		    else
		    {
			_sessionKeeper.sessionLost(
			    "Received application update (application synced) out of sequence");
		    }
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
		    if(_model.updateSerial(serial))
		    {
			_model.applicationUpdated(desc);
		    }
		    else
		    {
			_sessionKeeper.sessionLost("Received application update out of sequence");
		    }
		}
	    });
    }
    
    public void nodeUp(String name, Ice.Current current)
    {
	

    }

    public void nodeDown(String name, Ice.Current current)
    {

    }

    private Model _model;
    private SessionKeeper _sessionKeeper;
    private boolean _initialized = false;
    
    //
    // Values given to init
    //
    private int _serial;
    private java.util.LinkedList _applications;
    private String[] _nodesUp;
    
};
