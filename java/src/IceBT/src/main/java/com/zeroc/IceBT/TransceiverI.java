// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceBT;

import com.zeroc.IceInternal.Buffer;
import com.zeroc.IceInternal.ReadyCallback;
import com.zeroc.IceInternal.SocketOperation;
import com.zeroc.IceInternal.Transceiver;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.SocketException;

import java.lang.Thread;
import java.nio.ByteBuffer;
import java.nio.channels.*;
import java.util.UUID;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;

final class TransceiverI implements Transceiver
{
    @Override
    public SelectableChannel fd()
    {
        //
        // Android doesn't provide non-blocking APIs for Bluetooth.
        //
        return null;
    }

    @Override
    public void setReadyCallback(ReadyCallback callback)
    {
        _readyCallback = callback;
    }

    @Override
    public synchronized int initialize(Buffer readBuffer, Buffer writeBuffer)
    {
        if(_exception != null)
        {
            throw _exception;
            //throw (LocalException) _exception.fillInStackTrace();
        }

        if(_state == StateConnecting)
        {
            //
            // Wait until the connect thread is finished.
            //
            return SocketOperation.Read;
        }
        else if(_state == StateConnected)
        {
            //
            // Update our Read state to indicate whether we still have more data waiting to be read.
            //
            _readyCallback.ready(SocketOperation.Read, _readBuffer.b.position() > 0);
        }

        return SocketOperation.None;
    }

    @Override
    public int closing(boolean initiator, LocalException ex)
    {
        //
        // If we are initiating the connection closure, wait for the peer
        // to close the connection. Otherwise, close immediately.
        //
        return initiator ? SocketOperation.Read : SocketOperation.None;
    }

    @Override
    public void close()
    {
        Thread connectThread = null, readThread = null, writeThread = null;

        synchronized(this)
        {
            //
            // Close the socket first in order to interrupt the helper threads.
            //
            if(_socket != null)
            {
                try
                {
                    _socket.close();
                }
                catch(java.io.IOException ex)
                {
                    // Ignore.
                }
                _socket = null;
            }

            connectThread = _connectThread;
            _connectThread = null;
            readThread = _readThread;
            _readThread = null;
            writeThread = _writeThread;
            _writeThread = null;

            _state = StateClosed;

            if(writeThread != null)
            {
                notifyAll(); // Wake up the read/write threads.
            }
        }

        if(connectThread != null)
        {
            try
            {
                connectThread.join();
            }
            catch(InterruptedException ex)
            {
                // Ignore.
            }
        }

        if(readThread != null)
        {
            try
            {
                readThread.join();
            }
            catch(InterruptedException ex)
            {
                // Ignore.
            }
        }

        if(writeThread != null)
        {
            try
            {
                writeThread.join();
            }
            catch(InterruptedException ex)
            {
                // Ignore.
            }
        }
    }

    @Override
    public com.zeroc.IceInternal.EndpointI bind()
    {
        assert(false);
        return null;
    }

    @Override
    public synchronized int write(Buffer buf)
    {
        if(_exception != null)
        {
            throw _exception;
            //throw (LocalException) _exception.fillInStackTrace();
        }

        //
        // Accept up to _sndSize bytes in our internal buffer.
        //
        final int capacity = _sndSize - _writeBuffer.b.position();
        if(capacity > 0)
        {
            final int num = Math.min(capacity, buf.b.remaining());
            _writeBuffer.expand(num);
            final int lim = buf.b.limit();       // Save the current limit.
            buf.limit(buf.b.position() + num);   // Temporarily change the limit.
            _writeBuffer.b.put(buf.b);           // Copy to our internal buffer.
            buf.limit(lim);                      // Restore the previous limit.

            notifyAll(); // We've added data to the internal buffer, so wake up the write thread.
        }

        return buf.b.hasRemaining() ? SocketOperation.Write : SocketOperation.None;
    }

    @Override
    public synchronized int read(Buffer buf)
    {
        if(_exception != null)
        {
            throw _exception;
            //throw (LocalException) _exception.fillInStackTrace();
        }

        //
        // Copy the requested amount of data from our internal buffer to the given buffer.
        //
        _readBuffer.b.flip();
        if(_readBuffer.b.hasRemaining())
        {
            int bytesAvailable = _readBuffer.b.remaining();
            int bytesNeeded = buf.b.remaining();
            if(bytesAvailable > bytesNeeded)
            {
                bytesAvailable = bytesNeeded;
            }
            if(buf.b.hasArray())
            {
                //
                // Copy directly into the destination buffer's backing array.
                //
                byte[] arr = buf.b.array();
                _readBuffer.b.get(arr, buf.b.arrayOffset() + buf.b.position(), bytesAvailable);
                buf.position(buf.b.position() + bytesAvailable);
            }
            else if(_readBuffer.b.hasArray())
            {
                //
                // Copy directly from the source buffer's backing array.
                //
                byte[] arr = _readBuffer.b.array();
                buf.b.put(arr, _readBuffer.b.arrayOffset() + _readBuffer.b.position(), bytesAvailable);
                _readBuffer.b.position(_readBuffer.b.position() + bytesAvailable);
            }
            else
            {
                //
                // Copy using a temporary array.
                //
                byte[] arr = new byte[bytesAvailable];
                _readBuffer.b.get(arr);
                buf.b.put(arr);
            }
        }
        _readBuffer.b.compact();

        //
        // The read thread will temporarily stop reading if we exceed our configured limit.
        //
        if(_readBuffer.b.position() < _rcvSize)
        {
            notifyAll();
        }

        //
        // Update our Read state to indicate whether we still have more data waiting to be read.
        //
        _readyCallback.ready(SocketOperation.Read, _readBuffer.b.position() > 0);

        return buf.b.hasRemaining() ? SocketOperation.Read : SocketOperation.None;
    }

    @Override
    public String protocol()
    {
        return _instance.protocol();
    }

    @Override
    public String toString()
    {
        return _desc;
    }

    @Override
    public String toDetailedString()
    {
        return toString();
    }

    @Override
    public com.zeroc.Ice.ConnectionInfo getInfo()
    {
        ConnectionInfo info = new ConnectionInfo();
        info.incoming = _adapterName != null;
        info.adapterName = _adapterName != null ? _adapterName : "";
        info.connectionId = _connectionId;
        info.rcvSize = _rcvSize;
        info.sndSize = _sndSize;
        info.localAddress = BluetoothAdapter.getDefaultAdapter().getAddress();
        //info.localChannel - not available, use default value of -1
        info.remoteAddress = _remoteAddr;
        //info.remoteChannel - not available, use default value of -1
        info.uuid = _uuid;
        return info;
    }

    @Override
    public synchronized void setBufferSize(int rcvSize, int sndSize)
    {
        _rcvSize = Math.max(1024, rcvSize);
        _sndSize = Math.max(1024, sndSize);
    }

    @Override
    public void checkSendSize(Buffer buf)
    {
    }

    //
    // Used by ConnectorI.
    //
    TransceiverI(Instance instance, String remoteAddr, String uuid, String connectionId)
    {
        _instance = instance;
        _remoteAddr = remoteAddr;
        _uuid = uuid;
        _connectionId = connectionId;
        _state = StateConnecting;

        init();

        BluetoothAdapter adapter = _instance.bluetoothAdapter();
        assert(adapter != null);

        BluetoothDevice device = null;
        try
        {
            device = adapter.getRemoteDevice(_remoteAddr);
        }
        catch(IllegalArgumentException ex)
        {
            throw new SocketException(ex);
        }

        UUID uuidObj = null;
        try
        {
            uuidObj = UUID.fromString(_uuid);
        }
        catch(IllegalArgumentException ex)
        {
            throw new SocketException(ex);
        }

        try
        {
            //
            // We always connect using the "secure" method.
            //
            _socket = device.createRfcommSocketToServiceRecord(uuidObj);
        }
        catch(java.io.IOException ex)
        {
            throw new SocketException(ex);
        }

        _connectThread = new Thread()
        {
            public void run()
            {
                String name = "IceBT.ConnectThread";
                if(_remoteAddr != null && !_remoteAddr.isEmpty())
                {
                    name += "-" + _remoteAddr;
                }
                if(!_uuid.isEmpty())
                {
                    name += "-" + _uuid;
                }
                setName(name);

                runConnectThread();
            }
        };
        _connectThread.start();
    }

    //
    // Used by AcceptorI.
    //
    TransceiverI(Instance instance, BluetoothSocket socket, String uuid, String adapterName)
    {
        _instance = instance;
        _remoteAddr = socket.getRemoteDevice().getAddress();
        _uuid = uuid;
        _connectionId = "";
        _adapterName = adapterName;
        _socket = socket;
        _state = StateConnected;

        init();

        startReadWriteThreads();
    }

    private void init()
    {
        _desc = "local address = " + _instance.bluetoothAdapter().getAddress();
        if(_remoteAddr != null && !_remoteAddr.isEmpty())
        {
            _desc += "\nremote address = " + _remoteAddr;
        }
        if(!_uuid.isEmpty())
        {
            _desc += "\nservice uuid = " + _uuid;
        }

        final int defaultBufSize = 128 * 1024;
        _rcvSize = _instance.properties().getPropertyAsIntWithDefault("IceBT.RcvSize", defaultBufSize);
        _sndSize = _instance.properties().getPropertyAsIntWithDefault("IceBT.SndSize", defaultBufSize);

        _readBuffer = new Buffer(false);
        _writeBuffer = new Buffer(false);
    }

    private synchronized void exception(LocalException ex)
    {
        if(_exception == null)
        {
            _exception = ex;
        }
    }

    private void runConnectThread()
    {
        //
        // Always cancel discovery prior to a connect attempt.
        //
        _instance.bluetoothAdapter().cancelDiscovery();

        try
        {
            //
            // This can block for several seconds.
            //
            _socket.connect();
        }
        catch(java.io.IOException ex)
        {
            exception(new com.zeroc.Ice.ConnectFailedException(ex));
        }

        synchronized(this)
        {
            _connectThread = null;

            if(_exception == null)
            {
                //
                // Connect succeeded.
                //
                _state = StateConnected;

                startReadWriteThreads();
            }
        }

        //
        // This causes the Ice run time to invoke initialize() again.
        //
        _readyCallback.ready(SocketOperation.Read, true);
    }

    private void startReadWriteThreads()
    {
        String s = "";
        if(_remoteAddr != null && !_remoteAddr.isEmpty())
        {
            s += "-" + _remoteAddr;
        }
        if(!_uuid.isEmpty())
        {
            s += "-" + _uuid;
        }
        final String suffix = s;

        _readThread = new Thread()
        {
            public void run()
            {
                setName("IceBT.ReadThread" + suffix);

                runReadThread();
            }
        };
        _readThread.start();

        _writeThread = new Thread()
        {
            public void run()
            {
                setName("IceBT.WriteThread" + suffix);

                runWriteThread();
            }
        };
        _writeThread.start();
    }

    private void runReadThread()
    {
        java.io.InputStream in = null;

        try
        {
            byte[] buf = null;

            synchronized(this)
            {
                if(_socket == null)
                {
                    return;
                }
                in = _socket.getInputStream();

                buf = new byte[_rcvSize];
            }

            while(true)
            {
                synchronized(this)
                {
                    //
                    // If we've read too much data, wait until the application consumes some before we read again.
                    //
                    while(_state == StateConnected && _exception == null && _readBuffer.b.position() > _rcvSize)
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

                    if(_state != StateConnected || _exception != null)
                    {
                        break;
                    }
                }

                int num = in.read(buf);
                if(num > 0)
                {
                    synchronized(this)
                    {
                        _readBuffer.expand(num);
                        _readBuffer.b.put(buf, 0, num);
                        _readyCallback.ready(SocketOperation.Read, true);

                        if(buf.length != _rcvSize)
                        {
                            //
                            // Application must have called setBufferSize.
                            //
                            buf = new byte[_rcvSize];
                        }
                    }
                }
            }
        }
        catch(java.io.IOException ex)
        {
            exception(new SocketException(ex));
            //
            // Mark as ready for reading so that the Ice run time will invoke read() and we can report the exception.
            //
            _readyCallback.ready(SocketOperation.Read, true);
        }
        catch(LocalException ex)
        {
            exception(ex);
            //
            // Mark as ready for reading so that the Ice run time will invoke read() and we can report the exception.
            //
            _readyCallback.ready(SocketOperation.Read, true);
        }
        finally
        {
            if(in != null)
            {
                try
                {
                    in.close();
                }
                catch(java.io.IOException ex)
                {
                    // Ignore.
                }
            }
        }
    }

    private void runWriteThread()
    {
        java.io.OutputStream out = null;

        try
        {
            synchronized(this)
            {
                if(_socket == null)
                {
                    return;
                }
                out = _socket.getOutputStream();
            }

            boolean done = false;
            while(!done)
            {
                ByteBuffer b = null;

                synchronized(this)
                {
                    while(_state == StateConnected && _exception == null && _writeBuffer.b.position() == 0)
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

                    if(_state != StateConnected || _exception != null)
                    {
                        done = true;
                    }

                    b = _writeBuffer.b; // Adopt the ByteBuffer.
                    _writeBuffer.clear();
                }

                assert(b != null && b.hasArray());
                b.flip();
                if(b.hasRemaining() && !done)
                {
                    //
                    // write() blocks until all the data has been written.
                    //
                    out.write(b.array(), b.arrayOffset(), b.remaining());
                }

                // TBD: Recycle the buffer?

                synchronized(this)
                {
                    //
                    // After the write is complete, indicate whether we can accept more data.
                    //
                    _readyCallback.ready(SocketOperation.Write, _writeBuffer.b.position() < _sndSize);
                }
            }
        }
        catch(java.io.IOException ex)
        {
            exception(new SocketException(ex));
        }
        finally
        {
            if(out != null)
            {
                try
                {
                    out.close();
                }
                catch(java.io.IOException ex)
                {
                    // Ignore.
                }
            }
        }
    }

    private Instance _instance;
    private String _remoteAddr;
    private String _uuid;
    private String _connectionId;
    private String _adapterName;

    private BluetoothSocket _socket;

    private static final int StateConnecting = 0;
    private static final int StateConnected = 1;
    private static final int StateClosed = 2;
    private int _state;

    private Thread _connectThread;
    private Thread _readThread;
    private Thread _writeThread;

    private LocalException _exception;

    private int _rcvSize;
    private int _sndSize;

    private Buffer _readBuffer;
    private Buffer _writeBuffer;

    private String _desc;

    private ReadyCallback _readyCallback;
}
