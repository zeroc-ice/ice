// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI;

import com.zeroc.Ice.Current;
import com.zeroc.IceGrid.RegistryInfo;
import com.zeroc.IceGrid.RegistryObserver;

import javax.swing.SwingUtilities;

class RegistryObserverI implements RegistryObserver {
    RegistryObserverI(Coordinator coordinator) {
        _coordinator = coordinator;
        _trace = _coordinator.traceObservers();
    }

    @Override
    public void registryInit(final RegistryInfo[] registryInfos, Current current) {
        if (_trace) {
            if (registryInfos.length == 0) {
                _coordinator.traceObserver("registryInit (no registry)");
            } else {
                String names = "";
                for (RegistryInfo info : registryInfos) {
                    names += " " + info.name;
                }
                _coordinator.traceObserver(
                    "registryInit for registr"
                        + (registryInfos.length == 1 ? "y" : "ies")
                        + names);
            }
        }

        SwingUtilities.invokeLater(
            () -> {
                for (RegistryInfo info : registryInfos) {
                    _coordinator.registryUp(info);
                }
            });
    }

    @Override
    public void registryUp(final RegistryInfo registryInfo, Current current) {
        if (_trace) {
            _coordinator.traceObserver("registryUp for registry " + registryInfo.name);
        }

        SwingUtilities.invokeLater(() -> _coordinator.registryUp(registryInfo));
    }

    @Override
    public void registryDown(final String registryName, Current current) {
        if (_trace) {
            _coordinator.traceObserver("registryDown for registry " + registryName);
        }

        SwingUtilities.invokeLater(() -> _coordinator.registryDown(registryName));
    }

    private final Coordinator _coordinator;
    private final boolean _trace;
}
