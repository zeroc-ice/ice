// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
final class UdpMulticastServerTransceiver implements Transceiver
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
        Thread thread;

        synchronized(this)
        {
            //
            // Close the socket first in order to interrupt the helper thread.
            //
            if(_socket != null)
            {
                _socket.close();
                _socket = null;
            }

            thread = _thread;
            _thread = null;
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
    }

    @Override
    public EndpointI bind()
    {
        //
        // The constructor binds the socket so there's not much left to do.
        //

        _endpoint = _endpoint.endpoint(this);
        return _endpoint;
    }

    @Override
    public int write(Buffer buf)
    {
        //
        // This transceiver can only read.
        //
        throw new com.zeroc.Ice.SocketException();
    }

    @Override
    public synchronized int read(Buffer buf)
    {
        if(_exception != null)
        {
            throw _exception;
            //throw (com.zeroc.Ice.LocalException) _exception.fillInStackTrace();
        }

        assert(buf.b.position() == 0);

        if(!_buffers.isEmpty())
        {
            Buffer rb = _buffers.removeFirst();
            buf.swap(rb);
            buf.b.position(buf.b.limit());
            buf.resize(buf.b.limit(), true);

            if(rb.b.hasArray())
            {
                rb.b.clear();
                _recycle.add(rb);
            }

            //
            // The read thread will temporarily stop reading if we exceed our threshold. Wake it up if
            // we've transitioned below the limit.
            //
            if(_buffers.size() == _threshold - 1)
            {
                notifyAll();
            }

            //
            // Update our Read state to indicate whether we still have more data waiting to be read.
            //
            _readyCallback.ready(SocketOperation.Read, !_buffers.isEmpty());
        }

        return SocketOperation.None;
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
            info.localAddress = _addr.getAddress().getHostAddress();
            info.localPort = _addr.getPort();
            info.rcvSize = _size;
            info.mcastAddress = _addr.getAddress().getHostAddress();
            info.mcastPort = _addr.getPort();
        }
        return info;
    }

    @Override
    public void checkSendSize(Buffer buf)
    {
        //
        // Nothing to do.
        //
    }

    @Override
    public synchronized void setBufferSize(int rcvSize, int sndSize)
    {
        setBufSize(rcvSize);
    }

    public final int effectivePort()
    {
        return _addr.getPort();
    }

    //
    // Only for use by UdpEndpointI
    //
    UdpMulticastServerTransceiver(UdpEndpointI endpoint, ProtocolInstance instance, InetSocketAddress addr,
                                  String mcastInterface)
    {
        _endpoint = endpoint;
        _instance = instance;
        _mcastInterface = mcastInterface;
        _addr = addr;

        try
        {
            //
            // The MulticastSocket constructor binds the socket and calls setReuseAddress(true).
            //
            _socket = new MulticastSocket(_addr);

            //
            // Obtain the local socket address (in case a system-assigned port was requested).
            //
            _addr = (InetSocketAddress)_socket.getLocalSocketAddress();

            //
            // Set the multicast group.
            //
            Network.setMcastGroup(_socket, _addr, _mcastInterface);

            //
            // Configure the receive buffer size.
            //
            _size = _socket.getReceiveBufferSize();
            _newSize = -1;
            setBufSize(-1);
            if(_newSize != -1)
            {
                updateBufSize();
            }

            _thread = new Thread()
            {
                public void run()
                {
                    setName("IceUDPMulticast.ReadThread");
                    runReadThread();
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
            sz = _instance.properties().getPropertyAsIntWithDefault("Ice.UDP.RcvSize", _size);
        }

        //
        // Check for sanity.
        //
        if(sz < (_udpOverhead + Protocol.headerSize))
        {
            _instance.logger().warning("Invalid Ice.UDP.RcvSize value of " + sz + " adjusted to " + _size);
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
            _socket.setReceiveBufferSize(_newSize);
            _size = _socket.getReceiveBufferSize();

            //
            // Warn if the size that was set is less than the requested size and we have not already warned.
            //
            if(_size < _newSize)
            {
                BufSizeWarnInfo winfo = _instance.getBufSizeWarn(com.zeroc.Ice.UDPEndpointType.value);
                if(!winfo.rcvWarn || winfo.rcvSize != _newSize)
                {
                    _instance.logger().warning("UDP receive buffer size: requested size of " + _newSize +
                                               " adjusted to " + _size);
                    _instance.setRcvBufSizeWarn(com.zeroc.Ice.UDPEndpointType.value, _newSize);
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

    private void runReadThread()
    {
        try
        {
            DatagramPacket p = null;

            while(true)
            {
                MulticastSocket socket;
                int size;
                Buffer buf = null;

                synchronized(this)
                {
                    //
                    // If we've read too much data, wait until the application consumes some before we read again.
                    //
                    while(_socket != null && _exception == null && _buffers.size() >= _threshold)
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

                    if(!_recycle.isEmpty())
                    {
                        buf = _recycle.removeFirst();
                    }
                    else
                    {
                        buf = new Buffer(false);
                    }

                    buf.resize(_size, false);
                }

                assert(buf.b.hasArray());

                if(p == null)
                {
                    p = new DatagramPacket(buf.b.array(), buf.b.arrayOffset(), buf.b.capacity());
                }
                else
                {
                    p.setData(buf.b.array(), buf.b.arrayOffset(), buf.b.capacity());
                }

                socket.receive(p);

                if(p.getLength() > 0)
                {
                    buf.b.limit(p.getLength());

                    synchronized(this)
                    {
                        _buffers.add(buf);
                        _readyCallback.ready(SocketOperation.Read, true);
                    }
                }
            }
        }
        catch(java.io.IOException ex)
        {
            exception(new com.zeroc.Ice.SocketException(ex));
            //
            // Mark as ready for reading so that the Ice run time will invoke read() and we can report the exception.
            //
            _readyCallback.ready(SocketOperation.Read, true);
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            exception(ex);
            //
            // Mark as ready for reading so that the Ice run time will invoke read() and we can report the exception.
            //
            _readyCallback.ready(SocketOperation.Read, true);
        }
    }

    private UdpEndpointI _endpoint = null;
    private ProtocolInstance _instance;

    private int _size;
    private int _newSize;
    private MulticastSocket _socket;
    private InetSocketAddress _addr;
    private String _mcastInterface;

    //
    // The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for the UDP header
    // to get the maximum payload.
    //
    private final static int _udpOverhead = 20 + 8;

    //
    // The maximum number of packets that we'll queue before the read thread temporarily stops reading.
    //
    private final static int _threshold = 10;

    private Thread _thread;

    private java.util.LinkedList<Buffer> _buffers = new java.util.LinkedList<>();
    private java.util.LinkedList<Buffer> _recycle = new java.util.LinkedList<>();

    private com.zeroc.Ice.LocalException _exception;
    private ReadyCallback _readyCallback;
}
