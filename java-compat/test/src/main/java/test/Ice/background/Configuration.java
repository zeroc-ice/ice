// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.background;

public final class Configuration
{
    public synchronized void
    connectorsException(Ice.LocalException ex)
    {
        _connectorsException = ex;
    }

    public synchronized void
    checkConnectorsException()
    {
        if(_connectorsException != null)
        {
            throw _connectorsException;
        }
    }

    public synchronized void
    connectException(Ice.LocalException ex)
    {
        _connectException = ex;
    }

    public synchronized void
    checkConnectException()
    {
        if(_connectException != null)
        {
            throw _connectException;
        }
    }

    public synchronized void
    initializeSocketStatus(int status)
    {
        if(status == IceInternal.SocketOperation.None)
        {
            _initializeResetCount = 0;
            return;
        }
        _initializeResetCount = 10;
        _initializeSocketStatus = status;
    }

    public synchronized void
    initializeException(Ice.LocalException ex)
    {
        _initializeException = ex;
    }

    public synchronized int
    initializeSocketStatus()
    {
        if(_initializeResetCount == 0)
        {
            return IceInternal.SocketOperation.None;
        }
        --_initializeResetCount;
        return _initializeSocketStatus;
    }

    public synchronized void
    checkInitializeException()
    {
        if(_initializeException != null)
        {
            throw _initializeException;
        }
    }

    public synchronized void
    readReady(boolean ready)
    {
        _readReadyCount = ready ? 0 : 10;
    }

    public synchronized void
    readException(Ice.LocalException ex)
    {
        _readException = ex;
    }

    public synchronized boolean
    readReady()
    {
        if(_readReadyCount == 0)
        {
            return true;
        }
        --_readReadyCount;
        return false;
    }

    public synchronized void
    checkReadException()
    {
        if(_readException != null)
        {
            throw _readException;
        }
    }

    public synchronized void
    writeReady(boolean ready)
    {
        _writeReadyCount = ready ? 0 : 10;
    }

    public synchronized void
    writeException(Ice.LocalException ex)
    {
        _writeException = ex;
    }

    public synchronized boolean
    writeReady()
    {
        if(_writeReadyCount == 0)
        {
            return true;
        }
        --_writeReadyCount;
        return false;
    }

    public synchronized void
    checkWriteException()
    {
        if(_writeException != null)
        {
            throw _writeException;
        }
    }

    public synchronized void
    buffered(boolean b)
    {
        _buffered = b;
    }

    public synchronized boolean
    buffered()
    {
        return _buffered;
    }

    private Ice.LocalException _connectorsException;
    private Ice.LocalException _connectException;
    private int _initializeSocketStatus;
    private int _initializeResetCount;
    private Ice.LocalException _initializeException;
    private int _readReadyCount;
    private Ice.LocalException _readException;
    private int _writeReadyCount;
    private Ice.LocalException _writeException;
    private boolean _buffered;
}
