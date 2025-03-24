// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Net.Sockets;

internal class Transceiver : Ice.Internal.Transceiver
{
    public Socket fd() => _transceiver.fd();

    public int initialize(Ice.Internal.Buffer readBuffer, Ice.Internal.Buffer writeBuffer, ref bool hasMoreData)
    {
        _configuration.checkInitializeException();
        if (!_initialized)
        {
            int status = _transceiver.initialize(readBuffer, writeBuffer, ref hasMoreData);
            if (status != Ice.Internal.SocketOperation.None)
            {
                return status;
            }
            _initialized = true;
        }
        return Ice.Internal.SocketOperation.None;
    }

    public int closing(bool initiator, Ice.LocalException ex) => _transceiver.closing(initiator, ex);

    public void close() => _transceiver.close();

    public Ice.Internal.EndpointI bind() => _transceiver.bind();

    public int write(Ice.Internal.Buffer buf)
    {
        if (!_configuration.writeReady() && buf.b.hasRemaining())
        {
            return Ice.Internal.SocketOperation.Write;
        }

        _configuration.checkWriteException();
        return _transceiver.write(buf);
    }

    public int read(Ice.Internal.Buffer buf, ref bool hasMoreData)
    {
        if (!_configuration.readReady() && buf.b.hasRemaining())
        {
            return Ice.Internal.SocketOperation.Read;
        }

        _configuration.checkReadException();

        if (_buffered)
        {
            while (buf.b.hasRemaining())
            {
                if (_readBufferPos == _readBuffer.b.position())
                {
                    _readBufferPos = 0;
                    _readBuffer.b.position(0);
                    _transceiver.read(_readBuffer, ref hasMoreData);
                    if (_readBufferPos == _readBuffer.b.position())
                    {
                        hasMoreData = false;
                        return Ice.Internal.SocketOperation.Read;
                    }
                }

                int pos = _readBuffer.b.position();
                Debug.Assert(pos > _readBufferPos);
                int requested = buf.b.remaining();
                int available = pos - _readBufferPos;
                Debug.Assert(available > 0);
                if (available >= requested)
                {
                    available = requested;
                }

                byte[] arr = new byte[available];
                _readBuffer.b.position(_readBufferPos);
                _readBuffer.b.get(arr);
                buf.b.put(arr);
                _readBufferPos += available;
                _readBuffer.b.position(pos);
            }
            hasMoreData = _readBufferPos < _readBuffer.b.position();
            return Ice.Internal.SocketOperation.None;
        }
        else
        {
            return _transceiver.read(buf, ref hasMoreData);
        }
    }

    public bool startRead(Ice.Internal.Buffer buf, Ice.Internal.AsyncCallback callback, object state)
    {
        if (_configuration.readReady())
        {
            _configuration.checkReadException(); // Only raise if we're configured to read now.
        }
        if (_buffered)
        {
            int pos = _readBuffer.b.position();
            int available = pos - _readBufferPos;
            if (available > 0)
            {
                int requested = buf.b.remaining();
                if (available >= requested)
                {
                    available = requested;
                }

                byte[] arr = new byte[available];
                _readBuffer.b.position(_readBufferPos);
                _readBuffer.b.get(arr);
                buf.b.put(arr);
                _readBufferPos += available;
                _readBuffer.b.position(pos);
            }

            if (_readBufferPos == _readBuffer.b.position() && buf.b.hasRemaining())
            {
                _readBufferPos = 0;
                _readBuffer.b.position(0);
                return _transceiver.startRead(_readBuffer, callback, state);
            }
            else
            {
                Debug.Assert(!buf.b.hasRemaining());
                return true; // Completed synchronously
            }
        }
        else
        {
            return _transceiver.startRead(buf, callback, state);
        }
    }

    public void finishRead(Ice.Internal.Buffer buf)
    {
        _configuration.checkReadException();
        if (_buffered)
        {
            if (buf.b.hasRemaining())
            {
                _transceiver.finishRead(_readBuffer);

                int pos = _readBuffer.b.position();
                int requested = buf.b.remaining();
                int available = pos - _readBufferPos;
                if (available > 0)
                {
                    if (available >= requested)
                    {
                        available = requested;
                    }

                    byte[] arr = new byte[available];
                    _readBuffer.b.position(_readBufferPos);
                    _readBuffer.b.get(arr);
                    buf.b.put(arr);
                    _readBufferPos += available;
                    _readBuffer.b.position(pos);
                }
            }
        }
        else
        {
            _transceiver.finishRead(buf);
        }
    }

    public bool startWrite(
        Ice.Internal.Buffer buf,
        Ice.Internal.AsyncCallback callback,
        object state,
        out bool messageFullyWritten)
    {
        _configuration.checkWriteException();
        return _transceiver.startWrite(buf, callback, state, out messageFullyWritten);
    }

    public void finishWrite(Ice.Internal.Buffer buf)
    {
        _configuration.checkWriteException();
        _transceiver.finishWrite(buf);
    }

    public string protocol() => "test-" + _transceiver.protocol();

    public Ice.ConnectionInfo getInfo(bool incoming, string adapterName, string connectionId) =>
        _transceiver.getInfo(incoming, adapterName, connectionId);

    public override string ToString() => _transceiver.ToString();

    public string toDetailedString() => _transceiver.toDetailedString();

    public void checkSendSize(Ice.Internal.Buffer buf) => _transceiver.checkSendSize(buf);

    public void setBufferSize(int rcvSize, int sndSize) => _transceiver.setBufferSize(rcvSize, sndSize);

    public void destroy() => _transceiver.destroy();

    public Ice.Internal.Transceiver getDelegate() => _transceiver;

    //
    // Only for use by Connector, Acceptor
    //
    internal Transceiver(Ice.Internal.Transceiver transceiver)
    {
        _transceiver = transceiver;
        _configuration = Configuration.getInstance();
        _initialized = false;
        _readBuffer = new Ice.Internal.Buffer();
        _readBuffer.resize(1024 * 8, true); // 8KB buffer
        _readBuffer.b.position(0);
        _readBufferPos = 0;
        _buffered = _configuration.buffered();
    }

    private readonly Ice.Internal.Transceiver _transceiver;
    private readonly Configuration _configuration;
    private bool _initialized;
    private readonly Ice.Internal.Buffer _readBuffer;
    private int _readBufferPos;
    private readonly bool _buffered;
}
