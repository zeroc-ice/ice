// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;

namespace Ice.SSL;

#pragma warning disable CA1001 // _sslStream is disposed by destroy.
internal sealed class TransceiverI : Ice.Internal.Transceiver
#pragma warning restore CA1001
{
    public Socket fd() => _delegate.fd();

    public int initialize(Ice.Internal.Buffer readBuffer, Ice.Internal.Buffer writeBuffer, ref bool hasMoreData)
    {
        if (!_isConnected)
        {
            int status = _delegate.initialize(readBuffer, writeBuffer, ref hasMoreData);
            if (status != Ice.Internal.SocketOperation.None)
            {
                return status;
            }
            _isConnected = true;
        }

        Ice.Internal.Network.setBlock(fd(), true); // SSL requires a blocking socket

        // For timeouts to work properly, we need to receive/send the data in several chunks. Otherwise, we would only
        // be notified when all the data is received/written. The connection timeout could easily be triggered when
        // receiving/sending large messages.
        _maxSendPacketSize = Math.Max(512, Ice.Internal.Network.getSendBufferSize(fd()));
        _maxRecvPacketSize = Math.Max(512, Ice.Internal.Network.getRecvBufferSize(fd()));

        if (_sslStream == null)
        {
            try
            {
                _sslStream = new SslStream(
                    new NetworkStream(_delegate.fd(), ownsSocket: false),
                    leaveInnerStreamOpen: false);
            }
            catch (IOException ex)
            {
                if (Ice.Internal.Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                else
                {
                    throw new Ice.SocketException(ex);
                }
            }
            return Ice.Internal.SocketOperation.Connect;
        }

        Debug.Assert(_sslStream.IsAuthenticated);
        _authenticated = true;

        _cipher = _sslStream.CipherAlgorithm.ToString();
        _instance.verifyPeer((ConnectionInfo)getInfo(_incoming, _adapterName, connectionId: ""), ToString());

        if (_instance.securityTraceLevel() >= 1)
        {
            _instance.traceStream(_sslStream, ToString());
        }
        return Ice.Internal.SocketOperation.None;
    }

    public int closing(bool initiator, Ice.LocalException ex) => _delegate.closing(initiator, ex);

    public void close()
    {
        if (_sslStream != null)
        {
            cancelSslHandshake();
            _sslStream.Dispose(); // Disposing the stream also closes the socket.
            _sslStream = null;
        }

        _delegate.close();
    }

    public Ice.Internal.EndpointI bind()
    {
        Debug.Assert(false);
        return null;
    }

    public void destroy()
    {
        _delegate.destroy();
        cancelSslHandshake();
        _sslStream?.Dispose();
        _sslHandshakeCts.Dispose();
    }

    public int write(Ice.Internal.Buffer buf) =>
        // Force caller to use async write.
        buf.b.hasRemaining() ? Ice.Internal.SocketOperation.Write : Ice.Internal.SocketOperation.None;

    public int read(Ice.Internal.Buffer buf, ref bool hasMoreData) =>
        // Force caller to use async read.
        buf.b.hasRemaining() ? Ice.Internal.SocketOperation.Read : Ice.Internal.SocketOperation.None;

    public bool startRead(Ice.Internal.Buffer buf, Ice.Internal.AsyncCallback callback, object state)
    {
        if (!_isConnected)
        {
            return _delegate.startRead(buf, callback, state);
        }

        Debug.Assert(_sslStream != null && _sslStream.IsAuthenticated);

        int packetSz = getRecvPacketSize(buf.b.remaining());
        try
        {
            _readResult = _sslStream.ReadAsync(buf.b.rawBytes(), buf.b.position(), packetSz);
            _readResult.ContinueWith(
                task => callback(state),
                TaskScheduler.Default);
            return false;
        }
        catch (IOException ex)
        {
            if (Ice.Internal.Network.connectionLost(ex))
            {
                throw new Ice.ConnectionLostException(ex);
            }
            if (Ice.Internal.Network.timeout(ex))
            {
                throw new Ice.TimeoutException();
            }
            throw new Ice.SocketException(ex);
        }
        catch (ObjectDisposedException ex)
        {
            throw new Ice.ConnectionLostException(ex);
        }
        catch (Exception ex)
        {
            throw new Ice.SyscallException(ex);
        }
    }

    public void finishRead(Ice.Internal.Buffer buf)
    {
        if (!_isConnected)
        {
            _delegate.finishRead(buf);
            return;
        }
        else if (_sslStream == null) // Transceiver was closed
        {
            _readResult = null;
            return;
        }

        Debug.Assert(_readResult != null);
        try
        {
            int ret;
            try
            {
                ret = _readResult.Result;
            }
            catch (AggregateException ex)
            {
                throw ex.InnerException;
            }

            if (ret == 0)
            {
                throw new Ice.ConnectionLostException();
            }
            Debug.Assert(ret > 0);
            buf.b.position(buf.b.position() + ret);
        }
        catch (Ice.LocalException)
        {
            throw;
        }
        catch (IOException ex)
        {
            if (Ice.Internal.Network.connectionLost(ex))
            {
                throw new Ice.ConnectionLostException(ex);
            }
            if (Ice.Internal.Network.timeout(ex))
            {
                throw new Ice.TimeoutException();
            }
            throw new Ice.SocketException(ex);
        }
        catch (ObjectDisposedException ex)
        {
            throw new Ice.ConnectionLostException(ex);
        }
        catch (Exception ex)
        {
            throw new Ice.SyscallException(ex);
        }
    }

    public bool startWrite(Internal.Buffer buf, Internal.AsyncCallback cb, object state, out bool messageFullyWritten)
    {
        if (!_isConnected)
        {
            return _delegate.startWrite(buf, cb, state, out messageFullyWritten);
        }

        Debug.Assert(_sslStream != null);
        if (!_authenticated)
        {
            messageFullyWritten = false;
            return startAuthenticate(cb, state);
        }

        // We limit the packet size for beingWrite to ensure connection timeouts are based on a fixed packet size.
        int packetSize = getSendPacketSize(buf.b.remaining());
        try
        {
            _writeResult = _sslStream.WriteAsync(buf.b.rawBytes(), buf.b.position(), packetSize);
            _writeResult.ContinueWith(task => cb(state), TaskScheduler.Default);
            messageFullyWritten = packetSize == buf.b.remaining();
            return false;
        }
        catch (IOException ex)
        {
            if (Ice.Internal.Network.connectionLost(ex))
            {
                throw new Ice.ConnectionLostException(ex);
            }
            if (Ice.Internal.Network.timeout(ex))
            {
                throw new Ice.TimeoutException();
            }
            throw new Ice.SocketException(ex);
        }
        catch (ObjectDisposedException ex)
        {
            throw new Ice.ConnectionLostException(ex);
        }
        catch (Exception ex)
        {
            throw new Ice.SyscallException(ex);
        }
    }

    public void finishWrite(Ice.Internal.Buffer buf)
    {
        if (!_isConnected)
        {
            _delegate.finishWrite(buf);
            return;
        }
        else if (_sslStream == null) // Transceiver was closed
        {
            if (getSendPacketSize(buf.b.remaining()) == buf.b.remaining()) // Sent last packet
            {
                buf.b.position(buf.b.limit()); // Assume all the data was sent for at-most-once semantics.
            }
            _writeResult = null;
            return;
        }
        else if (!_authenticated)
        {
            finishAuthenticate();
            return;
        }

        int sent = getSendPacketSize(buf.b.remaining());
        Debug.Assert(_writeResult != null);
        try
        {
            try
            {
                _writeResult.Wait();
            }
            catch (AggregateException ex)
            {
                throw ex.InnerException;
            }
            buf.b.position(buf.b.position() + sent);
        }
        catch (IOException ex)
        {
            if (Ice.Internal.Network.connectionLost(ex))
            {
                throw new Ice.ConnectionLostException(ex);
            }
            if (Ice.Internal.Network.timeout(ex))
            {
                throw new Ice.TimeoutException();
            }
            throw new Ice.SocketException(ex);
        }
        catch (ObjectDisposedException ex)
        {
            throw new Ice.ConnectionLostException(ex);
        }
        catch (Exception ex)
        {
            throw new Ice.SyscallException(ex);
        }
    }

    public string protocol() => _delegate.protocol();

    public Ice.ConnectionInfo getInfo(bool incoming, string adapterName, string connectionId)
    {
        Debug.Assert(incoming == _incoming);
        // adapterName is the name of the object adapter currently associated with this connection, while _adapterName
        // represents the name of the object adapter that created this connection (incoming only).

        return new Ice.SSL.ConnectionInfo(
            _delegate.getInfo(incoming, adapterName, connectionId),
            _cipher,
            _sslStream is SslStream sslStream && sslStream.RemoteCertificate is X509Certificate2 remoteCertificate ?
                [remoteCertificate] : [],
            _verified);
    }

    public void checkSendSize(Ice.Internal.Buffer buf) => _delegate.checkSendSize(buf);

    public void setBufferSize(int rcvSize, int sndSize) => _delegate.setBufferSize(rcvSize, sndSize);

    public override string ToString() => _delegate.ToString();

    public string toDetailedString() => _delegate.toDetailedString();

    // Only for use by ConnectorI, AcceptorI.
    internal TransceiverI(
        Instance instance,
        Ice.Internal.Transceiver del,
        string hostOrAdapterName,
        bool incoming,
        SslServerAuthenticationOptions serverAuthenticationOptions)
    {
        _instance = instance;
        _delegate = del;
        _incoming = incoming;
        if (_incoming)
        {
            _adapterName = hostOrAdapterName;
            _serverAuthenticationOptions = serverAuthenticationOptions;
        }
        else
        {
            _host = hostOrAdapterName;
            Debug.Assert(_serverAuthenticationOptions is null);
        }

        _sslStream = null;

        _verifyPeer = _instance.properties().getIcePropertyAsInt("IceSSL.VerifyPeer");
    }

    /// <summary>
    /// If the SSL handshake is in progress, cancel it and wait for it to finish. This is used to ensure that the
    /// SSLStream is not disposed while the handshake is in progress.
    /// </summary>
    private void cancelSslHandshake()
    {
        if (!_verified && _writeResult is not null)
        {
            _sslHandshakeCts.Cancel();
            try
            {
                _writeResult.Wait();
                _writeResult = null;
            }
            catch
            {
            }
        }
        Debug.Assert(_writeResult is null || _writeResult.IsCompleted);
    }

    private bool startAuthenticate(Ice.Internal.AsyncCallback callback, object state)
    {
        try
        {
            if (_incoming)
            {
                _writeResult = _sslStream.AuthenticateAsServerAsync(
                    _serverAuthenticationOptions ??
                    _instance.engine().createServerAuthenticationOptions(validationCallback),
                    _sslHandshakeCts.Token);
            }
            else
            {
                _writeResult = _sslStream.AuthenticateAsClientAsync(
                    _instance.initializationData().clientAuthenticationOptions ??
                    _instance.engine().createClientAuthenticationOptions(validationCallback, _host),
                    _sslHandshakeCts.Token);
            }
            _writeResult.ContinueWith(
                task => callback(state),
                TaskScheduler.Default);
        }
        catch (IOException ex)
        {
            if (Ice.Internal.Network.connectionLost(ex))
            {
                // This situation occurs when connectToSelf is called; the "remote" end closes the socket immediately.
                throw new ConnectionLostException(ex);
            }
            throw new SocketException(ex);
        }
        catch (AuthenticationException ex)
        {
            throw new SecurityException(
                _errorDescription.Length == 0 ? "SSL authentication failure." : _errorDescription,
                ex);
        }
        catch (System.Exception ex)
        {
            throw new Ice.SyscallException(ex);
        }

        Debug.Assert(_writeResult != null);
        return false;
    }

    private void finishAuthenticate()
    {
        Debug.Assert(_writeResult != null);
        try
        {
            try
            {
                // If authentication fails the task throws AuthenticationException.
                _writeResult.Wait();
                _verified = true;
                _cipher = _sslStream.CipherAlgorithm.ToString();
            }
            catch (AggregateException ex)
            {
                throw ex.InnerException;
            }
        }
        catch (IOException ex)
        {
            if (Ice.Internal.Network.connectionLost(ex))
            {
                // This situation occurs when connectToSelf is called; the "remote" end closes the socket immediately.
                throw new Ice.ConnectionLostException();
            }
            throw new Ice.SocketException(ex);
        }
        catch (AuthenticationException ex)
        {
            throw new SecurityException(
                _errorDescription.Length == 0 ? "SSL authentication failure." : _errorDescription,
                ex);
        }
        catch (System.Exception ex)
        {
            throw new Ice.SyscallException(ex);
        }
    }

    private bool validationCallback(
        object sender,
        X509Certificate certificate,
        X509Chain chain,
        SslPolicyErrors policyErrors)
    {
        int errors = (int)policyErrors;
        int traceLevel = _instance.securityTraceLevel();
        string traceCategory = _instance.securityTraceCategory();
        Ice.Logger logger = _instance.logger();
        string message = "";

        if (_incoming && (errors & (int)SslPolicyErrors.RemoteCertificateNotAvailable) != 0 && _verifyPeer <= 1)
        {
            // The client certificate is optional when IceSSL.VerifyPeer = 1, and not required when IceSSL.VerifyPeer = 0
            errors ^= (int)SslPolicyErrors.RemoteCertificateNotAvailable;
        }

        if ((errors & (int)SslPolicyErrors.RemoteCertificateNameMismatch) != 0)
        {
            message += ": Remote certificate name mismatch";
        }

        if ((errors & (int)SslPolicyErrors.RemoteCertificateNotAvailable) != 0)
        {
            message += ": Remote certificate not available";
        }

        foreach (X509ChainStatus status in chain?.ChainStatus ?? [])
        {
            message += $": {status.StatusInformation}";
        }

        if (errors != 0)
        {
            _errorDescription =
                message.Length > 0 ? $"SSL authentication failure{message}." : "SSL authentication failure.";
            if (traceLevel >= 1)
            {
                logger.trace(traceCategory, _errorDescription);
            }
        }
        return errors == 0;
    }

    private int getSendPacketSize(int length) =>
        _maxSendPacketSize > 0 ? Math.Min(length, _maxSendPacketSize) : length;

    public int getRecvPacketSize(int length) =>
        _maxRecvPacketSize > 0 ? Math.Min(length, _maxRecvPacketSize) : length;

    private string _errorDescription = "";
    private readonly Instance _instance;
    private readonly Ice.Internal.Transceiver _delegate;
    private readonly string _host = "";
    private readonly string _adapterName = "";
    private readonly bool _incoming;
    private SslStream _sslStream;
    private readonly int _verifyPeer;
    private bool _isConnected;
    private bool _authenticated;
    private Task _writeResult;
    private Task<int> _readResult;
    private int _maxSendPacketSize;
    private int _maxRecvPacketSize;
    private string _cipher;
    private bool _verified;
    private readonly SslServerAuthenticationOptions _serverAuthenticationOptions;
    private readonly CancellationTokenSource _sslHandshakeCts = new();
}
