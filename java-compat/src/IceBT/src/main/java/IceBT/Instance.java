// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceBT;

import android.bluetooth.BluetoothAdapter;

class Instance extends IceInternal.ProtocolInstance
{
    Instance(Ice.Communicator communicator, short type, String protocol)
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
            throw new Ice.PluginInitializationException("bluetooth adapter not available");
        }
        else if(!_bluetoothAdapter.isEnabled())
        {
            throw new Ice.PluginInitializationException("bluetooth is not enabled");
        }
    }

    void destroy()
    {
        _communicator = null;
    }

    Ice.Communicator communicator()
    {
        return _communicator;
    }

    BluetoothAdapter bluetoothAdapter()
    {
        return _bluetoothAdapter;
    }

    private Ice.Communicator _communicator;
    private BluetoothAdapter _bluetoothAdapter;
}
