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

final class UdpTransceiver implements Transceiver
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
            String s;
            if (_sender)
            {
                s = "stopping to send udp packets to " + toString();
            }
            else
            {
                s = "stopping to receive udp packets at " + toString();
            }
            _logger.trace(_traceLevels.networkCat, s);
        }

        java.nio.channels.DatagramChannel fd = _fd;
        _fd = null;
        java.net.DatagramSocket socket = fd.socket();
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
    }

    public void
    write(java.nio.ByteBuffer buf, int timeout)
    {
        assert(_sender);
        assert(buf.position() == 0);
        final int packetSize = 64 * 1024; // TODO: configurable
        assert(packetSize >= buf.limit()); // TODO: exception

        while (buf.hasRemaining())
        {
            try
            {
                int ret = _fd.write(buf);

                if (_traceLevels.network >= 3)
                {
                    String s = "sent " + ret + " bytes via udp to " +
                        toString();
                    _logger.trace(_traceLevels.networkCat, s);
                }

                assert(ret == buf.limit());
                break;
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
        assert(false);

        //
        // TODO: Server
        //
        // Note: A ByteBuffer cannot be resized!
        //
    }

    public String
    toString()
    {
        return Network.addrToString(_addr);
    }

    public boolean
    equivalent(String host, int port)
    {
        if (_sender)
        {
            return false;
        }

        java.net.InetSocketAddress addr = Network.getAddress(host, port);
        if (addr.getAddress().isLoopbackAddress())
        {
            return port == _addr.getPort();
        }

        java.net.InetSocketAddress localAddr =
            Network.getLocalAddress(_addr.getPort());
        return addr.equals(localAddr);
    }

    public final int
    effectivePort()
    {
        return _addr.getPort();
    }

    //
    // Only for use by UdpEndpoint
    //
    UdpTransceiver(Instance instance, String host, int port)
    {
        _instance = instance;
        _traceLevels = instance.traceLevels();
        _logger = instance.logger();
        _sender = true;

        try
        {
            _addr = Network.getAddress(host, port);
            _fd = Network.createUdpSocket();
            Network.doConnect(_fd, _addr, -1);

            if (_traceLevels.network >= 1)
            {
                String s = "starting to send udp packets to " + toString();
                _logger.trace(_traceLevels.networkCat, s);
            }
        }
        catch (Ice.LocalException ex)
        {
            _fd = null;
            throw ex;
        }
    }

    /* TODO: Server
    //
    // Only for use by UdpEndpoint
    //
    UdpTransceiver(Instance instance, int port)
    {
        _instance = instance;
        _traceLevels = instance.traceLevels();
        _logger = instance.logger();
        _sender = false;

        try
        {
            _addr = new java.net.InetSocketAddress(port);
            _fd = Network.createUdpSocket();
            Network.doBind(_fd, _addr);

            if (_traceLevels.network >= 1)
            {
                String s = "starting to receive udp packets at " + toString();
                _logger.trace(_traceLevels.networkCat, s);
            }
        }
        catch (Ice.LocalException ex)
        {
            _fd = null;
            throw ex;
        }
    }
    */

    protected void
    finalize()
        throws Throwable
    {
        assert(_fd == null);
        super.finalize();
    }

    private Instance _instance;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private boolean _sender;
    private java.nio.channels.DatagramChannel _fd;
    private java.net.InetSocketAddress _addr;
}
