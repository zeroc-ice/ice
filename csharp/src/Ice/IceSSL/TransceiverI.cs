// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;

namespace IceSSL;

internal sealed class TransceiverI : IceInternal.Transceiver
{
    public Socket fd() => _delegate.fd();

    public int initialize(IceInternal.Buffer readBuffer, IceInternal.Buffer writeBuffer, ref bool hasMoreData)
    {
        if (!_isConnected)
        {
            int status = _delegate.initialize(readBuffer, writeBuffer, ref hasMoreData);
            if (status != IceInternal.SocketOperation.None)
            {
                return status;
            }
            _isConnected = true;
        }

        IceInternal.Network.setBlock(fd(), true); // SSL requires a blocking socket

        // For timeouts to work properly, we need to receive/send the data in several chunks. Otherwise, we would only
        // be notified when all the data is received/written. The connection timeout could easily be triggered when
        // receiving/sending large messages.
        _maxSendPacketSize = Math.Max(512, IceInternal.Network.getSendBufferSize(fd()));
        _maxRecvPacketSize = Math.Max(512, IceInternal.Network.getRecvBufferSize(fd()));

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
                if (IceInternal.Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                else
                {
                    throw new Ice.SocketException(ex);
                }
            }
            return IceInternal.SocketOperation.Connect;
        }

        Debug.Assert(_sslStream.IsAuthenticated);
        _authenticated = true;

        _cipher = _sslStream.CipherAlgorithm.ToString();
        _instance.verifyPeer((ConnectionInfo)getInfo(), ToString());

        if (_instance.securityTraceLevel() >= 1)
        {
            _instance.traceStream(_sslStream, ToString());
        }
        return IceInternal.SocketOperation.None;
    }

    public int closing(bool initiator, Ice.LocalException ex) => _delegate.closing(initiator, ex);

    public void close()
    {
        if (_sslStream != null)
        {
            _sslStream.Close(); // Closing the stream also closes the socket.
            _sslStream = null;
        }

        _delegate.close();
    }

    public IceInternal.EndpointI bind()
    {
        Debug.Assert(false);
        return null;
    }

    public void destroy() => _delegate.destroy();

    public int write(IceInternal.Buffer buf) =>
        // Force caller to use async write.
        buf.b.hasRemaining() ? IceInternal.SocketOperation.Write : IceInternal.SocketOperation.None;

    public int read(IceInternal.Buffer buf, ref bool hasMoreData) =>
        // Force caller to use async read.
        buf.b.hasRemaining() ? IceInternal.SocketOperation.Read : IceInternal.SocketOperation.None;

    public bool startRead(IceInternal.Buffer buf, IceInternal.AsyncCallback callback, object state)
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
            _readResult.ContinueWith(task => callback(state), TaskScheduler.Default);
            return false;
        }
        catch (IOException ex)
        {
            if (IceInternal.Network.connectionLost(ex))
            {
                throw new Ice.ConnectionLostException(ex);
            }
            if (IceInternal.Network.timeout(ex))
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

    public void finishRead(IceInternal.Buffer buf)
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
            if (IceInternal.Network.connectionLost(ex))
            {
                throw new Ice.ConnectionLostException(ex);
            }
            if (IceInternal.Network.timeout(ex))
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

    public bool startWrite(IceInternal.Buffer buf, IceInternal.AsyncCallback cb, object state, out bool completed)
    {
        if (!_isConnected)
        {
            return _delegate.startWrite(buf, cb, state, out completed);
        }

        Debug.Assert(_sslStream != null);
        if (!_authenticated)
        {
            completed = false;
            return startAuthenticate(cb, state);
        }

        // We limit the packet size for beingWrite to ensure connection timeouts are based on a fixed packet size.
        int packetSize = getSendPacketSize(buf.b.remaining());
        try
        {
            _writeResult = _sslStream.WriteAsync(buf.b.rawBytes(), buf.b.position(), packetSize);
            _writeResult.ContinueWith(task => cb(state), TaskScheduler.Default);
            completed = packetSize == buf.b.remaining();
            return false;
        }
        catch (IOException ex)
        {
            if (IceInternal.Network.connectionLost(ex))
            {
                throw new Ice.ConnectionLostException(ex);
            }
            if (IceInternal.Network.timeout(ex))
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

    public void finishWrite(IceInternal.Buffer buf)
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
            if (IceInternal.Network.connectionLost(ex))
            {
                throw new Ice.ConnectionLostException(ex);
            }
            if (IceInternal.Network.timeout(ex))
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

    public Ice.ConnectionInfo getInfo()
    {
        var info = new ConnectionInfo();
        info.underlying = _delegate.getInfo();
        info.incoming = _incoming;
        info.adapterName = _adapterName;
        info.cipher = _cipher;
        info.certs = _certs;
        info.verified = _verified;
        return info;
    }

    public void checkSendSize(IceInternal.Buffer buf) => _delegate.checkSendSize(buf);

    public void setBufferSize(int rcvSize, int sndSize) => _delegate.setBufferSize(rcvSize, sndSize);

    public override string ToString() => _delegate.ToString();

    public string toDetailedString() => _delegate.toDetailedString();

    //
    // Only for use by ConnectorI, AcceptorI.
    //
    internal TransceiverI(
        Instance instance,
        IceInternal.Transceiver del,
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

        _verifyPeer = _instance.properties().getPropertyAsIntWithDefault("IceSSL.VerifyPeer", 2);
    }

    private bool startAuthenticate(IceInternal.AsyncCallback callback, object state)
    {
        try
        {
            if (_incoming)
            {
                _writeResult = _sslStream.AuthenticateAsServerAsync(
                    _serverAuthenticationOptions ?? _instance.engine().serverAuthenticationOptions(
                        new RemoteCertificateValidationCallback(validationCallback)));
                _writeResult.ContinueWith(
                    task =>
                    {
                        try
                        {
                            // If authentication fails, AuthenticateAsServerAsync throws AuthenticationException,
                            // and the task won't complete successfully.
                            _verified = task.IsCompletedSuccessfully;
                            if (_verified)
                            {
                                _cipher = _sslStream.CipherAlgorithm.ToString();
                                if (_sslStream.RemoteCertificate is X509Certificate2 remoteCertificate)
                                {
                                    _certs = [remoteCertificate];
                                }
                            }
                        }
                        finally
                        {
                            callback(state);
                        }
                    },
                    TaskScheduler.Default);
            }
            else
            {
                _writeResult = _sslStream.AuthenticateAsClientAsync(
                    _instance.initializationData().clientAuthenticationOptions ??
                    _instance.engine().clientAuthenticationOptions(
                        new RemoteCertificateValidationCallback(validationCallback)));
                _writeResult.ContinueWith(
                    task =>
                    {
                        try
                        {
                            // If authentication fails, AuthenticateAsClientAsync throws AuthenticationException,
                            // and the task won't complete successfully.
                            _verified = task.IsCompletedSuccessfully;
                            if (_verified)
                            {
                                _cipher = _sslStream.CipherAlgorithm.ToString();
                                if (_sslStream.RemoteCertificate is X509Certificate2 remoteCertificate)
                                {
                                    _certs = [remoteCertificate];
                                }
                            }
                        }
                        finally
                        {
                            callback(state);
                        }
                    },
                    TaskScheduler.Default);
            }
        }
        catch (IOException ex)
        {
            if (IceInternal.Network.connectionLost(ex))
            {
                //
                // This situation occurs when connectToSelf is called; the "remote" end
                // closes the socket immediately.
                //
                throw new Ice.ConnectionLostException();
            }
            throw new Ice.SocketException(ex);
        }
        catch (AuthenticationException ex)
        {
            var e = new Ice.SecurityException(ex);
            e.reason = ex.Message;
            throw e;
        }
        catch (Exception ex)
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
                _writeResult.Wait();
            }
            catch (AggregateException ex)
            {
                throw ex.InnerException;
            }
        }
        catch (IOException ex)
        {
            if (IceInternal.Network.connectionLost(ex))
            {
                // This situation occurs when connectToSelf is called; the "remote" end closes the socket immediately.
                throw new Ice.ConnectionLostException();
            }
            throw new Ice.SocketException(ex);
        }
        catch (AuthenticationException ex)
        {
            var e = new Ice.SecurityException(ex);
            e.reason = ex.Message;
            throw e;
        }
        catch (Exception ex)
        {
            throw new Ice.SyscallException(ex);
        }
    }

    private bool validationCallback(
        object sender,
        X509Certificate certificate,
        X509Chain chainEngine,
        SslPolicyErrors policyErrors)
    {
        try
        {
            using var chain = new X509Chain(_instance.engine().useMachineContext());

            bool verified = false;
            int errors = (int)policyErrors;
            int traceLevel = _instance.securityTraceLevel();
            string traceCategory = _instance.securityTraceCategory();
            Ice.Logger logger = _instance.logger();
            bool checkCertName = _instance.engine().getCheckCertName();
            string message = "";

            X509Certificate2Collection caCerts = _instance.engine().caCerts();
            if (caCerts is null)
            {
                verified = policyErrors == SslPolicyErrors.None;
            }
            else
            {
                // We need to set this flag to be able to use a certificate authority from the extra store.
                chain.ChainPolicy.RevocationMode = X509RevocationMode.NoCheck;
                chain.ChainPolicy.DisableCertificateDownloads = true;
                chain.ChainPolicy.TrustMode = X509ChainTrustMode.CustomRootTrust;
                foreach (X509Certificate2 cert in caCerts)
                {
                    chain.ChainPolicy.CustomTrustStore.Add(cert);
                }

                if (certificate is not null)
                {
                    verified = chain.Build((X509Certificate2)certificate);
                    if (verified)
                    {
                        errors ^= (int)SslPolicyErrors.RemoteCertificateChainErrors;
                    }
                }
            }

            if ((errors & (int)SslPolicyErrors.RemoteCertificateNotAvailable) > 0)
            {
                // The RemoteCertificateNotAvailable case does not appear to be possible for an outgoing connection.
                // Since .NET requires an authenticated connection, the remote peer closes the socket if it does not
                // have a certificate to provide.
                if (_incoming)
                {
                    if (_verifyPeer > 1)
                    {
                        if (_instance.engine().securityTraceLevel() >= 1)
                        {
                            logger.trace(
                                traceCategory,
                                "SSL certificate validation failed - client certificate not provided");
                        }
                    }
                    else
                    {
                        errors ^= (int)SslPolicyErrors.RemoteCertificateNotAvailable;
                        message += "\nremote certificate not provided (ignored)";
                    }
                }
            }

            bool certificateNameMismatch = (errors & (int)SslPolicyErrors.RemoteCertificateNameMismatch) > 0;
            if (certificateNameMismatch)
            {
                if (checkCertName && !string.IsNullOrEmpty(_host))
                {
                    if (traceLevel >= 1)
                    {
                        string msg = "SSL certificate validation failed - Hostname mismatch";
                        if (_verifyPeer == 0)
                        {
                            msg += " (ignored)";
                        }
                        logger.trace(traceCategory, msg);
                    }
                }
                else
                {
                    errors ^= (int)SslPolicyErrors.RemoteCertificateNameMismatch;
                }
            }

            foreach (X509ChainStatus status in chain?.ChainStatus ?? chainEngine?.ChainStatus)
            {
                message += status.Status switch
                {
                    X509ChainStatusFlags.UntrustedRoot => "\nuntrusted root certificate",
                    X509ChainStatusFlags.Revoked => "\ncertificate revoked",
                    X509ChainStatusFlags.RevocationStatusUnknown => "\ncertificate revocation status unknown",
                    X509ChainStatusFlags.PartialChain => "\npartial certificate chain",
                    X509ChainStatusFlags.NoError => "",
                    _ => $"\ncertificate chain error: {status.Status}",
                };
            }

            if (traceLevel >= 1)
            {
                if (message.Length > 0)
                {
                    logger.trace(traceCategory, $"SSL certificate validation failed:{message}");
                }
                else
                {
                    logger.trace(traceCategory, "SSL certificate validation failed");
                }
            }
            return errors == 0;
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex.ToString());
            throw;
        }
    }

    private int getSendPacketSize(int length) =>
        _maxSendPacketSize > 0 ? Math.Min(length, _maxSendPacketSize) : length;

    public int getRecvPacketSize(int length) =>
        _maxRecvPacketSize > 0 ? Math.Min(length, _maxRecvPacketSize) : length;

    private readonly Instance _instance;
    private readonly IceInternal.Transceiver _delegate;
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
    private X509Certificate2[] _certs;
    private bool _verified;
    private readonly SslServerAuthenticationOptions _serverAuthenticationOptions;
}
