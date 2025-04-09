// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.TimeoutException;
import com.zeroc.IceGrid.ApplicationInfo;
import com.zeroc.IceGrid.ApplicationObserver;
import com.zeroc.IceGrid.ApplicationUpdateInfo;

import javax.swing.SwingUtilities;

import java.util.List;

class ApplicationObserverI implements ApplicationObserver {
    ApplicationObserverI(String instanceName, Coordinator coordinator) {
        _instanceName = instanceName;
        _coordinator = coordinator;
        _trace = coordinator.traceObservers();
    }

    // Runs in the UI thread
    synchronized void waitForInit() {
        // TODO: configurable timeout
        long timeout = 10000;

        if (!_initialized) {
            try {
                wait(timeout);
            } catch (InterruptedException e) {
            }
        }

        if (_initialized) {
            _coordinator.applicationInit(_instanceName, _serial, _applications);
        } else {
            throw new TimeoutException();
        }
    }

    @Override
    public synchronized void applicationInit(
            int serial,
            List<ApplicationInfo> applications,
            Current current) {
        if (_trace) {
            if (applications.isEmpty()) {
                _coordinator.traceObserver(
                    "applicationInit (no application);" + "serial is " + serial);
            } else {
                String names = "";
                for (ApplicationInfo p : applications) {
                    names += " " + p.descriptor.name;
                }

                _coordinator.traceObserver(
                    "applicationInit for application"
                        + (applications.size() == 1 ? "" : "s")
                        + names
                        + "; serial is "
                        + serial);
            }
        }

        _initialized = true;
        _serial = serial;

        _applications = applications;

        notify();
    }

    @Override
    public void applicationAdded(
            final int serial, final ApplicationInfo info, Current current) {
        if (_trace) {
            _coordinator.traceObserver(
                "applicationAdded for application "
                    + info.descriptor.name
                    + "; serial is "
                    + serial);
        }

        SwingUtilities.invokeLater(
            () -> {
                _coordinator.applicationAdded(serial, info);
            });
    }

    @Override
    public void applicationRemoved(
            final int serial, final String name, final Current current) {
        if (_trace) {
            _coordinator.traceObserver(
                "applicationRemoved for application " + name + "; serial is " + serial);
        }

        SwingUtilities.invokeLater(
            () -> {
                _coordinator.applicationRemoved(serial, name);
            });
    }

    @Override
    public void applicationUpdated(
            final int serial, final ApplicationUpdateInfo info, Current current) {
        if (_trace) {
            _coordinator.traceObserver(
                "applicationUpdated for application "
                    + info.descriptor.name
                    + "; serial is "
                    + serial);
        }

        SwingUtilities.invokeLater(
            () -> {
                _coordinator.applicationUpdated(serial, info);
            });
    }

    private final Coordinator _coordinator;
    private final boolean _trace;

    private boolean _initialized;

    // Values given to init
    private final String _instanceName;
    private int _serial;
    private List<ApplicationInfo> _applications;
}
