// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package IceInternal;

final class TcpTransceiver implements Transceiver
{
    public java.nio.channels.SelectableChannel
    fd()
    {
        assert(_fd != null);
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

        assert(_fd != null);
        java.net.Socket socket = _fd.socket();
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
            _fd.close();
        }
        catch(java.io.IOException ex)
        {
        }
        _fd = null;
    }

    public void
    shutdown()
    {
        if(_traceLevels.network >= 2)
        {
            String s = "shutting down tcp connection\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        assert(_fd != null);
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
                assert(_fd != null);
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
                assert(_fd != null);
                int ret = _fd.read(buf);

                if(ret == -1)
                {
                    throw new Ice.ConnectionLostException();
                }

                if(ret == 0)
                {
                    assert(_fd != null);

                    if(_selector == null)
                    {
                        _selector = java.nio.channels.Selector.open();
                        _fd.register(_selector, java.nio.channels.SelectionKey.OP_READ, null);
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

    private java.nio.channels.SocketChannel _fd;
    private java.nio.channels.Selector _selector;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
}
