// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

internal class Configuration
{
    public void connectorsException(Ice.LocalException ex)
    {
        lock(this)
        {
            _connectorsException = ex;
        }
    }

    public void checkConnectorsException()
    {
        lock(this)
        {
            if(_connectorsException != null)
            {
                throw _connectorsException;
            }
        }
    }

    public void connectException(Ice.LocalException ex)
    {
        lock(this)
        {
            _connectException = ex;
        }
    }

    public void checkConnectException()
    {
        lock(this)
        {
            if(_connectException != null)
            {
                throw _connectException;
            }
        }
    }

    public void initializeException(Ice.LocalException ex)
    {
        lock(this)
        {
            _initializeException = ex;
        }
    }

    public void checkInitializeException()
    {
        lock(this)
        {
            if(_initializeException != null)
            {
                throw _initializeException;
            }
        }
    }

    public void readReady(bool ready)
    {
        lock(this)
        {
            _readReadyCount = ready ? 0 : 10;
        }
    }

    public void readException(Ice.LocalException ex)
    {
        lock(this)
        {
            _readException = ex;
        }
    }

    public bool readReady()
    {
        lock(this)
        {
            if(_readReadyCount == 0)
            {
                return true;
            }
            --_readReadyCount;
            return false;
        }
    }

    public void checkReadException()
    {
        lock(this)
        {
            if(_readException != null)
            {
                throw _readException;
            }
        }
    }

    public void writeReady(bool ready)
    {
        lock(this)
        {
            _writeReadyCount = ready ? 0 : 10;
        }
    }

    public void writeException(Ice.LocalException ex)
    {
        lock(this)
        {
            _writeException = ex;
        }
    }

    public bool writeReady()
    {
        lock(this)
        {
            if(_writeReadyCount == 0)
            {
                return true;
            }
            --_writeReadyCount;
            return false;
        }
    }

    public void checkWriteException()
    {
        lock(this)
        {
            if(_writeException != null)
            {
                throw _writeException;
            }
        }
    }

    public void buffered(bool b)
    {
        _buffered = b;
    }

    public bool buffered()
    {
        return _buffered;
    }

    static public Configuration getInstance()
    {
        return _instance;
    }

    private Ice.LocalException _connectorsException;
    private Ice.LocalException _connectException;
    private Ice.LocalException _initializeException;
    private int _readReadyCount;
    private Ice.LocalException _readException;
    private int _writeReadyCount;
    private Ice.LocalException _writeException;
    private bool _buffered;

    private static Configuration _instance = new Configuration();
}
