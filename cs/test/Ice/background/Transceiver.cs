// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Net.Sockets;

internal class Transceiver : IceInternal.Transceiver
{
    public Socket fd()
    {
        return _transceiver.fd();
    }

    public bool restartable()
    {
        return _transceiver.restartable();
    }

    public bool initialize(AsyncCallback callback)
    {
        _configuration.checkInitializeException();
        bool done = _transceiver.initialize(callback);
        if(done)
        {
            if(_initialized)
            {
                throw new Ice.SocketException();
            }

            _initialized = true;
        }
        return done;
    }

    public void close()
    {
        _transceiver.close();
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

    public IAsyncResult beginRead(IceInternal.Buffer buf, AsyncCallback callback, object state)
    {
        if(!_initialized)
        {
            throw new Ice.SocketException();
        }

        if(_configuration.readReady())
        {
            _configuration.checkReadException(); // Only raise if we're configured to read now.
        }
        return _transceiver.beginRead(buf, callback, state);
    }

    public void endRead(IceInternal.Buffer buf, IAsyncResult result)
    {
        if(!_initialized)
        {
            throw new Ice.SocketException();
        }

        _configuration.checkReadException();
        _transceiver.endRead(buf, result);
    }

    public IAsyncResult beginWrite(IceInternal.Buffer buf, AsyncCallback callback, object state)
    {
        if(!_initialized)
        {
            throw new Ice.SocketException();
        }

        _configuration.checkWriteException();
        return _transceiver.beginWrite(buf, callback, state);
    }

    public void endWrite(IceInternal.Buffer buf, IAsyncResult result)
    {
        if(!_initialized)
        {
            throw new Ice.SocketException();
        }

        _configuration.checkWriteException();
        _transceiver.endWrite(buf, result);
    }

    public string type()
    {
        return "test-" + _transceiver.type();
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
