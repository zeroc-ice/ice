// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
    initializeSocketStatus(IceInternal.SocketStatus status)
    {
        if(status == IceInternal.SocketStatus.Finished)
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
    
    public synchronized IceInternal.SocketStatus
    initializeSocketStatus()
    {
        if(_initializeResetCount == 0)
        {
            return IceInternal.SocketStatus.Finished;
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

    static public synchronized Configuration
    getInstance()
    {
        return _instance;
    }

    private Ice.LocalException _connectorsException;
    private Ice.LocalException _connectException;
    private IceInternal.SocketStatus _initializeSocketStatus;
    private int _initializeResetCount;
    private Ice.LocalException _initializeException;
    private int _readReadyCount;
    private Ice.LocalException _readException;
    private int _writeReadyCount;
    private Ice.LocalException _writeException;

    private final static Configuration _instance = new Configuration();
}
