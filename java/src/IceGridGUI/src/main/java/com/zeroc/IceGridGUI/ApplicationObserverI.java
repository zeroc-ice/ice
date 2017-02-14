// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI;

import javax.swing.SwingUtilities;
import com.zeroc.IceGrid.*;

class ApplicationObserverI implements ApplicationObserver
{
    ApplicationObserverI(String instanceName, Coordinator coordinator)
    {
        _instanceName = instanceName;
        _coordinator = coordinator;
        _trace = coordinator.traceObservers();
    }

    //
    // Runs in the UI thread
    //
    synchronized void waitForInit()
    {
        //
        // TODO: configurable timeout
        //
        long timeout = 10000;

        if(!_initialized)
        {
            try
            {
                wait(timeout);
            }
            catch(InterruptedException e)
            {
            }
        }

        if(_initialized)
        {
            _coordinator.applicationInit(_instanceName, _serial, _applications);
        }
        else
        {
            throw new com.zeroc.Ice.TimeoutException();
        }
    }

    @Override
    public synchronized void applicationInit(int serial, java.util.List<ApplicationInfo> applications,
                                             com.zeroc.Ice.Current current)
    {
        if(_trace)
        {
            if(applications.size() == 0)
            {
                _coordinator.traceObserver("applicationInit (no application);" + "serial is " + serial);
            }
            else
            {
                String names = "";
                for(ApplicationInfo p : applications)
                {
                    names += " " + p.descriptor.name;
                }

                _coordinator.traceObserver("applicationInit for application"
                                           + (applications.size() == 1 ? "" : "s")
                                           + names
                                           + "; serial is " + serial);
            }
        }

        _initialized = true;
        _serial = serial;

        _applications = applications;

        notify();
    }

    @Override
    public void applicationAdded(final int serial, final ApplicationInfo info, com.zeroc.Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("applicationAdded for application "
                                       + info.descriptor.name
                                       + "; serial is " + serial);
        }

        SwingUtilities.invokeLater(() -> { _coordinator.applicationAdded(serial, info); });
    }

    @Override
    public void applicationRemoved(final int serial, final String name, final com.zeroc.Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("applicationRemoved for application "
                                       + name
                                       + "; serial is " + serial);
        }

        SwingUtilities.invokeLater(() -> { _coordinator.applicationRemoved(serial, name); });
    }

    @Override
    public void applicationUpdated(final int serial, final ApplicationUpdateInfo info, com.zeroc.Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("applicationUpdated for application "
                                       + info.descriptor.name
                                       + "; serial is " + serial);
        }

        SwingUtilities.invokeLater(() ->  { _coordinator.applicationUpdated(serial, info); });
    }

    private final Coordinator _coordinator;
    private final boolean _trace;

    private boolean _initialized = false;

    //
    // Values given to init
    //
    private final String _instanceName;
    private int _serial;
    private java.util.List<ApplicationInfo> _applications;
}
