// Copyright (c) ZeroC, Inc.

internal class Configuration
{
    public void connectorsException(Ice.LocalException ex)
    {
        lock (_mutex)
        {
            _connectorsException = ex;
        }
    }

    public void checkConnectorsException()
    {
        lock (_mutex)
        {
            if (_connectorsException != null)
            {
                throw _connectorsException;
            }
        }
    }

    public void connectException(Ice.LocalException ex)
    {
        lock (_mutex)
        {
            _connectException = ex;
        }
    }

    public void checkConnectException()
    {
        lock (_mutex)
        {
            if (_connectException != null)
            {
                throw _connectException;
            }
        }
    }

    public void initializeException(Ice.LocalException ex)
    {
        lock (_mutex)
        {
            _initializeException = ex;
        }
    }

    public void checkInitializeException()
    {
        lock (_mutex)
        {
            if (_initializeException != null)
            {
                throw _initializeException;
            }
        }
    }

    public void readReady(bool ready)
    {
        lock (_mutex)
        {
            _readReadyCount = ready ? 0 : 10;
        }
    }

    public void readException(Ice.LocalException ex)
    {
        lock (_mutex)
        {
            _readException = ex;
        }
    }

    public bool readReady()
    {
        lock (_mutex)
        {
            if (_readReadyCount == 0)
            {
                return true;
            }
            --_readReadyCount;
            return false;
        }
    }

    public void checkReadException()
    {
        lock (_mutex)
        {
            if (_readException != null)
            {
                throw _readException;
            }
        }
    }

    public void writeReady(bool ready)
    {
        lock (_mutex)
        {
            _writeReadyCount = ready ? 0 : 10;
        }
    }

    public void writeException(Ice.LocalException ex)
    {
        lock (_mutex)
        {
            _writeException = ex;
        }
    }

    public bool writeReady()
    {
        lock (_mutex)
        {
            if (_writeReadyCount == 0)
            {
                return true;
            }
            --_writeReadyCount;
            return false;
        }
    }

    public void checkWriteException()
    {
        lock (_mutex)
        {
            if (_writeException != null)
            {
                throw _writeException;
            }
        }
    }

    public void buffered(bool b) => _buffered = b;

    public bool buffered() => _buffered;

    public static Configuration getInstance() => _instance;

    private Ice.LocalException _connectorsException;
    private Ice.LocalException _connectException;
    private Ice.LocalException _initializeException;
    private int _readReadyCount;
    public Ice.LocalException _readException;
    private int _writeReadyCount;
    private Ice.LocalException _writeException;
    private bool _buffered;
    private readonly object _mutex = new();

    private static readonly Configuration _instance = new Configuration();
}
