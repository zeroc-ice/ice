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

class ObjectObserverI extends _ObjectObserverDisp
{
    ObjectObserverI(Coordinator coordinator)
    {
        _coordinator = coordinator;
        _trace = coordinator.traceObservers();
    }

    @Override
    public synchronized void objectInit(final ObjectInfo[] objects, Ice.Current current)
    {
        if(_trace)
        {
            if(objects.length == 0)
            {
                _coordinator.traceObserver("objectInit (no object)");
            }
            else
            {
                String names = "";
                for(ObjectInfo obj : objects)
                {
                    names += " " + obj.proxy.toString();
                }

                _coordinator.traceObserver("objectInit for objects" + names);
            }
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                @Override
                public void run()
                {
                    _coordinator.objectInit(objects);
                }
            });
    }

    @Override
    public void objectAdded(final ObjectInfo info, Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("objectAdded for object " + info.proxy.toString());
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                @Override
                public void run()
                {
                    _coordinator.objectAdded(info);
                }
            });
    }

    @Override
    public void objectUpdated(final ObjectInfo info, Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("objectUpdated for object " + info.proxy.toString());
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                @Override
                public void run()
                {
                    _coordinator.objectUpdated(info);
                }
            });
    }

    @Override
    public void objectRemoved(final Ice.Identity id, Ice.Current current)
    {
        if(_trace)
        {
            _coordinator.traceObserver("objectRemoved for object " + Ice.Util.identityToString(id));
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                @Override
                public void run()
                {
                    _coordinator.objectRemoved(id);
                }
            });
    }

    private final Coordinator _coordinator;
    private final boolean _trace;
}
