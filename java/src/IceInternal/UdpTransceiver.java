// **********************************************************************
//
// Copyright (c) 2002
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

final class UdpTransceiver implements Transceiver
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
            String s = "closing udp connection\n" + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        assert(_fd != null);
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
    }

    public void
    write(BasicStream stream, int timeout)
    {
	// TODO: Timeouts are ignored!!

        java.nio.ByteBuffer buf = stream.prepareWrite();

        assert(buf.position() == 0);
        final int packetSize = 64 * 1024; // TODO: configurable
        assert(packetSize >= buf.limit()); // TODO: exception

        while(buf.hasRemaining())
        {
            try
            {
                assert(_fd != null);
                int ret = _fd.write(buf);

                if(_traceLevels.network >= 3)
                {
                    String s = "sent " + ret + " bytes via udp\n" + toString();
                    _logger.trace(_traceLevels.networkCat, s);
                }

                assert(ret == buf.limit());
                break;
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
	// TODO: Timeouts are ignored!!

        assert(stream.pos() == 0);
        final int packetSize = 64 * 1024; // TODO: configurable
        stream.resize(packetSize, true);
        java.nio.ByteBuffer buf = stream.prepareRead();
        buf.position(0);

        int ret = 0;
        while(true)
        {
            if(_connect)
            {
                //
                // If we must connect, then we connect to the first peer that
                // sends us a packet.
                //
                try
                {
                    assert(_fd != null);
                    java.net.InetSocketAddress peerAddr = (java.net.InetSocketAddress)_fd.receive(buf);
                    ret = buf.position();
                    Network.doConnect(_fd, peerAddr, -1);
                    _connect = false; // We're connected now

                    if(_traceLevels.network >= 1)
                    {
                        String s = "connected udp socket\n" + toString();
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
            else
            {
                try
                {
                    assert(_fd != null);
                    _fd.receive(buf);
                    ret = buf.position();
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

            break;
        }

        if(_traceLevels.network >= 3)
        {
            String s = "received " + ret + " bytes via udp\n" + toString();
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
        java.net.InetSocketAddress addr = Network.getAddress(host, port);
        return addr.equals(_addr);
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
        _traceLevels = instance.traceLevels();
        _logger = instance.logger();
        _incoming = false;
        _connect = true;

        try
        {
            _fd = Network.createUdpSocket();
            Network.setBlock(_fd, false);
            _addr = Network.getAddress(host, port);
            Network.doConnect(_fd, _addr, -1);
            _connect = false; // We're connected now

            if(_traceLevels.network >= 1)
            {
                String s = "starting to send udp packets\n" + toString();
                _logger.trace(_traceLevels.networkCat, s);
            }
        }
        catch(Ice.LocalException ex)
        {
            _fd = null;
            throw ex;
        }
    }

    //
    // Only for use by UdpEndpoint
    //
    UdpTransceiver(Instance instance, String host, int port, boolean connect)
    {
        _traceLevels = instance.traceLevels();
        _logger = instance.logger();
        _incoming = true;
        _connect = connect;

        try
        {
            _fd = Network.createUdpSocket();
            Network.setBlock(_fd, false);
            _addr = new java.net.InetSocketAddress(host, port);
	    if(_traceLevels.network >= 2)
	    {
		String s = "attempting to bind to udp socket\n" + toString();
		_logger.trace(_traceLevels.networkCat, s);
	    }
            _addr = Network.doBind(_fd, _addr);

            if(_traceLevels.network >= 1)
            {
                String s = "starting to receive udp packets\n" + toString();
                _logger.trace(_traceLevels.networkCat, s);
            }
        }
        catch(Ice.LocalException ex)
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

    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private boolean _incoming;
    private boolean _connect;
    private java.nio.channels.DatagramChannel _fd;
    private java.net.InetSocketAddress _addr;
}
