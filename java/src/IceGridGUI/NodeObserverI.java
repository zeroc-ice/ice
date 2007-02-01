// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        _trace = _coordinator.traceObservers();
    }
    
    public void nodeInit(final NodeDynamicInfo[] nodes, Ice.Current current)
    {
        if(_trace)
        {
            if(nodes.length == 0)
            {
                _coordinator.traceObserver("nodeInit (no node)");
            }
            else
            {
                String names = "";
                for(int i = 0; i < nodes.length; ++i)
                {
                    names += " " + nodes[i].info.name;
                }
                _coordinator.traceObserver("nodeInit for node" 
                                           + (nodes.length == 1 ? "" : "s")
                                           + names);
            }
        }

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
        if(_trace)
        {
            _coordinator.traceObserver("nodeUp for node " + nodeInfo.info.name);
        }
        
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
        if(_trace)
        {
            _coordinator.traceObserver("nodeUp for node " + nodeName);
        }

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
        if(_trace)
        {
            _coordinator.traceObserver("updateServer for server " + updatedInfo.id
                                       + " on node " + node + "; new state is "
                                       + updatedInfo.state.toString());
        }

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
        if(_trace)
        {
            _coordinator.traceObserver("updateAdapter for adapter " + updatedInfo.id
                                       + " on node " + node + "; new proxy is "
                                       + (updatedInfo.proxy == null ? "null" 
                                          : updatedInfo.proxy.toString()));
        }
        
        SwingUtilities.invokeLater(new Runnable() 
            {
                public void run() 
                {
                    _coordinator.updateAdapter(node, updatedInfo);
                }
            }); 
    }

    private final Coordinator _coordinator;
    private final boolean _trace;
};
