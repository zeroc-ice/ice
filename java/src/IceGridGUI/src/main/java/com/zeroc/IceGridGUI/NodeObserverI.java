// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI;

import com.zeroc.Ice.Current;
import com.zeroc.IceGrid.*;

import javax.swing.SwingUtilities;

class NodeObserverI implements NodeObserver {
    NodeObserverI(Coordinator coordinator) {
        _coordinator = coordinator;
        _trace = _coordinator.traceObservers();
    }

    @Override
    public void nodeInit(final NodeDynamicInfo[] nodes, Current current) {
        if (_trace) {
            if (nodes.length == 0) {
                _coordinator.traceObserver("nodeInit (no node)");
            } else {
                String names = "";
                for (NodeDynamicInfo node : nodes) {
                    names += " " + node.info.name;
                }
                _coordinator.traceObserver(
                    "nodeInit for node" + (nodes.length == 1 ? "" : "s") + names);
            }
        }

        SwingUtilities.invokeLater(
            () -> {
                for (NodeDynamicInfo node : nodes) {
                    _coordinator.nodeUp(node);
                }
            });
    }

    @Override
    public void nodeUp(final NodeDynamicInfo nodeInfo, Current current) {
        if (_trace) {
            _coordinator.traceObserver("nodeUp for node " + nodeInfo.info.name);
        }

        SwingUtilities.invokeLater(() -> _coordinator.nodeUp(nodeInfo));
    }

    @Override
    public void nodeDown(final String nodeName, Current current) {
        if (_trace) {
            _coordinator.traceObserver("nodeUp for node " + nodeName);
        }

        SwingUtilities.invokeLater(() -> _coordinator.nodeDown(nodeName));
    }

    @Override
    public void updateServer(
            final String node, final ServerDynamicInfo updatedInfo, Current current) {
        if (_trace) {
            _coordinator.traceObserver(
                "updateServer for server "
                    + updatedInfo.id
                    + " on node "
                    + node
                    + "; new state is "
                    + updatedInfo.state.toString());
        }

        SwingUtilities.invokeLater(() -> _coordinator.updateServer(node, updatedInfo));
    }

    @Override
    public void updateAdapter(
            final String node,
            final AdapterDynamicInfo updatedInfo,
            Current current) {
        if (_trace) {
            _coordinator.traceObserver(
                "updateAdapter for adapter "
                    + updatedInfo.id
                    + " on node "
                    + node
                    + "; new proxy is "
                    + (updatedInfo.proxy == null ? "null" : updatedInfo.proxy.toString()));
        }

        SwingUtilities.invokeLater(() -> _coordinator.updateAdapter(node, updatedInfo));
    }

    private final Coordinator _coordinator;
    private final boolean _trace;
}
