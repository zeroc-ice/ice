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
            String s = "closing udp connection\n" + toString();
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
    write(BasicStream stream, int timeout)
    {
        java.nio.ByteBuffer buf = stream.prepareWrite();

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
                    String s = "sent " + ret + " bytes via " + _protocolName + "\n" + toString();
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
    read(BasicStream stream, int timeout)
    {
        assert(stream.pos() == 0);
        final int packetSize = 64 * 1024; // TODO: configurable
        stream.resize(packetSize, true);
        java.nio.ByteBuffer buf = stream.prepareRead();
        buf.position(0);

        int ret = 0;
        while (true)
        {
            if (_connect)
            {
                //
                // If we must connect, then we connect to the first peer that
                // sends us a packet.
                //
                try
                {
                    java.net.InetSocketAddress peerAddr = (java.net.InetSocketAddress)_fd.receive(buf);
                    ret = buf.position();
                    Network.doConnect(_fd, peerAddr, -1);
                    _connect = false; // We're connected now

                    if (_traceLevels.network >= 1)
                    {
                        String s = "connected " + _protocolName + "socket\n" + toString();
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
            else
            {
                try
                {
                    _fd.receive(buf);
                    ret = buf.position();
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

            break;
        }

        if (_traceLevels.network >= 3)
        {
            String s = "received " + ret + " bytes via " + _protocolName + "\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        stream.resize(ret, true);
        stream.pos(ret);
    }

    public String
    toString()
    {
        return Network.fdToString(_fd);
    }

    public final boolean
    equivalent(String host, int port)
    {
        assert(_incoming); // This equivalence test is only valid for incoming connections.

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

    public final void
    setProtocolName(String protocolName)
    {
        _protocolName = protocolName;
    }

    //
    // Only for use by UdpEndpoint
    //
    UdpTransceiver(Instance instance,
                   String host,
                   int port)
    {
        this(instance, host, port, "udp");
    }

    //
    // Only for use by UdpEndpoint
    //
    UdpTransceiver(Instance instance,
                   String host,
                   int port,
                   String protocolName)
    {
        _instance = instance;
        _traceLevels = instance.traceLevels();
        _logger = instance.logger();
        _incoming = false;
        _connect = true;
        _protocolName = protocolName;

        try
        {
            _addr = Network.getAddress(host, port);
            _fd = Network.createUdpSocket();
            Network.doConnect(_fd, _addr, -1);
            _connect = false; // We're connected now

            if (_traceLevels.network >= 1)
            {
                String s = "starting to send " + _protocolName + " packets\n" + toString();
                _logger.trace(_traceLevels.networkCat, s);
            }
        }
        catch (Ice.LocalException ex)
        {
            _fd = null;
            throw ex;
        }
    }

    //
    // Only for use by UdpEndpoint
    //
    UdpTransceiver(Instance instance,
                   int port,
                   boolean connect)
    {
        this(instance, port, connect, "udp");
    }

    //
    // Only for use by UdpEndpoint
    //
    UdpTransceiver(Instance instance,
                   int port,
                   boolean connect,
                   String protocolName)
    {
        _instance = instance;
        _traceLevels = instance.traceLevels();
        _logger = instance.logger();
        _incoming = true;
        _connect = connect;
        _protocolName = protocolName;

        try
        {
            _addr = new java.net.InetSocketAddress(port);
            _fd = Network.createUdpSocket();
            Network.doBind(_fd, _addr);

            if (_traceLevels.network >= 1)
            {
                String s = "starting to receive " + _protocolName + " packets\n" + toString();
                _logger.trace(_traceLevels.networkCat, s);
            }
        }
        catch (Ice.LocalException ex)
        {
            _fd = null;
            throw ex;
        }
    }

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
    private boolean _incoming;
    private boolean _connect;
    private java.nio.channels.DatagramChannel _fd;
    private java.net.InetSocketAddress _addr;
    private String _protocolName;
}
