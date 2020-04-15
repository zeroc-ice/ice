//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;

namespace IceSSL
{
    internal sealed class Transceiver : IceInternal.ITransceiver
    {
        public Socket? Fd() => _delegate.Fd();

        public int Initialize(ref ArraySegment<byte> readBuffer, IList<ArraySegment<byte>> writeBuffer)
        {
            if (!_isConnected)
            {
                int status = _delegate.Initialize(ref readBuffer, writeBuffer);
                if (status != IceInternal.SocketOperation.None)
                {
                    return status;
                }
                _isConnected = true;
            }

            Socket? fd = _delegate.Fd();
            Debug.Assert(fd != null);

            IceInternal.Network.SetBlock(fd, true); // SSL requires a blocking socket

            //
            // For timeouts to work properly, we need to receive/send
            // the data in several chunks. Otherwise, we would only be
            // notified when all the data is received/written. The
            // connection timeout could easily be triggered when
            // receiving/sending large messages.
            //
            _maxSendPacketSize = Math.Max(512, IceInternal.Network.GetSendBufferSize(fd));
            _maxRecvPacketSize = Math.Max(512, IceInternal.Network.GetRecvBufferSize(fd));

            if (_sslStream == null)
            {
                try
                {
                    _sslStream = new SslStream(new NetworkStream(fd, false),
                                               false,
                                               new RemoteCertificateValidationCallback(ValidationCallback),
                                               new LocalCertificateSelectionCallback(SelectCertificate));
                }
                catch (IOException ex)
                {
                    if (IceInternal.Network.ConnectionLost(ex))
                    {
                        throw new Ice.ConnectionLostException(ex);
                    }
                    else
                    {
                        throw new Ice.TransportException(ex);
                    }
                }
                return IceInternal.SocketOperation.Connect;
            }

            Debug.Assert(_sslStream.IsAuthenticated);
            _authenticated = true;

            _cipher = _sslStream.CipherAlgorithm.ToString();
            _instance.VerifyPeer((ConnectionInfo)GetInfo(), ToString());

            if (_instance.SecurityTraceLevel() >= 1)
            {
                _instance.TraceStream(_sslStream, ToString());
            }
            return IceInternal.SocketOperation.None;
        }

        public int Closing(bool initiator, System.Exception? ex) => _delegate.Closing(initiator, ex);

        public void Close()
        {
            if (_sslStream != null)
            {
                _sslStream.Close(); // Closing the stream also closes the socket.
                _sslStream = null;
            }

            _delegate.Close();
        }

        public Ice.Endpoint Bind()
        {
            Debug.Assert(false);
            return null;
        }

        public void Destroy() => _delegate.Destroy();

        // Force caller to use asynchronous write.
        public int Write(IList<ArraySegment<byte>> buffer, ref int offset) =>
            offset < buffer.GetByteCount() ? IceInternal.SocketOperation.Write : IceInternal.SocketOperation.None;

        // Force caller to use asynchronous read.
        public int Read(ref ArraySegment<byte> buffer, ref int offset) =>
            offset < buffer.Count ? IceInternal.SocketOperation.Read : IceInternal.SocketOperation.None;

        public bool StartRead(ref ArraySegment<byte> buffer, ref int offset, IceInternal.AsyncCallback callback, object state)
        {
            if (!_isConnected)
            {
                return _delegate.StartRead(ref buffer, ref offset, callback, state);
            }

            Debug.Assert(_sslStream != null && _sslStream.IsAuthenticated);

            int packetSize = GetRecvPacketSize(buffer.Count - offset);
            try
            {
                _readCallback = callback;
                _readResult = _sslStream.BeginRead(buffer.Array, buffer.Offset + offset, packetSize, ReadCompleted, state);
                return _readResult.CompletedSynchronously;
            }
            catch (IOException ex)
            {
                if (IceInternal.Network.ConnectionLost(ex))
                {
                    throw new ConnectionLostException(ex);
                }
                if (IceInternal.Network.Timeout(ex))
                {
                    throw new Ice.ConnectionTimeoutException();
                }
                throw new Ice.TransportException(ex);
            }
            catch (ObjectDisposedException ex)
            {
                throw new ConnectionLostException(ex);
            }
        }

        public void FinishRead(ref ArraySegment<byte> buffer, ref int offset)
        {
            if (!_isConnected)
            {
                _delegate.FinishRead(ref buffer, ref offset);
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
                int ret = _sslStream.EndRead(_readResult);
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
                if (IceInternal.Network.ConnectionLost(ex))
                {
                    throw new ConnectionLostException(ex);
                }
                if (IceInternal.Network.Timeout(ex))
                {
                    throw new Ice.ConnectionTimeoutException();
                }
                throw new Ice.TransportException(ex);
            }
            catch (ObjectDisposedException ex)
            {
                throw new ConnectionLostException(ex);
            }
        }

        public bool
        StartWrite(IList<ArraySegment<byte>> buffer, int offset, IceInternal.AsyncCallback cb, object state, out bool completed)
        {
            if (!_isConnected)
            {
                return _delegate.StartWrite(buffer, offset, cb, state, out completed);
            }

            Debug.Assert(_sslStream != null);
            if (!_authenticated)
            {
                completed = false;
                return StartAuthenticate(cb, state);
            }

            //
            // We limit the packet size for beingWrite to ensure connection timeouts are based
            // on a fixed packet size.
            //
            int remaining = buffer.GetByteCount() - offset;
            int packetSize = GetSendPacketSize(remaining);
            try
            {
                _writeCallback = cb;
                ArraySegment<byte> data = buffer.GetSegment(offset, packetSize);
                _writeResult = _sslStream.BeginWrite(data.Array, 0, data.Count, WriteCompleted, state);
                completed = packetSize == remaining;
                return _writeResult.CompletedSynchronously;
            }
            catch (IOException ex)
            {
                if (IceInternal.Network.ConnectionLost(ex))
                {
                    throw new ConnectionLostException(ex);
                }
                if (IceInternal.Network.Timeout(ex))
                {
                    throw new Ice.ConnectionTimeoutException();
                }
                throw new Ice.TransportException(ex);
            }
            catch (ObjectDisposedException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
        }

        public void FinishWrite(IList<ArraySegment<byte>> buffer, ref int offset)
        {
            if (!_isConnected)
            {
                _delegate.FinishWrite(buffer, ref offset);
                return;
            }
            else if (_sslStream == null) // Transceiver was closed
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
                _sslStream.EndWrite(_writeResult);
                offset += bytesTransferred;
            }
            catch (IOException ex)
            {
                if (IceInternal.Network.ConnectionLost(ex))
                {
                    throw new ConnectionLostException(ex);
                }
                if (IceInternal.Network.Timeout(ex))
                {
                    throw new Ice.ConnectionTimeoutException();
                }
                throw new Ice.TransportException(ex);
            }
            catch (ObjectDisposedException ex)
            {
                throw new ConnectionLostException(ex);
            }
        }

        public string Transport() => _delegate.Transport();

        public Ice.ConnectionInfo GetInfo()
        {
            var info = new ConnectionInfo();
            info.Underlying = _delegate.GetInfo();
            info.Incoming = _incoming;
            info.AdapterName = _adapterName;
            info.Cipher = _cipher;
            info.Certs = _certs;
            info.Verified = _verified;
            return info;
        }

        public void CheckSendSize(int size) => _delegate.CheckSendSize(size);

        public void SetBufferSize(int rcvSize, int sndSize) => _delegate.SetBufferSize(rcvSize, sndSize);

        public override string ToString() => _delegate.ToString()!;

        public string ToDetailedString() => _delegate.ToDetailedString();

        //
        // Only for use by ConnectorI, AcceptorI.
        //
        internal Transceiver(Instance instance, IceInternal.ITransceiver del, string hostOrAdapterName, bool incoming)
        {
            _instance = instance;
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

            _sslStream = null;

            _verifyPeer = _instance.Communicator.GetPropertyAsInt("IceSSL.VerifyPeer") ?? 2;
        }

        private bool StartAuthenticate(IceInternal.AsyncCallback callback, object state)
        {
            Debug.Assert(_sslStream != null);
            try
            {
                _writeCallback = callback;
                if (!_incoming)
                {
                    //
                    // Client authentication.
                    //
                    _writeResult = _sslStream.BeginAuthenticateAsClient(_host,
                                                                        _instance.Certs(),
                                                                        _instance.Protocols(),
                                                                        _instance.CheckCRL() > 0,
                                                                        WriteCompleted,
                                                                        state);
                }
                else
                {
                    //
                    // Server authentication.
                    //
                    // Get the certificate collection and select the first one.
                    //
                    X509Certificate2Collection? certs = _instance.Certs();
                    X509Certificate2? cert = null;
                    if (certs != null && certs.Count > 0)
                    {
                        cert = certs[0];
                    }

                    _writeResult = _sslStream.BeginAuthenticateAsServer(cert,
                                                                        _verifyPeer > 0,
                                                                        _instance.Protocols(),
                                                                        _instance.CheckCRL() > 0,
                                                                        WriteCompleted,
                                                                        state);
                }
            }
            catch (IOException ex)
            {
                if (IceInternal.Network.ConnectionLost(ex))
                {
                    //
                    // This situation occurs when connectToSelf is called; the "remote" end
                    // closes the socket immediately.
                    //
                    throw new Ice.ConnectionLostException();
                }
                throw new Ice.TransportException(ex);
            }
            catch (AuthenticationException ex)
            {
                throw new Ice.SecurityException(ex);
            }

            Debug.Assert(_writeResult != null);
            return _writeResult.CompletedSynchronously;
        }

        private void FinishAuthenticate()
        {
            Debug.Assert(_writeResult != null);
            Debug.Assert(_sslStream != null);

            try
            {
                if (!_incoming)
                {
                    _sslStream.EndAuthenticateAsClient(_writeResult);
                }
                else
                {
                    _sslStream.EndAuthenticateAsServer(_writeResult);
                }
            }
            catch (IOException ex)
            {
                if (IceInternal.Network.ConnectionLost(ex))
                {
                    //
                    // This situation occurs when connectToSelf is called; the "remote" end
                    // closes the socket immediately.
                    //
                    throw new Ice.ConnectionLostException();
                }
                throw new Ice.TransportException(ex);
            }
            catch (AuthenticationException ex)
            {
                throw new SecurityException(ex);
            }
        }

        private X509Certificate? SelectCertificate(object sender,
            string targetHost,
            X509CertificateCollection? certs,
            X509Certificate remoteCertificate,
            string[]? acceptableIssuers)
        {
            if (certs == null || certs.Count == 0)
            {
                return null;
            }
            else if (certs.Count == 1)
            {
                return certs[0];
            }

            //
            // Use the first certificate that match the acceptable issuers.
            //
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

        private bool ValidationCallback(object sender, X509Certificate certificate, X509Chain chainEngine,
                                        SslPolicyErrors policyErrors)
        {
            var chain = new X509Chain(_instance.Engine().UseMachineContext());
            try
            {
                if (_instance.CheckCRL() == 0)
                {
                    chain.ChainPolicy.RevocationMode = X509RevocationMode.NoCheck;
                }

                X509Certificate2Collection? caCerts = _instance.Engine().CaCerts();
                if (caCerts != null)
                {
                    //
                    // We need to set this flag to be able to use a certificate authority from the extra store.
                    //
                    chain.ChainPolicy.VerificationFlags = X509VerificationFlags.AllowUnknownCertificateAuthority;
                    foreach (X509Certificate2 cert in caCerts)
                    {
                        chain.ChainPolicy.ExtraStore.Add(cert);
                    }
                }

                string message = "";
                int errors = (int)policyErrors;
                if (certificate != null)
                {
                    chain.Build(new X509Certificate2(certificate));
                    if (chain.ChainStatus != null && chain.ChainStatus.Length > 0)
                    {
                        errors |= (int)SslPolicyErrors.RemoteCertificateChainErrors;
                    }
                    else if (_instance.Engine().CaCerts() != null)
                    {
                        X509ChainElement e = chain.ChainElements[chain.ChainElements.Count - 1];
                        if (!chain.ChainPolicy.ExtraStore.Contains(e.Certificate))
                        {
                            if (_verifyPeer > 0)
                            {
                                message += "\nuntrusted root certificate";
                            }
                            else
                            {
                                message += "\nuntrusted root certificate (ignored)";
                                _verified = false;
                            }
                            errors |= (int)SslPolicyErrors.RemoteCertificateChainErrors;
                        }
                        else
                        {
                            _verified = true;
                            return true;
                        }
                    }
                    else
                    {
                        _verified = true;
                        return true;
                    }
                }

                if ((errors & (int)SslPolicyErrors.RemoteCertificateNotAvailable) > 0)
                {
                    //
                    // The RemoteCertificateNotAvailable case does not appear to be possible
                    // for an outgoing connection. Since .NET requires an authenticated
                    // connection, the remote peer closes the socket if it does not have a
                    // certificate to provide.
                    //

                    if (_incoming)
                    {
                        if (_verifyPeer > 1)
                        {
                            if (_instance.SecurityTraceLevel() >= 1)
                            {
                                _instance.Logger.Trace(_instance.SecurityTraceCategory(),
                                    "SSL certificate validation failed - client certificate not provided");
                            }
                            return false;
                        }
                        errors ^= (int)SslPolicyErrors.RemoteCertificateNotAvailable;
                        message += "\nremote certificate not provided (ignored)";
                    }
                }

                bool certificateNameMismatch = (errors & (int)SslPolicyErrors.RemoteCertificateNameMismatch) > 0;
                if (certificateNameMismatch)
                {
                    if (_instance.Engine().GetCheckCertName() && !string.IsNullOrEmpty(_host))
                    {
                        if (_instance.SecurityTraceLevel() >= 1)
                        {
                            string msg = "SSL certificate validation failed - Hostname mismatch";
                            if (_verifyPeer == 0)
                            {
                                msg += " (ignored)";
                            }
                            _instance.Logger.Trace(_instance.SecurityTraceCategory(), msg);
                        }

                        if (_verifyPeer > 0)
                        {
                            return false;
                        }
                        else
                        {
                            errors ^= (int)SslPolicyErrors.RemoteCertificateNameMismatch;
                        }
                    }
                    else
                    {
                        errors ^= (int)SslPolicyErrors.RemoteCertificateNameMismatch;
                        certificateNameMismatch = false;
                    }
                }

                if ((errors & (int)SslPolicyErrors.RemoteCertificateChainErrors) > 0 &&
                   chain.ChainStatus != null && chain.ChainStatus.Length > 0)
                {
                    int errorCount = 0;
                    foreach (X509ChainStatus status in chain.ChainStatus)
                    {
                        if (status.Status == X509ChainStatusFlags.UntrustedRoot && _instance.Engine().CaCerts() != null)
                        {
                            //
                            // Untrusted root is OK when using our custom chain engine if
                            // the CA certificate is present in the chain policy extra store.
                            //
                            X509ChainElement e = chain.ChainElements[chain.ChainElements.Count - 1];
                            if (!chain.ChainPolicy.ExtraStore.Contains(e.Certificate))
                            {
                                if (_verifyPeer > 0)
                                {
                                    message += "\nuntrusted root certificate";
                                    ++errorCount;
                                }
                                else
                                {
                                    message += "\nuntrusted root certificate (ignored)";
                                }
                            }
                            else
                            {
                                _verified = !certificateNameMismatch;
                            }
                        }
                        else if (status.Status == X509ChainStatusFlags.Revoked)
                        {
                            if (_instance.CheckCRL() > 0)
                            {
                                message += "\ncertificate revoked";
                                ++errorCount;
                            }
                            else
                            {
                                message += "\ncertificate revoked (ignored)";
                            }
                        }
                        else if (status.Status == X509ChainStatusFlags.RevocationStatusUnknown)
                        {
                            //
                            // If a certificate's revocation status cannot be determined, the strictest
                            // policy is to reject the connection.
                            //
                            if (_instance.CheckCRL() > 1)
                            {
                                message += "\ncertificate revocation status unknown";
                                ++errorCount;
                            }
                            else
                            {
                                message += "\ncertificate revocation status unknown (ignored)";
                            }
                        }
                        else if (status.Status == X509ChainStatusFlags.PartialChain)
                        {
                            if (_verifyPeer > 0)
                            {
                                message += "\npartial certificate chain";
                                ++errorCount;
                            }
                            else
                            {
                                message += "\npartial certificate chain (ignored)";
                            }
                        }
                        else if (status.Status != X509ChainStatusFlags.NoError)
                        {
                            message = message + "\ncertificate chain error: " + status.Status.ToString();
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
                    if (_instance.SecurityTraceLevel() >= 1)
                    {
                        if (message.Length > 0)
                        {
                            _instance.Logger.Trace(_instance.SecurityTraceCategory(),
                                $"SSL certificate validation failed:{message}");
                        }
                        else
                        {
                            _instance.Logger.Trace(_instance.SecurityTraceCategory(), "SSL certificate validation failed");
                        }
                    }
                    return false;
                }
                else if (message.Length > 0 && _instance.SecurityTraceLevel() >= 1)
                {
                    _instance.Logger.Trace(_instance.SecurityTraceCategory(), $"SSL certificate validation status:{message}");
                }
                return true;
            }
            finally
            {
                if (chain.ChainElements != null && chain.ChainElements.Count > 0)
                {
                    _certs = new X509Certificate2[chain.ChainElements.Count];
                    for (int i = 0; i < chain.ChainElements.Count; ++i)
                    {
                        _certs[i] = chain.ChainElements[i].Certificate;
                    }
                }

                try
                {
                    chain.Dispose();
                }
                catch (System.Exception)
                {
                }
            }
        }

        internal void ReadCompleted(IAsyncResult result)
        {
            if (!result.CompletedSynchronously)
            {
                Debug.Assert(_readCallback != null && result.AsyncState != null);
                _readCallback(result.AsyncState);
            }
        }

        internal void WriteCompleted(IAsyncResult result)
        {
            if (!result.CompletedSynchronously)
            {
                Debug.Assert(_writeCallback != null && result.AsyncState != null);
                _writeCallback(result.AsyncState);
            }
        }

        private int GetSendPacketSize(int length) => _maxSendPacketSize > 0 ? Math.Min(length, _maxSendPacketSize) : length;

        public int GetRecvPacketSize(int length) => _maxRecvPacketSize > 0 ? Math.Min(length, _maxRecvPacketSize) : length;

        private readonly Instance _instance;
        private readonly IceInternal.ITransceiver _delegate;
        private readonly string? _host;
        private readonly string? _adapterName;
        private readonly bool _incoming;
        private SslStream? _sslStream;
        private readonly int _verifyPeer;
        private bool _isConnected;
        private bool _authenticated;
        private IAsyncResult? _writeResult;
        private IAsyncResult? _readResult;
        private IceInternal.AsyncCallback? _readCallback;
        private IceInternal.AsyncCallback? _writeCallback;
        private int _maxSendPacketSize;
        private int _maxRecvPacketSize;
        private string? _cipher;
        private X509Certificate2[]? _certs;
        private bool _verified;
    }
}
