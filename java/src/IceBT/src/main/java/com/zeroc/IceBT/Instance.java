// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceBT;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.PluginInitializationException;

import android.bluetooth.BluetoothAdapter;

class Instance extends com.zeroc.IceInternal.ProtocolInstance
{
    Instance(Communicator communicator, short type, String protocol)
    {
        //
        // We consider the transport to be "secure" because it uses the secure versions Android's Bluetooth API
        // methods for establishing and accepting connections. The boolean argument below sets secure=true.
        //
        super(communicator, type, protocol, true);

        _communicator = communicator;

        _bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if(_bluetoothAdapter == null)
        {
            throw new PluginInitializationException("bluetooth adapter not available");
        }
        else if(!_bluetoothAdapter.isEnabled())
        {
            throw new PluginInitializationException("bluetooth is not enabled");
        }
    }

    void destroy()
    {
        _communicator = null;
    }

    Communicator communicator()
    {
        return _communicator;
    }

    BluetoothAdapter bluetoothAdapter()
    {
        return _bluetoothAdapter;
    }

    private Communicator _communicator;
    private BluetoothAdapter _bluetoothAdapter;
}
