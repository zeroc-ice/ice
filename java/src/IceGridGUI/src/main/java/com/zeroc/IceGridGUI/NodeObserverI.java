// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI;

import javax.swing.SwingUtilities;
import com.zeroc.IceGrid.*;

class NodeObserverI implements NodeObserver
{
    NodeObserverI(Coordinator coordinator)
    {
        _coordinator = coordinator;
        _trace = _coordinator.traceObservers();
    }

    @Override
    public void nodeInit(final NodeDynamicInfo[] nodes, com.zeroc.Ice.Current current)
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

        SwingUtilities.invokeLater(() ->
            {
                for(NodeDynamicInfo node : nodes)
                {
                    _coordinator.nodeUp(node);
                }
            });
    }

    @Override
    public void nodeUp(final NodeDynamicInfo nodeInfo, com.zeroc.Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("nodeUp for node " + nodeInfo.info.name);
        }

        SwingUtilities.invokeLater(() -> _coordinator.nodeUp(nodeInfo));
    }

    @Override
    public void nodeDown(final String nodeName, com.zeroc.Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("nodeUp for node " + nodeName);
        }

        SwingUtilities.invokeLater(() -> _coordinator.nodeDown(nodeName));
    }

    @Override
    public void updateServer(final String node, final ServerDynamicInfo updatedInfo, com.zeroc.Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("updateServer for server " + updatedInfo.id
                                       + " on node " + node + "; new state is "
                                       + updatedInfo.state.toString());
        }

        SwingUtilities.invokeLater(() -> _coordinator.updateServer(node, updatedInfo));
    }

    @Override
    public void updateAdapter(final String node, final AdapterDynamicInfo updatedInfo, com.zeroc.Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("updateAdapter for adapter " + updatedInfo.id
                                       + " on node " + node + "; new proxy is "
                                       + (updatedInfo.proxy == null ? "null"
                                          : updatedInfo.proxy.toString()));
        }

        SwingUtilities.invokeLater(() -> _coordinator.updateAdapter(node, updatedInfo));
    }

    private final Coordinator _coordinator;
    private final boolean _trace;
}
