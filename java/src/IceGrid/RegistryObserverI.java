// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;

class RegistryObserverI extends _RegistryObserverDisp
{

    RegistryObserverI(Model model)
    {
	_model = model;
    }

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
	
	if(!_initialized)
	{
	    throw new Ice.TimeoutException();
	}
    }


    public void init(int serial, ApplicationDescriptor[] applications, String[] nodesUp, Ice.Current current)
    {
	synchronized(this)
	{
	    _initialized = true;
	    notify();
	}

	_model.registryInit(serial, applications, nodesUp);
    }

    public void applicationAdded(int serial, ApplicationDescriptor desc, Ice.Current current)
    {
	_model.applicationAdded(serial, desc);
    }

    public void applicationRemoved(int serial, String name, Ice.Current current)
    {
	_model.applicationRemoved(serial, name);
    }

    public void applicationSynced(int serial, ApplicationDescriptor desc, Ice.Current current)
    {

    }

    public void applicationUpdated(int serial, ApplicationUpdateDescriptor desc, Ice.Current current)
    {

    }
    
    public void nodeUp(String name, Ice.Current current)
    {

    }
    public void nodeDown(String name, Ice.Current current)
    {

    }

    private Model _model;
    private boolean _initialized = false;
};
