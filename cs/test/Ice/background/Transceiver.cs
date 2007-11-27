// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Net.Sockets;

internal class Transceiver : IceInternal.Transceiver
{
    public Socket fd()
    {
        return _transceiver.fd();
    }

    public IceInternal.SocketStatus initialize(int timeout)
    {
        if(timeout == 0)
        {
            IceInternal.SocketStatus status = _configuration.initializeSocketStatus();
            if(status == IceInternal.SocketStatus.NeedConnect)
            {
                return status;
            }
            else if(status == IceInternal.SocketStatus.NeedWrite)
            {
                if(!_initialized)
                {
                    status = _transceiver.initialize(timeout);
                    if(status != IceInternal.SocketStatus.Finished)
                    {
                        return status;
                    }
                    _initialized = true;
                }
                return IceInternal.SocketStatus.NeedWrite;
            }
            else if(status == IceInternal.SocketStatus.NeedRead)
            {
                return status;
            }
        }
        _configuration.checkInitializeException();
        if(!_initialized)
        {
            IceInternal.SocketStatus status = _transceiver.initialize(timeout);
            if(status != IceInternal.SocketStatus.Finished)
            {
                return status;
            }
            _initialized = true;
        }
        return IceInternal.SocketStatus.Finished;
    }

    public void close()
    {
        _transceiver.close();
    }

    public void shutdownWrite()
    {
        _transceiver.shutdownWrite();
    }

    public void shutdownReadWrite()
    {
        _transceiver.shutdownReadWrite();
    }

    public bool write(IceInternal.Buffer buf, int timeout)
    {
        if(!_initialized)
        {
            throw new Ice.SocketException();
        }

        if(timeout == 0)
        {
            if(!_configuration.writeReady())
            {
                return false;
            }
        }
        _configuration.checkWriteException();
        return _transceiver.write(buf, timeout);
    }

    public bool read(IceInternal.Buffer buf, int timeout)
    {
        if(!_initialized)
        {
            throw new Ice.SocketException();
        }

        if(timeout == 0)
        {
            if(!_configuration.readReady())
            {
                return false;
            }
        }
        _configuration.checkReadException();
        return _transceiver.read(buf, timeout);
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
