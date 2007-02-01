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

class AdapterObserverI extends _AdapterObserverDisp
{
    AdapterObserverI(Coordinator coordinator)
    {
        _coordinator = coordinator;
        _trace = coordinator.traceObservers();
    }

    public synchronized void adapterInit(final AdapterInfo[] adapters, Ice.Current current)
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
                for(int i = 0; i < adapters.length; ++i)
                {
                    names += " " + adapters[i].id;
                }
                
                _coordinator.traceObserver("adapterInit for adapters" + names);
            }
        }

        SwingUtilities.invokeLater(new Runnable() 
            {
                public void run() 
                {
                    _coordinator.adapterInit(adapters);
                }
            });
    }

    public void adapterAdded(final AdapterInfo info, Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("adapterAdded for adapter " + info.id);
        }

        SwingUtilities.invokeLater(new Runnable() 
            {
                public void run() 
                {
                    _coordinator.adapterAdded(info);
                }
            });
    }    

    public void adapterUpdated(final AdapterInfo info, Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("adapterUpdated for adapter " + info.id);
        }

        SwingUtilities.invokeLater(new Runnable() 
            {
                public void run() 
                {
                    _coordinator.adapterUpdated(info);
                }
            });
    }    

    public void adapterRemoved(final String id, Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("adapterRemoved for adapter " + id);
        }

        SwingUtilities.invokeLater(new Runnable() 
            {
                public void run() 
                {
                    _coordinator.adapterRemoved(id);
                }
            });
    }

    private final Coordinator _coordinator;
    private final boolean _trace;
};
