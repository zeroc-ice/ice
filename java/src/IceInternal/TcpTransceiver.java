// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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
        if (_traceLevels.network >= 1)
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
        catch (java.io.IOException ex)
        {
        }
        try
        {
            socket.shutdownOutput();
        }
        catch (java.io.IOException ex)
        {
        }
        try
        {
            fd.close();
        }
        catch (java.io.IOException ex)
        {
        }
    }

    public void
    shutdown()
    {
        if (_traceLevels.network >= 1)
        {
            String s = "shutting down tcp connection\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        java.net.Socket socket = _fd.socket();
        try
        {
            socket.shutdownOutput(); // Shutdown socket for writing
        }
        catch (java.io.IOException ex)
        {
        }
    }

    public void
    write(java.nio.ByteBuffer buf, int timeout)
    {
        while (buf.hasRemaining())
        {
            try
            {
                int ret = _fd.write(buf);

                /* TODO: Review
                if (ret == 0)
                {
                    throw new Ice.ConnectionLostException();
                }
                */

                if (_traceLevels.network >= 3)
                {
                    String s = "sent " + ret + " of " + buf.limit() +
                        " bytes via tcp\n" + toString();
                    _logger.trace(_traceLevels.networkCat, s);
                }
            }
            catch (java.io.InterruptedIOException ex)
            {
                continue;
            }
            catch (java.io.IOException ex)
            {
                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }
        }
    }

    public void
    read(java.nio.ByteBuffer buf, int timeout)
    {
        while (buf.hasRemaining())
        {
            try
            {
                int ret = _fd.read(buf);

                if (ret == -1)
                {
                    throw new Ice.ConnectionLostException();
                }

                if (_traceLevels.network >= 3)
                {
                    String s = "received " + ret + " of " + buf.limit() +
                        " bytes via tcp\n" + toString();
                    _logger.trace(_traceLevels.networkCat, s);
                }
            }
            catch (java.io.InterruptedIOException ex)
            {
                continue;
            }
            catch (java.io.IOException ex)
            {
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
        super.finalize();
    }

    private Instance _instance;
    private java.nio.channels.SocketChannel _fd;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
}
