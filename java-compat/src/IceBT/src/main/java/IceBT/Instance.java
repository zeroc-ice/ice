// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceBT;

import android.bluetooth.BluetoothAdapter;

class Instance extends IceInternal.ProtocolInstance
{
    Instance(Ice.Communicator communicator, short type, String protocol)
    {
        super(communicator, type, protocol, type == Ice.BTSEndpointType.value);

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
