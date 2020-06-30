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

        public Connection CreateConnection(
            Endpoint endpoint,
            IACMMonitor? monitor,
            IConnector? connector,
            ObjectAdapter? adapter)
        {
            Debug.Assert(endpoint.IsSecure);
            return new TcpConnection(endpoint, monitor, this, connector, adapter);
        }

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

            Socket? fd = _delegate.Fd();
            Debug.Assert(fd != null);

            Network.SetBlock(fd, true); // SSL requires a blocking socket

            // For timeouts to work properly, we need to receive/send the data in several chunks. Otherwise, we would
            // only be notified when all the data is received/written. The connection timeout could easily be triggered
            // when receiving/sending large frames.
            _maxSendPacketSize = Math.Max(512, Network.GetSendBufferSize(fd));
            _maxRecvPacketSize = Math.Max(512, Network.GetRecvBufferSize(fd));

            if (SslStream == null)
            {
                try
                {
                    if (_incoming)
                    {
                        SslStream = new SslStream(
                            new NetworkStream(fd, false),
                            false,
                            _engine.ClientCertificateValidationCallback ?? RemoteCertificateValidationCallback,
                            _engine.ServerCertificateSelectionCallback ?? CertificateSelectionCallback);
                    }
                    else
                    {
                        SslStream = new SslStream(
                            new NetworkStream(fd, false),
                            false,
                            _engine.ServerCertificateValidationCallback ?? RemoteCertificateValidationCallback,
                            _engine.ClientCertificateSelectionCallback ?? CertificateSelectionCallback);
                    }
                }
                catch (IOException ex)
                {
                    if (Network.ConnectionLost(ex))
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
                string msg = string.Format("{0} connection rejected by trust manager\n{1}",
                    _incoming ? "incoming" : "outgoing",
                    description);
                if (_engine.SecurityTraceLevel >= 1)
                {
                    _communicator.Logger.Trace(_engine.SecurityTraceCategory, msg);
                }

                throw new TransportException(msg);
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
                SslStream.Close(); // Closing the stream also closes the socket.
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
            X509Chain _,
            SslPolicyErrors policyErrors)
        {
            var chain = new X509Chain(_engine.UseMachineContext);

            chain.ChainPolicy.RevocationMode = _incoming ? _engine.ServerCertificateRevocationCheckMode :
                                                           _engine.ClientCertificateRevocationCheckMode;

            X509Certificate2Collection? caCerts = _incoming ? _engine.ClientCertificateCertificateAuthorities :
                                                              _engine.ServerCertificateCertificateAuthorities;
            if (caCerts != null)
            {
                // We need to set this flag to be able to use a certificate authority from the extra store.
                chain.ChainPolicy.VerificationFlags = X509VerificationFlags.AllowUnknownCertificateAuthority;
                foreach (X509Certificate2 cert in caCerts)
                {
                    chain.ChainPolicy.ExtraStore.Add(cert);
                }
            }

            var message = new StringBuilder();
            int errors = (int)policyErrors;
            if (certificate != null)
            {
                chain.Build(new X509Certificate2(certificate));
                if (chain.ChainStatus != null && chain.ChainStatus.Length > 0)
                {
                    errors |= (int)SslPolicyErrors.RemoteCertificateChainErrors;
                }
                else if (caCerts != null)
                {
                    X509ChainElement e = chain.ChainElements[^1];
                    if (!chain.ChainPolicy.ExtraStore.Contains(e.Certificate))
                    {
                        message.Append("\nuntrusted root certificate");
                        errors |= (int)SslPolicyErrors.RemoteCertificateChainErrors;
                    }
                    else
                    {
                        return true;
                    }
                }
                else
                {
                    return true;
                }
            }

            if ((errors & (int)SslPolicyErrors.RemoteCertificateNotAvailable) > 0)
            {
                // The RemoteCertificateNotAvailable case is not possible for an outgoing connection. Since .NET
                // requires an authenticated connection, the remote peer closes the socket if it does not have a
                // certificate to provide.
                if (_incoming)
                {
                    if (_engine.RequireClientCertificate)
                    {
                        if (_engine.SecurityTraceLevel >= 1)
                        {
                            _communicator.Logger.Trace(
                                _engine.SecurityTraceCategory,
                                "SSL certificate validation failed - client certificate not provided");
                        }
                        return false;
                    }

                    errors ^= (int)SslPolicyErrors.RemoteCertificateNotAvailable;
                    message.Append("\nremote certificate not provided (ignored)");
                }
            }

            if ((errors & (int)SslPolicyErrors.RemoteCertificateNameMismatch) > 0)
            {
                if (_engine.SecurityTraceLevel >= 1)
                {
                    _communicator.Logger.Trace(
                        _engine.SecurityTraceCategory,
                        "SSL certificate validation failed - Hostname mismatch");
                }
                return false;
            }

            if ((errors & (int)SslPolicyErrors.RemoteCertificateChainErrors) > 0 &&
                chain.ChainStatus != null && chain.ChainStatus.Length > 0)
            {
                int errorCount = 0;
                foreach (X509ChainStatus status in chain.ChainStatus)
                {
                    if (status.Status == X509ChainStatusFlags.UntrustedRoot && caCerts != null)
                    {
                        // Untrusted root is OK when using our custom chain engine if the CA certificate is present
                        // in the chain policy extra store.
                        X509ChainElement root = chain.ChainElements[^1];
                        if (!chain.ChainPolicy.ExtraStore.Contains(root.Certificate))
                        {
                            message.Append("\nuntrusted root certificate");
                            ++errorCount;
                        }
                    }
                    else if (status.Status == X509ChainStatusFlags.PartialChain)
                    {
                        message.Append("\npartial certificate chain");
                        ++errorCount;
                    }
                    else if (status.Status != X509ChainStatusFlags.NoError)
                    {
                        message.Append("\ncertificate chain error: ");
                        message.Append(status.Status);
                        ++errorCount;
                    }
                }

                if (errorCount == 0)
                {
                    errors ^= (int)SslPolicyErrors.RemoteCertificateChainErrors;
                }
            }

            if (errors > 0)
            {
                if (_engine.SecurityTraceLevel >= 1)
                {
                    if (message.Length > 0)
                    {
                        _communicator.Logger.Trace(_engine.SecurityTraceCategory,
                            $"SSL certificate validation failed: {message}");
                    }
                    else
                    {
                        _communicator.Logger.Trace(_engine.SecurityTraceCategory, "SSL certificate validation failed");
                    }
                }
                return false;
            }
            else if (message.Length > 0 && _engine.SecurityTraceLevel >= 1)
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
                    _writeResult = SslStream.BeginAuthenticateAsServer(_engine.ServerCertificate,
                                                                       _engine.RequireClientCertificate,
                                                                       _engine.ServerEnabledSslProtocols,
                                                                       false,
                                                                       WriteCompleted,
                                                                       state);
                }
                else
                {
                    _writeResult = SslStream.BeginAuthenticateAsClient(_host,
                                                                       _engine.ClientCertificates,
                                                                       _engine.ClientEnabledSslProtocols,
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
