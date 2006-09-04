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

class ObjectObserverI extends _ObjectObserverDisp
{

    ObjectObserverI(Coordinator coordinator)
    {
	_coordinator = coordinator;
    }

    public synchronized void objectInit(final ObjectInfo[] objects, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.objectInit(objects);
		}
	    });
    }

    public void objectAdded(final ObjectInfo info, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.objectAdded(info);
		}
	    });
    }    

    public void objectUpdated(final ObjectInfo info, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.objectUpdated(info);
		}
	    });
    }    

    public void objectRemoved(final Ice.Identity id, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.objectRemoved(id);
		}
	    });
    }

    private final Coordinator _coordinator; 
};
