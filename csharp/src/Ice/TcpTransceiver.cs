//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    internal sealed class TcpTransceiver : ITransceiver
    {
        public Socket Socket { get; }

        private readonly EndPoint? _addr;
        private readonly Communicator _communicator;
        private string _desc;
        private readonly INetworkProxy? _proxy;
        private readonly IPAddress? _sourceAddr;

        public void CheckSendSize(int size)
        {
        }

        public ValueTask CloseAsync(Exception ex, CancellationToken cancel) => new ValueTask();

        public ValueTask DisposeAsync()
        {
            Socket.Dispose();
            return new ValueTask();
        }

        public async ValueTask InitializeAsync(CancellationToken cancel)
        {
            if (_addr != null)
            {
                try
                {
                    // Bind the socket to the source address if one is set.
                    if (_sourceAddr != null)
                    {
                        Socket.Bind(new IPEndPoint(_sourceAddr, 0));
                    }

                    // Connect to the server or proxy server.
                    // TODO: use the cancelable ConnectAsync with 5.0
                    await Socket.ConnectAsync(_proxy?.Address ?? _addr).WaitAsync(cancel).ConfigureAwait(false);

                    _desc = Network.SocketToString(Socket, _proxy, _addr);

                    if (_proxy != null)
                    {
                        await _proxy.ConnectAsync(Socket, _addr, cancel).ConfigureAwait(false);
                    }
                }
                catch (SocketException) when (cancel.IsCancellationRequested)
                {
                    throw new OperationCanceledException(cancel);
                }
                catch (SocketException ex) when (ex.SocketErrorCode == SocketError.ConnectionRefused)
                {
                    throw new ConnectionRefusedException(ex);
                }
                catch (SocketException ex)
                {
                    throw new ConnectFailedException(ex);
                }
            }
        }

        public ValueTask<ArraySegment<byte>> ReceiveDatagramAsync(CancellationToken cancel) =>
            throw new InvalidOperationException("only supported by datagram transports");

        public async ValueTask<int> ReceiveAsync(ArraySegment<byte> buffer, CancellationToken cancel)
        {
            if (buffer.Count == 0)
            {
                throw new ArgumentException($"empty {nameof(buffer)}");
            }

            int received;
            try
            {
                received = await Socket.ReceiveAsync(buffer, SocketFlags.None, cancel).ConfigureAwait(false);
            }
            catch (SocketException) when (cancel.IsCancellationRequested)
            {
                throw new OperationCanceledException(cancel);
            }
            catch (SocketException ex) when (ex.IsConnectionLost())
            {
                throw new ConnectionLostException(ex);
            }
            catch (SocketException ex)
            {
                throw new TransportException(ex);
            }
            if (received == 0)
            {
                throw new ConnectionLostException();
            }
            return received;
        }

        public async ValueTask<int> SendAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel)
        {
            try
            {
                // TODO: Use cancellable API once https://github.com/dotnet/runtime/issues/33417 is fixed.
                return await Socket.SendAsync(buffer, SocketFlags.None).WaitAsync(cancel).ConfigureAwait(false);
            }
            catch (SocketException) when (cancel.IsCancellationRequested)
            {
                throw new OperationCanceledException(cancel);
            }
            catch (SocketException ex) when (ex.IsConnectionLost())
            {
                throw new ConnectionLostException(ex);
            }
            catch (SocketException ex)
            {
                throw new TransportException(ex);
            }
        }

        public override string ToString() => _desc;

        public string ToDetailedString() => _desc;

        internal TcpTransceiver(Communicator communicator, EndPoint addr, INetworkProxy? proxy, IPAddress? sourceAddr)
        {
            _communicator = communicator;
            _proxy = proxy;
            _addr = addr;
            _desc = "";
            _sourceAddr = sourceAddr;
            Socket = Network.CreateSocket(false, (_proxy != null ? _proxy.Address : _addr).AddressFamily);
            try
            {
                Network.SetBufSize(Socket, _communicator, Transport.TCP);
            }
            catch (Exception)
            {
                Socket.CloseNoThrow();
                throw;
            }
        }

        internal TcpTransceiver(Communicator communicator, Socket fd)
        {
            _communicator = communicator;
            Socket = fd;
            try
            {
                Network.SetBufSize(Socket, _communicator, Transport.TCP);
                _desc = Network.SocketToString(Socket);
            }
            catch (Exception)
            {
                Socket.CloseNoThrow();
                throw;
            }
        }
    }
}
