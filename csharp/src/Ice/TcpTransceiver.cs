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
        private readonly EndPoint? _addr;
        private readonly Communicator _communicator;
        private string _desc;
        private readonly Socket _fd;
        private readonly INetworkProxy? _proxy;
        private readonly IPAddress? _sourceAddr;

        public void CheckSendSize(int size)
        {
        }

        public ValueTask ClosingAsync(Exception ex, CancellationToken cancel) => new ValueTask();

        public ValueTask DisposeAsync()
        {
            _fd.Dispose();
            return new ValueTask();
        }

        public Socket? Fd() => _fd;

        public async ValueTask InitializeAsync(CancellationToken cancel)
        {
            if (_addr != null)
            {
                try
                {
                    // Bind the socket to the source address if one is set.
                    if (_sourceAddr != null)
                    {
                        _fd.Bind(new IPEndPoint(_sourceAddr, 0));
                    }

                    // Connect to the server or proxy server.
                    // TODO: use the cancelable ConnectAsync with 5.0
                    await _fd.ConnectAsync(_proxy?.GetAddress() ?? _addr).WaitAsync(cancel).ConfigureAwait(false);

                    _desc = Network.FdToString(_fd);

                    if (_proxy != null)
                    {
                        await _proxy.ConnectAsync(_fd, _addr, cancel).ConfigureAwait(false);
                    }
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

        public ValueTask<ArraySegment<byte>> ReadAsync(CancellationToken cancel) => throw new InvalidOperationException();

        public async ValueTask<int> ReadAsync(ArraySegment<byte> buffer, CancellationToken cancel)
        {
            int received;
            try
            {
                received = await _fd.ReceiveAsync(buffer, SocketFlags.None, cancel).ConfigureAwait(false);
            }
            catch (SocketException ex) when (Network.ConnectionLost(ex))
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

        public override string ToString() => _desc;

        public string ToDetailedString() => _desc;

        public async ValueTask<int> WriteAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel)
        {
            try
            {
                // TODO: Use cancellable API once https://github.com/dotnet/runtime/issues/33417 is fixed.
                return await _fd.SendAsync(buffer, SocketFlags.None).WaitAsync(cancel).ConfigureAwait(false);
            }
            catch (SocketException ex) when (Network.ConnectionLost(ex))
            {
                throw new ConnectionLostException(ex);
            }
            catch (SocketException ex)
            {
                throw new TransportException(ex);
            }
        }

        internal TcpTransceiver(Communicator communicator, EndPoint addr, INetworkProxy? proxy, IPAddress? sourceAddr)
        {
            _communicator = communicator;
            _proxy = proxy;
            _addr = addr;
            _desc = "";
            _sourceAddr = sourceAddr;
            _fd = Network.CreateSocket(false, (_proxy != null ? _proxy.GetAddress() : _addr).AddressFamily);
            try
            {
                Network.SetBufSize(_fd, _communicator, Transport.TCP);
            }
            catch (Exception)
            {
                Network.CloseSocketNoThrow(_fd);
                throw;
            }
        }

        internal TcpTransceiver(Communicator communicator, Socket fd)
        {
            _communicator = communicator;
            _fd = fd;
            try
            {
                Network.SetBufSize(_fd, _communicator, Transport.TCP);
                _desc = Network.FdToString(_fd);
            }
            catch (Exception)
            {
                Network.CloseSocketNoThrow(_fd);
                throw;
            }
        }
    }
}
