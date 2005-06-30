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

    RegistryObserverI(SessionKeeper sessionKeeper, StatusBar statusBar, Model model)
    {
	_sessionKeeper = sessionKeeper;
	_statusBar = statusBar;
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
	    _model.registryInit(_serial, _applications);
	    _statusBar.setText("Connected; initialized (" + _serial + ")"); 
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
		    if(_model.updateSerial(serial))
		    {
			_model.applicationAdded(desc);
			_statusBar.setText("Connected; application '" 
					   + desc.name + "' added (" + serial + ")"); 
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
			_statusBar.setText("Connected; application '" 
					   + name + "' removed (" + serial + ")"); 
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
			_statusBar.setText("Connected; application '" 
					   + desc.name + "' sync-ed (" + serial + ")"); 
		    }
		    else
		    {
			_sessionKeeper.sessionLost(
			    "Received application update (application sync-ed) out of sequence");
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
			_statusBar.setText("Connected; application '" 
					   + desc.name + "' updated (" + serial + ")"); 
		    }
		    else
		    {
			_sessionKeeper.sessionLost("Received application update out of sequence");
		    }
		}
	    });
    }
    

    private SessionKeeper _sessionKeeper;
    private StatusBar _statusBar;
    private Model _model;
 
    private boolean _initialized = false;
    
    //
    // Values given to init
    //
    private int _serial;
    private java.util.LinkedList _applications;
};
