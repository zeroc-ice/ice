// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// This is a placeholder for the Android API. It is not included in the IceBT JAR file.
//

package android.bluetooth;

public final class BluetoothSocket implements java.io.Closeable
{
    public void close()
        throws java.io.IOException
    {
    }

    public void connect()
        throws java.io.IOException
    {
    }

    public java.io.InputStream getInputStream()
        throws java.io.IOException
    {
        return null;
    }

    public java.io.OutputStream getOutputStream()
        throws java.io.IOException
    {
        return null;
    }

    public BluetoothDevice getRemoteDevice()
    {
        return null;
    }

    public boolean isConnected()
    {
        return false;
    }
}
