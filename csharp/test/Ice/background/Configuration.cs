//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

internal class Configuration
{
    public void ConnectorsException(System.Exception? ex)
    {
        lock (this)
        {
            _connectorsException = ex;
        }
    }

    public void CheckConnectorsException()
    {
        lock (this)
        {
            if (_connectorsException != null)
            {
                throw _connectorsException;
            }
        }
    }

    public void ConnectException(System.Exception? ex)
    {
        lock (this)
        {
            _connectException = ex;
        }
    }

    public void CheckConnectException()
    {
        lock (this)
        {
            if (_connectException != null)
            {
                throw _connectException;
            }
        }
    }

    public void InitializeException(System.Exception? ex)
    {
        lock (this)
        {
            _initializeException = ex;
        }
    }

    public void CheckInitializeException()
    {
        lock (this)
        {
            if (_initializeException != null)
            {
                throw _initializeException;
            }
        }
    }

    public void ReadReady(bool ready)
    {
        lock (this)
        {
            _readReadyCount = ready ? 0 : 10;
        }
    }

    public void ReadException(System.Exception? ex)
    {
        lock (this)
        {
            _readException = ex;
        }
    }

    public bool ReadReady()
    {
        lock (this)
        {
            if (_readReadyCount == 0)
            {
                return true;
            }
            --_readReadyCount;
            return false;
        }
    }

    public void CheckReadException()
    {
        lock (this)
        {
            if (_readException != null)
            {
                throw _readException;
            }
        }
    }

    public void WriteReady(bool ready)
    {
        lock (this)
        {
            _writeReadyCount = ready ? 0 : 10;
        }
    }

    public void WriteException(System.Exception? ex)
    {
        lock (this)
        {
            _writeException = ex;
        }
    }

    public bool WriteReady()
    {
        lock (this)
        {
            if (_writeReadyCount == 0)
            {
                return true;
            }
            --_writeReadyCount;
            return false;
        }
    }

    public void CheckWriteException()
    {
        lock (this)
        {
            if (_writeException != null)
            {
                throw _writeException;
            }
        }
    }

    public void Buffered(bool b) => _buffered = b;

    public bool Buffered() => _buffered;

    static public Configuration GetInstance() => _instance;

    private System.Exception? _connectorsException;
    private System.Exception? _connectException;
    private System.Exception? _initializeException;
    private int _readReadyCount;
    private System.Exception? _readException;
    private int _writeReadyCount;
    private System.Exception? _writeException;
    private bool _buffered;

    private static readonly Configuration _instance = new Configuration();
}
