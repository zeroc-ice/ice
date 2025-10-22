// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetSocketAddress;
import java.net.MulticastSocket;
import java.nio.channels.SelectableChannel;
import java.util.LinkedList;
import java.util.List;

//
// This class is only used on Android, where the java.nio.channels.MulticastChannel interface is not
// supported.
//
// NOTE: Most of the important methods on java.net.MulticastSocket are synchronized.
//
final class UdpMulticastServerTransceiver implements Transceiver {
    @Override
    public SelectableChannel fd() {
        //
        // Android doesn't provide non-blocking APIs for UDP multicast.
        //
        return null;
    }

    @Override
    public void setReadyCallback(ReadyCallback callback) {
        assert (_readyCallback == null && callback != null);
        _readyCallback = callback;

        //
        // Start the thread only once the ready callback is set or otherwise the thread
        // might start receiving datagrams but wouldn't be able to notify the thread pool.
        //
        _thread.start();
    }

    @Override
    public int initialize(Buffer readBuffer, Buffer writeBuffer) {
        //
        // Nothing to do.
        //
        return SocketOperation.None;
    }

    @Override
    public int closing(boolean initiator, LocalException ex) {
        //
        // Nothing to do.
        //
        return SocketOperation.None;
    }

    @Override
    public void close() {
        Thread thread;

        synchronized (this) {
            //
            // Close the socket first in order to interrupt the helper thread.
            //
            if (_socket != null) {
                _socket.close();
                _socket = null;
            }

            thread = _thread;
            _thread = null;
        }

        if (thread != null) {
            try {
                thread.join();
            } catch (InterruptedException ex) {
                // Ignore.
            }
        }
    }

    @Override
    public EndpointI bind() {
        //
        // The constructor binds the socket so there's not much left to do.
        //

        _endpoint = _endpoint.endpoint(this);
        return _endpoint;
    }

    @Override
    public int write(Buffer buf) {
        //
        // This transceiver can only read.
        //
        throw new SocketException();
    }

    @Override
    public synchronized int read(Buffer buf) {
        if (_exception != null) {
            throw _exception;
            // throw (LocalException) _exception.fillInStackTrace();
        }

        assert (buf.b.position() == 0);

        if (!_buffers.isEmpty()) {
            Buffer rb = _buffers.removeFirst();
            buf.swap(rb);
            buf.position(buf.b.limit());
            buf.resize(buf.b.limit(), true);

            if (rb.b.hasArray()) {
                rb.b.clear();
                _recycle.add(rb);
            }

            //
            // The read thread will temporarily stop reading if we exceed our threshold. Wake it up
            // if we've transitioned below the limit.
            //
            if (_buffers.size() == _threshold - 1) {
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
    public String protocol() {
        return _instance.protocol();
    }

    @Override
    public synchronized String toString() {
        if (_socket == null) {
            return "<closed>";
        }

        return "multicast address = " + Network.addrToString(_addr);
    }

    @Override
    public String toDetailedString() {
        StringBuilder s = new StringBuilder(toString());
        List<String> intfs =
            Network.getInterfacesForMulticast(
                _mcastInterface, Network.getProtocolSupport(_addr));
        if (!intfs.isEmpty()) {
            s.append("\nlocal interfaces = ");
            s.append(String.join(", ", intfs));
        }
        return s.toString();
    }

    @Override
    public synchronized ConnectionInfo getInfo(
            boolean incoming, String adapterName, String connectionId) {
        return new UDPConnectionInfo(
            incoming,
            adapterName,
            connectionId,
            _socket != null ? _socket.getLocalAddress().getHostAddress() : "",
            _socket != null ? _socket.getLocalPort() : -1,
            "",
            -1,
            _socket != null ? _addr.getAddress().getHostAddress() : "",
            _socket != null ? _addr.getPort() : -1,
            _size,
            0);
    }

    @Override
    public void checkSendSize(Buffer buf) {
        //
        // Nothing to do.
        //
    }

    @Override
    public synchronized void setBufferSize(int rcvSize, int sndSize) {
        setBufSize(rcvSize);
    }

    public final int effectivePort() {
        return _addr.getPort();
    }

    //
    // Only for use by UdpEndpointI
    //
    UdpMulticastServerTransceiver(
            UdpEndpointI endpoint,
            ProtocolInstance instance,
            InetSocketAddress addr,
            String mcastInterface) {
        _endpoint = endpoint;
        _instance = instance;
        _mcastInterface = mcastInterface;
        _addr = addr;

        try {
            //
            // The MulticastSocket constructor binds the socket and calls setReuseAddress(true).
            //
            _socket = new MulticastSocket(_addr);

            //
            // Obtain the local socket address (in case a system-assigned port was requested).
            //
            _addr = (InetSocketAddress) _socket.getLocalSocketAddress();

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
            if (_newSize != -1) {
                updateBufSize();
            }

            _thread =
                new Thread() {
                    public void run() {
                        setName("IceUDPMulticast.ReadThread");
                        runReadThread();
                    }
                };
        } catch (Exception ex) {
            if (_socket != null) {
                _socket.close();
            }
            _socket = null;
            if (ex instanceof LocalException) {
                throw (LocalException) ex;
            } else {
                throw new SocketException(ex);
            }
        }
    }

    private synchronized void exception(LocalException ex) {
        if (_exception == null) {
            _exception = ex;
        }
    }

    private void setBufSize(int sz) {
        assert (_socket != null);

        //
        // Get property for buffer size if size not passed in.
        //
        if (sz == -1) {
            sz = _instance.properties().getPropertyAsIntWithDefault("Ice.UDP.RcvSize", _size);
        }

        //
        // Check for sanity.
        //
        if (sz < (_udpOverhead + Protocol.headerSize)) {
            _instance
                .logger()
                .warning("Invalid Ice.UDP.RcvSize value of " + sz + " adjusted to " + _size);
        } else if (sz != _size) {
            _newSize = sz;
        }

        //
        // Defer the actual modification of the buffer size to the helper thread.
        //
    }

    private void updateBufSize() {
        //
        // Must be called without any other threads holding the lock to the MulticastSocket!
        //

        try {
            //
            // Try to set the buffer size. The kernel will silently adjust the size to an acceptable
            // value. Then read the size back to get the size that was actually set.
            //
            _socket.setReceiveBufferSize(_newSize);
            _size = _socket.getReceiveBufferSize();

            //
            // Warn if the size that was set is less than the requested size and we have not already
            // warned.
            //
            if (_size < _newSize) {
                BufSizeWarnInfo winfo = _instance.getBufSizeWarn(UDPEndpointType.value);
                if (!winfo.rcvWarn || winfo.rcvSize != _newSize) {
                    _instance
                        .logger()
                        .warning(
                            "UDP receive buffer size: requested size of "
                                + _newSize
                                + " adjusted to "
                                + _size);
                    _instance.setRcvBufSizeWarn(UDPEndpointType.value, _newSize);
                }
            }
        } catch (IOException ex) {
            if (_socket != null) {
                _socket.close();
            }
            _socket = null;
            throw new SocketException(ex);
        }
    }

    private void runReadThread() {
        try {
            DatagramPacket p = null;

            while (true) {
                MulticastSocket socket;
                Buffer buf = null;

                synchronized (this) {
                    //
                    // If we've read too much data, wait until the application consumes some before
                    // we read again.
                    //
                    while (_socket != null && _exception == null && _buffers.size() >= _threshold) {
                        try {
                            wait();
                        } catch (InterruptedException ex) {
                            break;
                        }
                    }

                    if (_socket == null || _exception != null) {
                        break;
                    }

                    if (_newSize != -1) {
                        //
                        // Application must have called setBufferSize.
                        //
                        updateBufSize();
                        _newSize = -1;
                    }

                    socket = _socket;

                    if (!_recycle.isEmpty()) {
                        buf = _recycle.removeFirst();
                    } else {
                        buf = new Buffer(false);
                    }

                    buf.resize(_size, false);
                }

                assert (buf.b.hasArray());

                if (p == null) {
                    p = new DatagramPacket(buf.b.array(), buf.b.arrayOffset(), buf.b.capacity());
                } else {
                    p.setData(buf.b.array(), buf.b.arrayOffset(), buf.b.capacity());
                }

                socket.receive(p);

                if (p.getLength() > 0) {
                    buf.limit(p.getLength());

                    synchronized (this) {
                        _buffers.add(buf);
                        _readyCallback.ready(SocketOperation.Read, true);
                    }
                }
            }
        } catch (IOException ex) {
            exception(new SocketException(ex));
            //
            // Mark as ready for reading so that the Ice run time will invoke read() and we can
            // report the exception.
            //
            _readyCallback.ready(SocketOperation.Read, true);
        } catch (LocalException ex) {
            exception(ex);
            //
            // Mark as ready for reading so that the Ice run time will invoke read() and we can
            // report the exception.
            //
            _readyCallback.ready(SocketOperation.Read, true);
        }
    }

    private UdpEndpointI _endpoint;
    private final ProtocolInstance _instance;

    private int _size;
    private int _newSize;
    private MulticastSocket _socket;
    private InetSocketAddress _addr;
    private final String _mcastInterface;

    //
    // The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for
    // the
    // UDP header
    // to get the maximum payload.
    //
    private static final int _udpOverhead = 20 + 8;

    //
    // The maximum number of packets that we'll queue before the read thread temporarily stops
    // reading.
    //
    private static final int _threshold = 10;

    private Thread _thread;

    private final LinkedList<Buffer> _buffers = new LinkedList<>();
    private final LinkedList<Buffer> _recycle = new LinkedList<>();

    private LocalException _exception;
    private ReadyCallback _readyCallback;
}
