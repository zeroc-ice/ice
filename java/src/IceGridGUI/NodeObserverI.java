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

class NodeObserverI extends _NodeObserverDisp
{
    NodeObserverI(Coordinator coordinator)
    {
	_coordinator = coordinator;
    }
    
    public void init(final NodeDynamicInfo[] nodes, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    for(int i = 0; i < nodes.length; ++i)
		    {
			_coordinator.nodeUp(nodes[i]);
		    }
		}
	    });
    }

    public void nodeUp(final NodeDynamicInfo nodeInfo, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.nodeUp(nodeInfo);
		}
	    });			   
    }

    public void nodeDown(final String nodeName, Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.nodeDown(nodeName);
		}
	    });			   
    }

    public void updateServer(final String node, final ServerDynamicInfo updatedInfo, 
			     Ice.Current current)
    {	
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.updateServer(node, updatedInfo);
		}
	    });
    }

    public void updateAdapter(final String node, final AdapterDynamicInfo updatedInfo, 
			      Ice.Current current)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _coordinator.updateAdapter(node, updatedInfo);
		}
	    }); 
    }

    private Coordinator _coordinator;
};
