// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Net.Sockets;

internal class Transceiver : IceInternal.Transceiver
{
    public int initialize()
    {
        _configuration.checkInitializeException();
        if(!_initialized)
        {
            int s = _transceiver.initialize();
            if(s != IceInternal.SocketOperation.None)
            {
                return s;
            }
            _initialized = true;
        }
        return IceInternal.SocketOperation.None;
    }

    public void close()
    {
        _transceiver.close();
    }

    public void destroy()
    {
    }

    public bool write(IceInternal.Buffer buf)
    {
        if(!_initialized)
        {
            throw new Ice.SocketException();
        }

        if(!_configuration.writeReady())
        {
            return false;
        }

        _configuration.checkWriteException();
        return _transceiver.write(buf);
    }

    public bool read(IceInternal.Buffer buf)
    {
        if(!_initialized)
        {
            throw new Ice.SocketException();
        }

        if(!_configuration.readReady())
        {
            return false;
        }

        _configuration.checkReadException();
        return _transceiver.read(buf);
    }

    public bool startRead(IceInternal.Buffer buf, IceInternal.AsyncCallback callback, object state)
    {
        if(_configuration.readReady())
        {
            _configuration.checkReadException(); // Only raise if we're configured to read now.
        }
        return _transceiver.startRead(buf, callback, state);
    }

    public void finishRead(IceInternal.Buffer buf)
    {
        _configuration.checkReadException();
        _transceiver.finishRead(buf);
    }

    public bool startWrite(IceInternal.Buffer buf, IceInternal.AsyncCallback callback, object state, out bool completed)
    {
        _configuration.checkWriteException();
        return _transceiver.startWrite(buf, callback, state, out completed);
    }

    public void finishWrite(IceInternal.Buffer buf)
    {
        _configuration.checkWriteException();
        _transceiver.finishWrite(buf);
    }

    public string type()
    {
        return "test-" + _transceiver.type();
    }

    public Ice.ConnectionInfo getInfo()
    {
        return _transceiver.getInfo();
    }

    public override string ToString()
    {
        return _transceiver.ToString();
    }

    public void checkSendSize(IceInternal.Buffer buf, int messageSizeMax)
    {
        _transceiver.checkSendSize(buf, messageSizeMax);
    }

    //
    // Only for use by Connector, Acceptor
    //
    internal Transceiver(IceInternal.Transceiver transceiver)
    {
        _transceiver = transceiver;
        _configuration = Configuration.getInstance();
    }

    private IceInternal.Transceiver _transceiver;
    private Configuration _configuration;
    private bool _initialized = false;
}
