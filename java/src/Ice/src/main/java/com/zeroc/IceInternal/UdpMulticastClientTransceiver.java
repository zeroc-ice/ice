// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import java.net.DatagramPacket;
import java.net.InetSocketAddress;
import java.net.MulticastSocket;

//
// This class is only used on Android, where the java.nio.channels.MulticastChannel interface is not supported.
//
// NOTE: Most of the important methods on java.net.MulticastSocket are synchronized.
//
final class UdpMulticastClientTransceiver implements Transceiver
{
    @Override
    public java.nio.channels.SelectableChannel fd()
    {
        //
        // Android doesn't provide non-blocking APIs for UDP multicast.
        //
        return null;
    }

    @Override
    public void setReadyCallback(ReadyCallback callback)
    {
        _readyCallback = callback;
    }

    @Override
    public int initialize(Buffer readBuffer, Buffer writeBuffer)
    {
        //
        // Nothing to do.
        //
        return SocketOperation.None;
    }

    @Override
    public int closing(boolean initiator, com.zeroc.Ice.LocalException ex)
    {
        //
        // Nothing to do.
        //
        return SocketOperation.None;
    }

    @Override
    public void close()
    {
        MulticastSocket socket;
        Thread thread;

        synchronized(this)
        {
            socket = _socket;
            _socket = null;

            thread = _thread;
            _thread = null;

            if(thread != null)
            {
                notifyAll(); // Wake up the thread.
            }
        }

        if(thread != null)
        {
            try
            {
                thread.join();
            }
            catch(InterruptedException ex)
            {
                // Ignore.
            }
        }

        if(socket != null)
        {
            socket.close();
        }
    }

    @Override
    public EndpointI bind()
    {
        //
        // Nothing to do for a client transceiver.
        //
        return null;
    }

    @Override
    public synchronized int write(Buffer buf)
    {
        if(_exception != null)
        {
            throw _exception;
            //throw (com.zeroc.Ice.LocalException) _exception.fillInStackTrace();
        }

        if(!buf.b.hasRemaining())
        {
            return SocketOperation.None;
        }

        assert(buf.b.position() == 0);
        assert(_socket != null);

        //
        // The caller is supposed to check the send size before by calling checkSendSize.
        //
        assert(java.lang.Math.min(_maxPacketSize, _size - _udpOverhead) >= buf.size());

        //
        // Queue the buffer for processing by the write thread.
        //
        _buffers.add(new Buffer(buf, true));
        notifyAll();

        return SocketOperation.None;
    }

    @Override
    public synchronized int read(Buffer buf)
    {
        //
        // This transceiver can only write.
        //
        throw new com.zeroc.Ice.SocketException();
    }

    @Override
    public String protocol()
    {
        return _instance.protocol();
    }

    @Override
    public synchronized String toString()
    {
        if(_socket == null)
        {
            return "<closed>";
        }

        return "multicast address = " + Network.addrToString(_addr);
    }

    @Override
    public String toDetailedString()
    {
        StringBuilder s = new StringBuilder(toString());
        java.util.List<String> intfs = Network.getInterfacesForMulticast(_mcastInterface,
                                                                         Network.getProtocolSupport(_addr));
        if(!intfs.isEmpty())
        {
            s.append("\nlocal interfaces = ");
            s.append(com.zeroc.IceUtilInternal.StringUtil.joinString(intfs, ", "));
        }
        return s.toString();
    }

    @Override
    public synchronized com.zeroc.Ice.ConnectionInfo getInfo()
    {
        com.zeroc.Ice.UDPConnectionInfo info = new com.zeroc.Ice.UDPConnectionInfo();
        if(_socket != null)
        {
            info.mcastAddress = _addr.getAddress().getHostAddress();
            info.mcastPort = _addr.getPort();
            info.sndSize = _size;
        }
        return info;
    }

    @Override
    public synchronized void checkSendSize(Buffer buf)
    {
        //
        // The maximum packetSize is either the maximum allowable UDP packet size, or
        // the UDP send buffer size (whichever is smaller).
        //
        final int packetSize = java.lang.Math.min(_maxPacketSize, _size - _udpOverhead);
        if(packetSize < buf.size())
        {
            throw new com.zeroc.Ice.DatagramLimitException();
        }
    }

    @Override
    public synchronized void setBufferSize(int rcvSize, int sndSize)
    {
        setBufSize(sndSize);
    }

    //
    // Only for use by UdpConnector
    //
    UdpMulticastClientTransceiver(ProtocolInstance instance, InetSocketAddress addr, String mcastInterface,
                                  int mcastTtl)
    {
        assert(addr.getAddress().isMulticastAddress());

        _instance = instance;
        _addr = addr;
        _mcastInterface = mcastInterface;

        try
        {
            _socket = new MulticastSocket();

            //
            // Configure the send buffer size.
            //
            _size = _socket.getSendBufferSize();
            _newSize = -1;
            setBufSize(-1);
            if(_newSize != -1)
            {
                updateBufSize();
            }

            //
            // NOTE: Setting the multicast interface before performing the connect is important for some systems
            // such as macOS.
            //
            if(mcastInterface.length() > 0)
            {
                _socket.setNetworkInterface(Network.getInterface(mcastInterface));
            }
            if(mcastTtl != -1)
            {
                _socket.setTimeToLive(mcastTtl);
            }

            _socket.connect(addr); // Does not block

            _thread = new Thread()
            {
                public void run()
                {
                    setName("IceUDPMulticast.WriteThread");
                    runWriteThread();
                }
            };
            _thread.start();
        }
        catch(Exception ex)
        {
            if(_socket != null)
            {
                _socket.close();
            }
            _socket = null;
            if(ex instanceof com.zeroc.Ice.LocalException)
            {
                throw (com.zeroc.Ice.LocalException)ex;
            }
            else
            {
                throw new com.zeroc.Ice.SocketException(ex);
            }
        }
    }

    private synchronized void exception(com.zeroc.Ice.LocalException ex)
    {
        if(_exception == null)
        {
            _exception = ex;
        }
    }

    private void setBufSize(int sz)
    {
        assert(_socket != null);

        //
        // Get property for buffer size if size not passed in.
        //
        if(sz == -1)
        {
            sz = _instance.properties().getPropertyAsIntWithDefault("Ice.UDP.SndSize", _size);
        }

        //
        // Check for sanity.
        //
        if(sz < (_udpOverhead + Protocol.headerSize))
        {
            _instance.logger().warning("Invalid Ice.UDP.SndSize value of " + sz + " adjusted to " + _size);
        }
        else if(sz != _size)
        {
            _newSize = sz;
        }

        //
        // Defer the actual modification of the buffer size to the helper thread.
        //
    }

    private void updateBufSize()
    {
        //
        // Must be called without any other threads holding the lock to the MulticastSocket!
        //

        try
        {
            //
            // Try to set the buffer size. The kernel will silently adjust the size to an acceptable value.
            // Then read the size back to get the size that was actually set.
            //
            _socket.setSendBufferSize(_newSize);
            _size = _socket.getSendBufferSize();

            //
            // Warn if the size that was set is less than the requested size and we have not already warned.
            //
            if(_size < _newSize)
            {
                BufSizeWarnInfo winfo = _instance.getBufSizeWarn(com.zeroc.Ice.UDPEndpointType.value);
                if(!winfo.sndWarn || winfo.sndSize != _newSize)
                {
                    _instance.logger().warning("UDP send buffer size: requested size of " + _newSize +
                                               " adjusted to " + _size);
                    _instance.setSndBufSizeWarn(com.zeroc.Ice.UDPEndpointType.value, _newSize);
                }
            }
        }
        catch(java.io.IOException ex)
        {
            if(_socket != null)
            {
                _socket.close();
            }
            _socket = null;
            throw new com.zeroc.Ice.SocketException(ex);
        }
    }

    @SuppressWarnings("deprecation")
    @Override
    protected synchronized void finalize()
        throws Throwable
    {
        try
        {
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_socket == null);
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    private void runWriteThread()
    {
        try
        {
            DatagramPacket p = new DatagramPacket(new byte[0], 0);
            p.setSocketAddress(_addr);

            while(true)
            {
                MulticastSocket socket;
                Buffer buf;

                synchronized(this)
                {
                    //
                    // Wait until the socket is closed, an exception occurs, or we have something to write.
                    //
                    while(_socket != null && _exception == null && _buffers.isEmpty())
                    {
                        try
                        {
                            wait();
                        }
                        catch(InterruptedException ex)
                        {
                            break;
                        }
                    }

                    if(_socket == null || _exception != null)
                    {
                        break;
                    }

                    if(_newSize != -1)
                    {
                        //
                        // Application must have called setBufferSize.
                        //
                        updateBufSize();
                        _newSize = -1;
                    }

                    socket = _socket;
                    buf = _buffers.removeFirst();
                }

                assert(buf != null);
                if(buf.b.hasRemaining())
                {
                    byte[] arr;
                    int offset;
                    if(buf.b.hasArray())
                    {
                        arr = buf.b.array();
                        offset = buf.b.arrayOffset();
                    }
                    else
                    {
                        //
                        // If the buffer doesn't have a backing array, we'll have to make a copy of the data.
                        //
                        arr = new byte[buf.b.limit()];
                        offset = 0;
                        buf.b.get(arr);
                    }

                    p.setData(arr, offset, buf.b.limit());
                    socket.send(p);
                }

                synchronized(this)
                {
                    //
                    // After the write is complete, indicate whether we can accept more data.
                    //
                    _readyCallback.ready(SocketOperation.Write, !_buffers.isEmpty());
                }
            }
        }
        catch(java.io.IOException ex)
        {
            exception(new com.zeroc.Ice.SocketException(ex));
        }
    }

    private ProtocolInstance _instance;
    private InetSocketAddress _addr;
    private String _mcastInterface;

    private MulticastSocket _socket;
    private int _size;
    private int _newSize;

    //
    // The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for the UDP header
    // to get the maximum payload.
    //
    private final static int _udpOverhead = 20 + 8;
    private final static int _maxPacketSize = 65535 - _udpOverhead;

    private Thread _thread;

    private java.util.LinkedList<Buffer> _buffers = new java.util.LinkedList<>();

    private com.zeroc.Ice.LocalException _exception;
    private ReadyCallback _readyCallback;
}
