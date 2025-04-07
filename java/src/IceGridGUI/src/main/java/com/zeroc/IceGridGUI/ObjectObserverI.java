// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.Identity;
import com.zeroc.IceGrid.*;

import javax.swing.SwingUtilities;

class ObjectObserverI implements ObjectObserver {
    ObjectObserverI(Coordinator coordinator) {
        _coordinator = coordinator;
        _trace = coordinator.traceObservers();
    }

    @Override
    public synchronized void objectInit(final ObjectInfo[] objects, Current current) {
        if (_trace) {
            if (objects.length == 0) {
                _coordinator.traceObserver("objectInit (no object)");
            } else {
                String names = "";
                for (ObjectInfo obj : objects) {
                    names += " " + obj.proxy.toString();
                }

                _coordinator.traceObserver("objectInit for objects" + names);
            }
        }

        SwingUtilities.invokeLater(() -> _coordinator.objectInit(objects));
    }

    @Override
    public void objectAdded(final ObjectInfo info, Current current) {
        if (_trace) {
            _coordinator.traceObserver("objectAdded for object " + info.proxy.toString());
        }

        SwingUtilities.invokeLater(() -> _coordinator.objectAdded(info));
    }

    @Override
    public void objectUpdated(final ObjectInfo info, Current current) {
        if (_trace) {
            _coordinator.traceObserver("objectUpdated for object " + info.proxy.toString());
        }

        SwingUtilities.invokeLater(() -> _coordinator.objectUpdated(info));
    }

    @Override
    public void objectRemoved(final Identity id, Current current) {
        if (_trace) {
            _coordinator.traceObserver(
                    "objectRemoved for object "
                            + _coordinator.getCommunicator().identityToString(id));
        }

        SwingUtilities.invokeLater(() -> _coordinator.objectRemoved(id));
    }

    private final Coordinator _coordinator;
    private final boolean _trace;
}
