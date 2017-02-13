// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI;

import javax.swing.SwingUtilities;
import IceGrid.*;

class RegistryObserverI extends _RegistryObserverDisp
{
    RegistryObserverI(Coordinator coordinator)
    {
        _coordinator = coordinator;
        _trace = _coordinator.traceObservers();
    }

    @Override
    public void registryInit(final RegistryInfo[] registryInfos, Ice.Current current)
    {
        if(_trace)
        {
            if(registryInfos.length == 0)
            {
                _coordinator.traceObserver("registryInit (no registry)");
            }
            else
            {
                String names = "";
                for(RegistryInfo info : registryInfos)
                {
                    names += " " + info.name;
                }
                _coordinator.traceObserver("registryInit for registr" +
                                           (registryInfos.length == 1 ? "y" : "ies")
                                           + names);
            }
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                @Override
                public void run()
                {
                    for(RegistryInfo info : registryInfos)
                    {
                        _coordinator.registryUp(info);
                    }
                }
            });
    }

    @Override
    public void registryUp(final RegistryInfo registryInfo, Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("registryUp for registry " + registryInfo.name);
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                @Override
                public void run()
                {
                    _coordinator.registryUp(registryInfo);
                }
            });
    }

    @Override
    public void registryDown(final String registryName, Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("registryDown for registry " + registryName);
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                @Override
                public void run()
                {
                    _coordinator.registryDown(registryName);
                }
            });
    }

    private final Coordinator _coordinator;
    private final boolean _trace;
}
