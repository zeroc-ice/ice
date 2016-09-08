// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

    @Override
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
                for(NodeDynamicInfo node : nodes)
                {
                    names += " " + node.info.name;
                }
                _coordinator.traceObserver("nodeInit for node" + (nodes.length == 1 ? "" : "s") + names);
            }
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                @Override
                public void run()
                {
                    for(NodeDynamicInfo node : nodes)
                    {
                        _coordinator.nodeUp(node);
                    }
                }
            });
    }

    @Override
    public void nodeUp(final NodeDynamicInfo nodeInfo, Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("nodeUp for node " + nodeInfo.info.name);
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                @Override
                public void run()
                {
                    _coordinator.nodeUp(nodeInfo);
                }
            });
    }

    @Override
    public void nodeDown(final String nodeName, Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("nodeUp for node " + nodeName);
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                @Override
                public void run()
                {
                    _coordinator.nodeDown(nodeName);
                }
            });
    }

    @Override
    public void updateServer(final String node, final ServerDynamicInfo updatedInfo, Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("updateServer for server " + updatedInfo.id
                                       + " on node " + node + "; new state is "
                                       + updatedInfo.state.toString());
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                @Override
                public void run()
                {
                    _coordinator.updateServer(node, updatedInfo);
                }
            });
    }

    @Override
    public void updateAdapter(final String node, final AdapterDynamicInfo updatedInfo, Ice.Current current)
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
                @Override
                public void run()
                {
                    _coordinator.updateAdapter(node, updatedInfo);
                }
            });
    }

    private final Coordinator _coordinator;
    private final boolean _trace;
}
