// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

final class TcpTransceiver implements Transceiver
{
    public java.nio.channels.SelectableChannel
    fd()
    {
        return _fd;
    }

    public void
    close()
    {
        if(_traceLevels.network >= 1)
        {
            String s = "closing tcp connection\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        java.nio.channels.SocketChannel fd = _fd;
        _fd = null;
        java.net.Socket socket = fd.socket();
        try
        {
            socket.shutdownInput(); // helps to unblock threads in recv()
        }
        catch(java.io.IOException ex)
        {
        }
        try
        {
            socket.shutdownOutput();
        }
        catch(java.io.IOException ex)
        {
        }
        try
        {
            fd.close();
        }
        catch(java.io.IOException ex)
        {
        }
    }

    public void
    shutdown()
    {
        if(_traceLevels.network >= 1)
        {
            String s = "shutting down tcp connection\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        java.net.Socket socket = _fd.socket();
        try
        {
            socket.shutdownOutput(); // Shutdown socket for writing
        }
        catch(java.io.IOException ex)
        {
        }
    }

    public void
    write(BasicStream stream, int timeout)
    {
        java.nio.ByteBuffer buf = stream.prepareWrite();
        while(buf.hasRemaining())
        {
            try
            {
                int ret = _fd.write(buf);

                /* TODO: Review
                if(ret == 0)
                {
                    throw new Ice.ConnectionLostException();
                }
                */

                if(_traceLevels.network >= 3)
                {
                    String s = "sent " + ret + " of " + buf.limit() +
                        " bytes via tcp\n" + toString();
                    _logger.trace(_traceLevels.networkCat, s);
                }
            }
            catch(java.io.InterruptedIOException ex)
            {
                continue;
            }
            catch(java.io.IOException ex)
            {
                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }
        }
    }

    public void
    read(BasicStream stream, int timeout)
    {
        java.nio.ByteBuffer buf = stream.prepareRead();

        int remaining = 0;
        if(_traceLevels.network >= 3)
        {
            remaining = buf.remaining();
        }

        while(buf.hasRemaining())
        {
            try
            {
                int ret = _fd.read(buf);

                if(ret == -1)
                {
                    throw new Ice.ConnectionLostException();
                }

                if(ret == 0)
                {
                    // Copy fd, in case another thread calls close()
                    java.nio.channels.SocketChannel fd = _fd;

                    if(_selector == null)
                    {
                        _selector = java.nio.channels.Selector.open();
                        fd.register(_selector, java.nio.channels.SelectionKey.OP_READ, null);
                    }

                    while(true)
                    {
                        try
                        {
                            int n;
                            if(timeout == 0)
                            {
                                n = _selector.selectNow();
                            }
                            else if(timeout > 0)
                            {
                                n = _selector.select(timeout);
                            }
                            else
                            {
                                n = _selector.select();
                            }

                            if(n == 0 && timeout > 0)
                            {
                                throw new Ice.TimeoutException();
                            }

                            break;
                        }
                        catch(java.io.InterruptedIOException ex)
                        {
                            continue;
                        }
                    }
                }

                if(ret > 0 && _traceLevels.network >= 3)
                {
                    String s = "received " + ret + " of " + remaining + " bytes via tcp\n" + toString();
                    _logger.trace(_traceLevels.networkCat, s);
                }
            }
            catch(java.io.InterruptedIOException ex)
            {
                continue;
            }
            catch(java.io.IOException ex)
            {
                if(Network.connectionLost(ex))
                {
                    Ice.ConnectionLostException se = new Ice.ConnectionLostException();
                    se.initCause(ex);
                    throw se;
                }

                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }
        }
    }

    public String
    toString()
    {
        return Network.fdToString(_fd);
    }

    //
    // Only for use by TcpConnector, TcpAcceptor
    //
    TcpTransceiver(Instance instance, java.nio.channels.SocketChannel fd)
    {
        _instance = instance;
        _fd = fd;
        _traceLevels = instance.traceLevels();
        _logger = instance.logger();
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_fd == null);

        if(_selector != null)
        {
            try
            {
                _selector.close();
            }
            catch(java.io.IOException ex)
            {
            }
        }

        super.finalize();
    }

    private Instance _instance;
    private java.nio.channels.SocketChannel _fd;
    private java.nio.channels.Selector _selector;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
}
