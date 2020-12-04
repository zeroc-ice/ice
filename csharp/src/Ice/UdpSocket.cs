// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Net.Security;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    internal sealed class UdpSocket : SingleStreamSocket
    {
        public override Socket Socket { get; }
        public override SslStream? SslStream => null;

        internal IPEndPoint? MulticastAddress { get; private set; }

        // The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for the UDP header
        // to get the maximum payload.
        private const int MaxPacketSize = 65535 - UdpOverhead;
        private const int UdpOverhead = 20 + 8;

        private IPEndPoint _addr;
        private readonly Communicator _communicator;
        private readonly bool _incoming;
        private readonly string? _multicastInterface;
        private EndPoint? _peerAddr;
        private readonly int _rcvSize;
        private readonly int _sndSize;
        private readonly IPEndPoint? _sourceAddr;

        public Endpoint Bind(UdpEndpoint endpoint)
        {
            Debug.Assert(_incoming);
            try
            {
                if (Network.IsMulticast(_addr))
                {
                    Socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ExclusiveAddressUse, false);
                    Socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);

                    MulticastAddress = _addr;
                    if (OperatingSystem.IsWindows())
                    {
                        // Windows does not allow binding to the multicast address itself so we bind to INADDR_ANY
                        // instead. As a result, bidirectional connection won't work because the source address won't
                        // be the multicast address and the client will therefore reject the datagram.
                        if (_addr.AddressFamily == AddressFamily.InterNetwork)
                        {
                            _addr = new IPEndPoint(IPAddress.Any, _addr.Port);
                        }
                        else
                        {
                            _addr = new IPEndPoint(IPAddress.IPv6Any, _addr.Port);
                        }
                    }

                    Socket.Bind(_addr);
                    _addr = (IPEndPoint)Socket.LocalEndPoint!;

                    if (endpoint.Port == 0)
                    {
                        MulticastAddress.Port = _addr.Port;
                    }

                    Network.SetMulticastGroup(Socket, MulticastAddress.Address, _multicastInterface);
                }
                else
                {
                    Socket.Bind(_addr);
                    _addr = (IPEndPoint)Socket.LocalEndPoint!;
                }
            }
            catch (SocketException ex)
            {
                throw new TransportException(ex);
            }

            Debug.Assert(endpoint != null);
            return endpoint.Clone((ushort)_addr.Port);
        }

        public override ValueTask CloseAsync(Exception exception, CancellationToken cancel) => new ValueTask();

        public override async ValueTask InitializeAsync(CancellationToken cancel)
        {
            if (!_incoming)
            {
                try
                {
                    if (_sourceAddr != null)
                    {
                        Socket.Bind(_sourceAddr);
                    }

                    await Socket.ConnectAsync(_addr, cancel).ConfigureAwait(false);
                }
                catch (Exception ex)
                {
                    throw new ConnectFailedException(ex, RetryPolicy.AfterDelay(TimeSpan.Zero));
                }
            }
        }

        public override async ValueTask<ArraySegment<byte>> ReceiveDatagramAsync(CancellationToken cancel)
        {
            int packetSize = Math.Min(MaxPacketSize, _rcvSize - UdpOverhead);
            ArraySegment<byte> buffer = new byte[packetSize];

            int received = 0;
            try
            {
                // TODO: Workaround for https://github.com/dotnet/corefx/issues/31182
                if (!_incoming ||
                    (OperatingSystem.IsMacOS() &&
                     Socket.AddressFamily == AddressFamily.InterNetworkV6 && Socket.DualMode))
                {
                    received = await Socket.ReceiveAsync(buffer, SocketFlags.None, cancel).ConfigureAwait(false);
                }
                else
                {
                    EndPoint? peerAddr = _peerAddr;
                    if (peerAddr == null)
                    {
                        if (_addr.AddressFamily == AddressFamily.InterNetwork)
                        {
                            peerAddr = new IPEndPoint(IPAddress.Any, 0);
                        }
                        else
                        {
                            Debug.Assert(_addr.AddressFamily == AddressFamily.InterNetworkV6);
                            peerAddr = new IPEndPoint(IPAddress.IPv6Any, 0);
                        }
                    }

                    // TODO: Use the cancellable API once https://github.com/dotnet/runtime/issues/33418 is fixed
                    SocketReceiveFromResult result =
                        await Socket.ReceiveFromAsync(buffer,
                                                      SocketFlags.None,
                                                      peerAddr).WaitAsync(cancel).ConfigureAwait(false);
                    _peerAddr = result.RemoteEndPoint;
                    received = result.ReceivedBytes;
                }
            }
            catch (SocketException e) when (e.SocketErrorCode == SocketError.MessageSize)
            {
                // Ignore and return an empty buffer if the datagram is too large.
            }
            catch (SocketException e)
            {
                if (e.IsConnectionLost())
                {
                    throw new ConnectionLostException(RetryPolicy.AfterDelay(TimeSpan.Zero));
                }
                throw new TransportException(e, RetryPolicy.AfterDelay(TimeSpan.Zero));
            }

            return buffer.Slice(0, received);
        }

        public override ValueTask<int> ReceiveAsync(Memory<byte> buffer, CancellationToken cancel) =>
            throw new InvalidOperationException();

        public override async ValueTask<int> SendAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel)
        {
            int count = buffer.GetByteCount();

            if (_incoming && _peerAddr == null)
            {
                throw new TransportException("cannot send datagram to undefined peer", RetryPolicy.NoRetry);
            }

            try
            {
                if (!_incoming)
                {
                    // TODO: Use cancellable API once https://github.com/dotnet/runtime/issues/33417 is fixed.
                    return await Socket.SendAsync(buffer, SocketFlags.None).WaitAsync(cancel).ConfigureAwait(false);
                }
                else
                {
                    Debug.Assert(_peerAddr != null);
                    // TODO: Fix to use the cancellable API with 5.0
                    return await Socket.SendToAsync(buffer.GetSegment(0, count),
                                                    SocketFlags.None,
                                                    _peerAddr).WaitAsync(cancel).ConfigureAwait(false);
                }
            }
            catch (SocketException ex) when (ex.SocketErrorCode == SocketError.MessageSize)
            {
                // Don't retry if the datagram can't be sent because its too large.
                throw new TransportException(ex, RetryPolicy.NoRetry);
            }
            catch (Exception ex)
            {
                if (ex.IsConnectionLost())
                {
                    throw new ConnectionLostException(ex, RetryPolicy.AfterDelay(TimeSpan.Zero));
                }
                throw new TransportException(ex, RetryPolicy.AfterDelay(TimeSpan.Zero));
            }
        }

        public override string ToString()
        {
            try
            {
                var sb = new StringBuilder();
                if (_incoming)
                {
                    sb.Append("local address = " + Network.LocalAddrToString(Network.GetLocalAddress(Socket)));
                    if (_peerAddr != null)
                    {
                        sb.Append($"\nremote address = {_peerAddr}");
                    }
                }
                else
                {
                    sb.Append(Network.SocketToString(Socket));
                }
                if (MulticastAddress != null)
                {
                    sb.Append($"\nmulticast address = {MulticastAddress}");
                }

                List<string> interfaces;
                if (MulticastAddress == null)
                {
                    interfaces = Network.GetHostsForEndpointExpand(_addr.ToString(), Network.EnableBoth, true);
                }
                else
                {
                    interfaces = Network.GetInterfacesForMulticast(_multicastInterface,
                                                                   Network.GetIPVersion(MulticastAddress.Address));
                }
                if (interfaces.Count != 0)
                {
                    sb.Append("\nlocal interfaces = ");
                    sb.Append(string.Join(", ", interfaces));
                }
                return sb.ToString();
            }
            catch (ObjectDisposedException)
            {
                return "<closed>";
            }
        }

        protected override void Dispose(bool disposing) => Socket.Dispose();

        // Only for use by UdpEndpoint.
        internal UdpSocket(
            Communicator communicator,
            EndPoint addr,
            IPAddress? sourceAddr,
            string? multicastInterface,
            int multicastTtl)
        {
            _communicator = communicator;
            _addr = (IPEndPoint)addr;
            _multicastInterface = multicastInterface;
            _incoming = false;
            if (sourceAddr != null)
            {
                _sourceAddr = new IPEndPoint(sourceAddr, 0);
            }

            Socket = Network.CreateSocket(true, _addr.AddressFamily);
            try
            {
                Network.SetBufSize(Socket, _communicator, Transport.UDP);
                _rcvSize = (int)Socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer)!;
                _sndSize = (int)Socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer)!;

                if (Network.IsMulticast(_addr))
                {
                    if (_multicastInterface != null)
                    {
                        Debug.Assert(_multicastInterface.Length > 0);
                        Network.SetMulticastInterface(Socket, _multicastInterface, _addr.AddressFamily);
                    }
                    if (multicastTtl != -1)
                    {
                        Network.SetMulticastTtl(Socket, multicastTtl, _addr.AddressFamily);
                    }
                }
            }
            catch (SocketException ex)
            {
                Socket.CloseNoThrow();
                throw new TransportException(ex, RetryPolicy.NoRetry);
            }
        }

        // Only for use by UdpEndpoint.
        internal UdpSocket(UdpEndpoint endpoint, Communicator communicator)
        {
            Debug.Assert(endpoint.Address != IPAddress.None); // not a DNS name

            _communicator = communicator;
            _addr = new IPEndPoint(endpoint.Address, endpoint.Port);
            _multicastInterface = endpoint.MulticastInterface;
            _incoming = true;

            Socket = Network.CreateServerSocket(endpoint, _addr.AddressFamily);
            try
            {
                Network.SetBufSize(Socket, _communicator, Transport.UDP);
                _rcvSize = (int)Socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer)!;
                _sndSize = (int)Socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer)!;
            }
            catch (SocketException ex)
            {
                Socket.CloseNoThrow();
                throw new TransportException(ex, RetryPolicy.NoRetry);
            }
        }
    }
}
