//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;
using System.Text;

namespace ZeroC.Ice
{
    internal sealed class SslTransceiver : ITransceiver
    {
        internal SslStream? SslStream { get; private set; }

        private readonly string? _adapterName;
        private bool _authenticated;
        private readonly Communicator _communicator;
        private readonly ITransceiver _delegate;
        private readonly SslEngine _engine;
        private readonly string? _host;
        private readonly bool _incoming;
        private bool _isConnected;
        private int _maxRecvPacketSize;
        private int _maxSendPacketSize;
        private AsyncCallback? _readCallback;
        private IAsyncResult? _readResult;
        private AsyncCallback? _writeCallback;
        private IAsyncResult? _writeResult;

        public Socket? Fd() => _delegate.Fd();

        public int Initialize(ref ArraySegment<byte> readBuffer, IList<ArraySegment<byte>> writeBuffer)
        {
            if (!_isConnected)
            {
                int status = _delegate.Initialize(ref readBuffer, writeBuffer);
                if (status != SocketOperation.None)
                {
                    return status;
                }
                _isConnected = true;
            }

            if (SslStream == null)
            {
                try
                {
                    Socket? fd = _delegate.Fd();
                    Debug.Assert(fd != null);

                    Network.SetBlock(fd, true); // SSL requires a blocking socket

                    // For timeouts to work properly, we need to receive/send the data in several chunks. Otherwise,
                    // we would only be notified when all the data is received/written. The connection timeout could
                    // easily be triggered when receiving/sending large frames.
                    _maxSendPacketSize = Math.Max(512, Network.GetSendBufferSize(fd));
                    _maxRecvPacketSize = Math.Max(512, Network.GetRecvBufferSize(fd));

                    if (_incoming)
                    {
                        SslStream = new SslStream(
                            new NetworkStream(fd, false),
                            false,
                            _engine.TlsServerOptions.ClientCertificateValidationCallback ??
                                RemoteCertificateValidationCallback);
                    }
                    else
                    {
                        SslStream = new SslStream(
                            new NetworkStream(fd, false),
                            false,
                            _engine.TlsClientOptions.ServerCertificateValidationCallback ??
                                RemoteCertificateValidationCallback,
                            _engine.TlsClientOptions.ClientCertificateSelectionCallback ??
                                CertificateSelectionCallback);
                    }
                }
                catch (Exception ex)
                {
                    if (ex is IOException ioException && Network.ConnectionLost(ioException))
                    {
                        throw new ConnectionLostException(ex);
                    }
                    else
                    {
                        throw new TransportException(ex);
                    }
                }
                return SocketOperation.Connect;
            }

            Debug.Assert(SslStream.IsAuthenticated);
            _authenticated = true;

            string description = ToString();
            if (!_engine.SslTrustManager.Verify(_incoming,
                                                SslStream.RemoteCertificate as X509Certificate2,
                                                _adapterName ?? "",
                                                description))
            {
                var s = new StringBuilder();
                s.Append(_incoming ? "incoming " : "outgoing");
                s.Append("connection rejected by trust manager\n");
                s.Append(description);
                if (_engine.SecurityTraceLevel >= 1)
                {
                    _communicator.Logger.Trace(_engine.SecurityTraceCategory, s.ToString());
                }

                throw new TransportException(s.ToString());
            }

            if (_engine.SecurityTraceLevel >= 1)
            {
                _engine.TraceStream(SslStream, description);
            }
            return SocketOperation.None;
        }

        public Endpoint Bind()
        {
            Debug.Assert(false);
            return null;
        }

        public void Close()
        {
            if (SslStream != null)
            {
                SslStream.Close();
                SslStream = null;
            }

            _delegate.Close();
        }

        public int Closing(bool initiator, Exception? ex) => _delegate.Closing(initiator, ex);

        public void CheckSendSize(int size) => _delegate.CheckSendSize(size);

        public void Destroy() => _delegate.Destroy();

        public void FinishRead(ref ArraySegment<byte> buffer, ref int offset)
        {
            if (!_isConnected)
            {
                _delegate.FinishRead(ref buffer, ref offset);
                return;
            }
            else if (SslStream == null) // Transceiver was closed
            {
                _readResult = null;
                return;
            }

            Debug.Assert(_readResult != null);
            try
            {
                int ret = SslStream.EndRead(_readResult);
                _readResult = null;

                if (ret == 0)
                {
                    throw new ConnectionLostException();
                }
                Debug.Assert(ret > 0);
                offset += ret;
            }
            catch (IOException ex)
            {
                if (Network.ConnectionLost(ex))
                {
                    throw new ConnectionLostException(ex);
                }
                if (Network.Timeout(ex))
                {
                    throw new ConnectionTimeoutException();
                }
                throw new TransportException(ex);
            }
            catch (ObjectDisposedException ex)
            {
                throw new ConnectionLostException(ex);
            }
        }

        public void FinishWrite(IList<ArraySegment<byte>> buffer, ref int offset)
        {
            if (!_isConnected)
            {
                _delegate.FinishWrite(buffer, ref offset);
                return;
            }
            else if (SslStream == null) // Transceiver was closed
            {
                int remaining = buffer.GetByteCount() - offset;
                if (GetSendPacketSize(remaining) == remaining) // Sent last packet
                {
                    offset = remaining; // Assume all the data was sent for at-most-once semantics.
                }
                _writeResult = null;
                return;
            }
            else if (!_authenticated)
            {
                FinishAuthenticate();
                return;
            }

            Debug.Assert(_writeResult != null);
            int bytesTransferred = GetSendPacketSize(buffer.GetByteCount() - offset);
            try
            {
                SslStream.EndWrite(_writeResult);
                offset += bytesTransferred;
            }
            catch (IOException ex)
            {
                if (Network.ConnectionLost(ex))
                {
                    throw new ConnectionLostException(ex);
                }
                if (Network.Timeout(ex))
                {
                    throw new ConnectionTimeoutException();
                }
                throw new TransportException(ex);
            }
            catch (ObjectDisposedException ex)
            {
                throw new ConnectionLostException(ex);
            }
        }

        // Force caller to use asynchronous read.
        public int Read(ref ArraySegment<byte> buffer, ref int offset) =>
            offset < buffer.Count ? SocketOperation.Read : SocketOperation.None;

        public bool StartRead(ref ArraySegment<byte> buffer, ref int offset, AsyncCallback callback, object state)
        {
            if (!_isConnected)
            {
                return _delegate.StartRead(ref buffer, ref offset, callback, state);
            }
            else if (SslStream == null)
            {
                throw new ConnectionLostException();
            }
            Debug.Assert(SslStream.IsAuthenticated);

            int packetSize = GetRecvPacketSize(buffer.Count - offset);
            try
            {
                _readCallback = callback;
                _readResult = SslStream.BeginRead(buffer.Array, buffer.Offset + offset, packetSize, ReadCompleted, state);
                return _readResult.CompletedSynchronously;
            }
            catch (IOException ex)
            {
                if (Network.ConnectionLost(ex))
                {
                    throw new ConnectionLostException(ex);
                }
                if (Network.Timeout(ex))
                {
                    throw new ConnectionTimeoutException();
                }
                throw new TransportException(ex);
            }
            catch (ObjectDisposedException ex)
            {
                throw new ConnectionLostException(ex);
            }
        }

        public bool StartWrite(
            IList<ArraySegment<byte>> buffer,
            int offset,
            AsyncCallback cb,
            object state,
            out bool completed)
        {
            if (!_isConnected)
            {
                return _delegate.StartWrite(buffer, offset, cb, state, out completed);
            }
            else if (SslStream == null)
            {
                throw new ConnectionLostException();
            }

            Debug.Assert(SslStream != null);
            if (!_authenticated)
            {
                completed = false;
                return StartAuthenticate(cb, state);
            }

            // We limit the write packet size to ensure connection timeouts are based on a fixed packet size.
            int remaining = buffer.GetByteCount() - offset;
            int packetSize = GetSendPacketSize(remaining);
            try
            {
                _writeCallback = cb;
                ArraySegment<byte> data = buffer.GetSegment(offset, packetSize);
                _writeResult = SslStream.BeginWrite(data.Array, 0, data.Count, WriteCompleted, state);
                completed = packetSize == remaining;
                return _writeResult.CompletedSynchronously;
            }
            catch (IOException ex)
            {
                if (Network.ConnectionLost(ex))
                {
                    throw new ConnectionLostException(ex);
                }
                if (Network.Timeout(ex))
                {
                    throw new ConnectionTimeoutException();
                }
                throw new TransportException(ex);
            }
            catch (ObjectDisposedException ex)
            {
                throw new ConnectionLostException(ex);
            }
        }

        public string ToDetailedString() => _delegate.ToDetailedString();
        public override string ToString() => _delegate.ToString()!;

        // Force caller to use asynchronous write.
        public int Write(IList<ArraySegment<byte>> buffer, ref int offset) =>
            offset < buffer.GetByteCount() ? SocketOperation.Write : SocketOperation.None;

        // Only for use by TcpEndpoint.
        internal SslTransceiver(Communicator communicator, ITransceiver del, string hostOrAdapterName, bool incoming)
        {
            _communicator = communicator;
            _engine = communicator.SslEngine;
            _delegate = del;
            _incoming = incoming;
            if (_incoming)
            {
                _adapterName = hostOrAdapterName;
            }
            else
            {
                _host = hostOrAdapterName;
            }

            SslStream = null;
        }

        private X509Certificate? CertificateSelectionCallback(
            object sender,
            string targetHost,
            X509CertificateCollection? certs,
            X509Certificate remoteCertificate,
            string[]? acceptableIssuers)
        {
            if (certs == null || certs.Count == 0)
            {
                return null;
            }

            // Use the first certificate that match the acceptable issuers.
            if (acceptableIssuers != null && acceptableIssuers.Length > 0)
            {
                foreach (X509Certificate certificate in certs)
                {
                    if (Array.IndexOf(acceptableIssuers, certificate.Issuer) != -1)
                    {
                        return certificate;
                    }
                }
            }
            return certs[0];
        }

        private void FinishAuthenticate()
        {
            Debug.Assert(_writeResult != null);
            Debug.Assert(SslStream != null);

            try
            {
                if (!_incoming)
                {
                    SslStream.EndAuthenticateAsClient(_writeResult);
                }
                else
                {
                    SslStream.EndAuthenticateAsServer(_writeResult);
                }
            }
            catch (IOException ex)
            {
                if (Network.ConnectionLost(ex))
                {
                    //
                    // This situation occurs when connectToSelf is called; the "remote" end
                    // closes the socket immediately.
                    //
                    throw new ConnectionLostException();
                }
                throw new TransportException(ex);
            }
            catch (AuthenticationException ex)
            {
                throw new TransportException(ex);
            }
        }

        private int GetSendPacketSize(int length) =>
            _maxSendPacketSize > 0 ? Math.Min(length, _maxSendPacketSize) : length;

        public int GetRecvPacketSize(int length) =>
            _maxRecvPacketSize > 0 ? Math.Min(length, _maxRecvPacketSize) : length;

        private void ReadCompleted(IAsyncResult result)
        {
            if (!result.CompletedSynchronously)
            {
                Debug.Assert(_readCallback != null && result.AsyncState != null);
                _readCallback(result.AsyncState);
            }
        }

        private bool RemoteCertificateValidationCallback(
            object sender,
            X509Certificate certificate,
            X509Chain chain,
            SslPolicyErrors errors)
        {
            var message = new StringBuilder();

            if ((errors & SslPolicyErrors.RemoteCertificateNotAvailable) > 0)
            {
                // For an outgoing connection the peer must always provide a certificate, for an incoming connection
                // the certificate is only required if the RequireClientCertificate option was set.
                if (!_incoming || _engine.TlsServerOptions.RequireClientCertificate)
                {
                    if (_engine.SecurityTraceLevel >= 1)
                    {
                        _communicator.Logger.Trace(
                            _engine.SecurityTraceCategory,
                            "SSL certificate validation failed - remote certificate not provided");
                    }
                    return false;
                }
                else
                {
                    errors ^= SslPolicyErrors.RemoteCertificateNotAvailable;
                    message.Append("\nremote certificate not provided (ignored)");
                }
            }

            if ((errors & SslPolicyErrors.RemoteCertificateNameMismatch) > 0)
            {
                if (_engine.SecurityTraceLevel >= 1)
                {
                    _communicator.Logger.Trace(
                        _engine.SecurityTraceCategory,
                        "SSL certificate validation failed - Hostname mismatch");
                }
                return false;
            }

            X509Certificate2Collection? trustedCertificateAuthorities = _incoming ?
                _engine.TlsServerOptions.ClientCertificateCertificateAuthorities :
                _engine.TlsClientOptions.ServerCertificateCertificateAuthorities;

            bool useMachineContext = _incoming ?
                _engine.TlsServerOptions.UseMachineContex : _engine.TlsClientOptions.UseMachineContex;

            bool buildCustomChain =
                (trustedCertificateAuthorities != null || useMachineContext) && certificate != null;
            try
            {
                // If using custom certificate authorities or the machine context and the peer provides a certificate,
                // we rebuild the certificate chain with our custom chain policy.
                if (buildCustomChain)
                {
                    chain = new X509Chain(useMachineContext);
                    chain.ChainPolicy.RevocationMode = X509RevocationMode.NoCheck;

                    if (trustedCertificateAuthorities != null)
                    {
                        // We need to set this flag to be able to use a certificate authority from the extra store.
                        chain.ChainPolicy.VerificationFlags = X509VerificationFlags.AllowUnknownCertificateAuthority;
                        foreach (X509Certificate2 cert in trustedCertificateAuthorities)
                        {
                            chain.ChainPolicy.ExtraStore.Add(cert);
                        }
                    }
                    chain.Build(certificate as X509Certificate2);
                }

                if (chain != null && chain.ChainStatus != null)
                {
                    var chainStatus = new List<X509ChainStatus>(chain.ChainStatus);

                    if (trustedCertificateAuthorities != null)
                    {
                        // Untrusted root is OK when using our custom chain engine if the CA certificate is present in
                        // the chain policy extra store.
                        X509ChainElement root = chain.ChainElements[^1];
                        if (chain.ChainPolicy.ExtraStore.Contains(root.Certificate) &&
                            chainStatus.Exists(status => status.Status == X509ChainStatusFlags.UntrustedRoot))
                        {
                            chainStatus.Remove(
                                chainStatus.Find(status => status.Status == X509ChainStatusFlags.UntrustedRoot));
                            errors ^= SslPolicyErrors.RemoteCertificateChainErrors;
                        }
                        else if (!chain.ChainPolicy.ExtraStore.Contains(root.Certificate) &&
                                 !chainStatus.Exists(status => status.Status == X509ChainStatusFlags.UntrustedRoot))
                        {
                            chainStatus.Add(new X509ChainStatus() { Status = X509ChainStatusFlags.UntrustedRoot });
                            errors |= SslPolicyErrors.RemoteCertificateChainErrors;
                        }
                    }

                    foreach (X509ChainStatus status in chainStatus)
                    {
                        if (status.Status != X509ChainStatusFlags.NoError)
                        {
                            message.Append("\ncertificate chain error: ");
                            message.Append(status.Status);
                            errors |= SslPolicyErrors.RemoteCertificateChainErrors;
                        }
                    }
                }
            }
            finally
            {
                if (buildCustomChain)
                {
                    chain.Dispose();
                }
            }

            if (errors > 0)
            {
                if (_engine.SecurityTraceLevel >= 1)
                {
                    _communicator.Logger.Trace(
                        _engine.SecurityTraceCategory,
                        message.Length > 0 ?
                            $"SSL certificate validation failed: {message}" : "SSL certificate validation failed");
                }
                return false;
            }

            if (message.Length > 0 && _engine.SecurityTraceLevel >= 1)
            {
                _communicator.Logger.Trace(_engine.SecurityTraceCategory,
                    $"SSL certificate validation status: {message}");
            }
            return true;
        }

        private bool StartAuthenticate(AsyncCallback callback, object state)
        {
            Debug.Assert(SslStream != null);
            try
            {
                _writeCallback = callback;
                if (_incoming)
                {
                    _writeResult = SslStream.BeginAuthenticateAsServer(
                        _engine.TlsServerOptions.ServerCertificate,
                        _engine.TlsServerOptions.RequireClientCertificate,
                        _engine.TlsServerOptions.EnabledSslProtocols!.Value,
                        false,
                        WriteCompleted,
                        state);
                }
                else
                {
                    _writeResult = SslStream.BeginAuthenticateAsClient(
                        _host,
                        _engine.TlsClientOptions.ClientCertificates,
                        _engine.TlsClientOptions.EnabledSslProtocols!.Value,
                        false,
                        WriteCompleted,
                        state);
                }
            }
            catch (IOException ex)
            {
                if (Network.ConnectionLost(ex))
                {
                    //
                    // This situation occurs when connectToSelf is called; the "remote" end
                    // closes the socket immediately.
                    //
                    throw new ConnectionLostException();
                }
                throw new TransportException(ex);
            }
            catch (AuthenticationException ex)
            {
                throw new TransportException(ex);
            }

            Debug.Assert(_writeResult != null);
            return _writeResult.CompletedSynchronously;
        }

        internal void WriteCompleted(IAsyncResult result)
        {
            if (!result.CompletedSynchronously)
            {
                Debug.Assert(_writeCallback != null && result.AsyncState != null);
                _writeCallback(result.AsyncState);
            }
        }
    }
}
