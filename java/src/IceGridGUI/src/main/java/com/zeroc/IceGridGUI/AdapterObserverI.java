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

class AdapterObserverI implements AdapterObserver
{
    AdapterObserverI(Coordinator coordinator)
    {
        _coordinator = coordinator;
        _trace = coordinator.traceObservers();
    }

    @Override
    public synchronized void adapterInit(final AdapterInfo[] adapters, com.zeroc.Ice.Current current)
    {
        if(_trace)
        {
            if(adapters.length == 0)
            {
                _coordinator.traceObserver("adapterInit (no adapter)");
            }
            else
            {
                String names = "";
                for(AdapterInfo info : adapters)
                {
                    names += " " + info.id;
                }

                _coordinator.traceObserver("adapterInit for adapters" + names);
            }
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                @Override
                public void run()
                {
                    _coordinator.adapterInit(adapters);
                }
            });
    }

    @Override
    public void adapterAdded(final AdapterInfo info, com.zeroc.Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("adapterAdded for adapter " + info.id);
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                @Override
                public void run()
                {
                    _coordinator.adapterAdded(info);
                }
            });
    }

    @Override
    public void adapterUpdated(final AdapterInfo info, com.zeroc.Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("adapterUpdated for adapter " + info.id);
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                @Override
                public void run()
                {
                    _coordinator.adapterUpdated(info);
                }
            });
    }

    @Override
    public void adapterRemoved(final String id, com.zeroc.Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("adapterRemoved for adapter " + id);
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                @Override
                public void run()
                {
                    _coordinator.adapterRemoved(id);
                }
            });
    }

    private final Coordinator _coordinator;
    private final boolean _trace;
}
