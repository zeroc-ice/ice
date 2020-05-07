//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;

namespace IceInternal
{
    public sealed class StreamSocket
    {
        public StreamSocket(TransportInstance instance, INetworkProxy? proxy, EndPoint addr, IPAddress? sourceAddr)
        {
            _instance = instance;
            _proxy = proxy;
            _addr = addr;
            _sourceAddr = sourceAddr;
            _fd = Network.CreateSocket(false, (_proxy != null ? _proxy.GetAddress() : _addr).AddressFamily);
            _state = StateNeedConnect;

            Network.SetBlock(_fd, false);
            Network.SetTcpBufSize(_fd, _instance);

            _readEventArgs = new SocketAsyncEventArgs();
            _readEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(IoCompleted);

            _writeEventArgs = new SocketAsyncEventArgs();
            _writeEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(IoCompleted);

            //
            // For timeouts to work properly, we need to receive/send
            // the data in several chunks. Otherwise, we would only be
            // notified when all the data is received/written. The
            // connection timeout could easily be triggered when
            // receiving/sending large messages.
            //
            _maxSendPacketSize = Math.Max(512, Network.GetSendBufferSize(_fd));
            _maxRecvPacketSize = Math.Max(512, Network.GetRecvBufferSize(_fd));
        }

        public StreamSocket(TransportInstance instance, Socket fd)
        {
            _instance = instance;
            _fd = fd;
            _state = StateConnected;

            try
            {
                _desc = Network.FdToString(_fd);
            }
            catch (Exception)
            {
                Network.CloseSocketNoThrow(_fd);
                throw;
            }

            Network.SetBlock(_fd, false);
            Network.SetTcpBufSize(_fd, _instance);

            _readEventArgs = new SocketAsyncEventArgs();
            _readEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(IoCompleted);

            _writeEventArgs = new SocketAsyncEventArgs();
            _writeEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(IoCompleted);

            //
            // For timeouts to work properly, we need to receive/send
            // the data in several chunks. Otherwise, we would only be
            // notified when all the data is received/written. The
            // connection timeout could easily be triggered when
            // receiving/sending large messages.
            //
            _maxSendPacketSize = Math.Max(512, Network.GetSendBufferSize(_fd));
            _maxRecvPacketSize = Math.Max(512, Network.GetRecvBufferSize(_fd));
        }

        public void SetBlock(bool block)
        {
            Debug.Assert(_fd != null);
            Network.SetBlock(_fd, block);
        }

        public int Connect(ref ArraySegment<byte> readBuffer, IList<ArraySegment<byte>> writeBuffer)
        {
            if (_state == StateNeedConnect)
            {
                _state = StateConnectPending;
                return SocketOperation.Connect;
            }
            else if (_state <= StateConnectPending)
            {
                Debug.Assert(_writeEventArgs != null);
                if (_writeEventArgs.SocketError != SocketError.Success)
                {
                    var ex = new SocketException((int)_writeEventArgs.SocketError);
                    if (Network.ConnectionRefused(ex))
                    {
                        throw new ConnectionRefusedException(ex);
                    }
                    else
                    {
                        throw new ConnectFailedException(ex);
                    }
                }
                Debug.Assert(_fd != null);
                _desc = Network.FdToString(_fd, _proxy, _addr);
                _state = _proxy != null ? StateProxyWrite : StateConnected;
            }

            if (_state == StateProxyWrite)
            {
                Debug.Assert(_proxy != null);
                Debug.Assert(_addr != null);
                _proxy.BeginWrite(_addr, writeBuffer);
                return SocketOperation.Write;
            }
            else if (_state == StateProxyRead)
            {
                Debug.Assert(_proxy != null);
                readBuffer = _proxy.BeginRead();
                return SocketOperation.Read;
            }
            else if (_state == StateProxyConnected)
            {
                Debug.Assert(_proxy != null);
                _proxy.Finish(readBuffer);

                // TODO Return buffers to the pool
                readBuffer = ArraySegment<byte>.Empty;
                writeBuffer.Clear();

                _state = StateConnected;
            }

            Debug.Assert(_state == StateConnected);
            return SocketOperation.None;
        }

        public bool IsConnected() => _state == StateConnected && _fd != null;

        public Socket? Fd() => _fd;

        public int GetRecvPacketSize(int length) => _maxRecvPacketSize > 0 ? Math.Min(length, _maxRecvPacketSize) : length;

        public void SetBufferSize(int rcvSize, int sndSize)
        {
            Debug.Assert(_fd != null);
            Network.SetTcpBufSize(_fd, rcvSize, sndSize, _instance);
        }

        public int Read(ref ArraySegment<byte> buffer, ref int offset)
        {
            if (_state == StateProxyRead)
            {
                Debug.Assert(_proxy != null);
                while (true)
                {
                    int ret = ReadData(buffer, offset);
                    if (ret == 0)
                    {
                        return SocketOperation.Read;
                    }
                    offset += ret;
                    _state = ToState(_proxy.EndRead(ref buffer, offset));
                    if (_state != StateProxyRead)
                    {
                        return SocketOperation.None;
                    }
                }
            }
            offset += ReadData(buffer, offset);
            return offset < buffer.Count ? SocketOperation.Read : SocketOperation.None;
        }

        /// <summary>Write the buffer data to the socket starting at the given offset,
        /// the offset is incremented with the number of bytes written, returns
        /// SocketOperation.None if all the data was wrote otherwise returns
        /// SocketOperation.Write.</summary>
        /// <param name="buffer">The data to write to the socket as a list of byte array segments.</param>
        /// <param name="offset">The zero based byte offset into the buffer. The offset is increased
        /// by the amount of bytes written.</param>
        /// <returns>A constant indicating if all data was wrote to the socket, SocketOperation.None
        /// indicate there is no more data to write, SocketOperation.Write indicates there is still
        /// data to write in the buffer.</returns>
        public int Write(IList<ArraySegment<byte>> buffer, ref int offset)
        {
            int count = buffer.GetByteCount();
            if (_state == StateProxyWrite)
            {
                Debug.Assert(_proxy != null);
                while (true)
                {
                    int ret = WriteData(buffer, offset, count);
                    if (ret == 0)
                    {
                        return SocketOperation.Write;
                    }
                    _state = ToState(_proxy.EndWrite(buffer, ret));
                    if (_state != StateProxyWrite)
                    {
                        offset += ret;
                        return SocketOperation.None;
                    }
                }
            }
            int remaining = count - offset;
            int bytesTransferred = WriteData(buffer, offset, count);
            offset += bytesTransferred;
            return bytesTransferred < remaining ? SocketOperation.Write : SocketOperation.None;
        }

        public bool StartRead(ArraySegment<byte> buffer, int offset, AsyncCallback callback, object state)
        {
            Debug.Assert(_fd != null && _readEventArgs != null);
            int packetSize = GetRecvPacketSize(buffer.Count - offset);
            try
            {
                _readCallback = callback;
                _readEventArgs.UserToken = state;
                _readEventArgs.SetBuffer(buffer.Array, buffer.Offset + offset, packetSize);
                return !_fd.ReceiveAsync(_readEventArgs);
            }
            catch (SocketException ex)
            {
                if (Network.ConnectionLost(ex))
                {
                    throw new ConnectionLostException(ex);
                }
                throw new TransportException(ex);
            }
        }

        public void FinishRead(ref ArraySegment<byte> buffer, ref int offset)
        {
            if (_fd == null) // Transceiver was closed
            {
                return;
            }

            Debug.Assert(_fd != null && _readEventArgs != null);
            try
            {
                if (_readEventArgs.SocketError != SocketError.Success)
                {
                    throw new SocketException((int)_readEventArgs.SocketError);
                }
                int ret = _readEventArgs.BytesTransferred;
                _readEventArgs.SetBuffer(null, 0, 0);
                if (ret == 0)
                {
                    throw new ConnectionLostException();
                }

                Debug.Assert(ret > 0);
                offset += ret;
                if (_state == StateProxyRead)
                {
                    Debug.Assert(_proxy != null);
                    _state = ToState(_proxy.EndRead(ref buffer, offset));
                }
            }
            catch (SocketException ex)
            {
                if (Network.ConnectionLost(ex))
                {
                    throw new ConnectionLostException(ex);
                }
                throw new TransportException(ex);
            }
            catch (ObjectDisposedException ex)
            {
                throw new ConnectionLostException(ex);
            }
        }

        /// <summary>Starts an asynchronous write operation of the buffer data to the socket
        /// starting at the given offset, completed is set to true if the write operation
        /// account for the remaining of the buffer data or false otherwise, returns whenever
        /// the asynchronous operation completed synchronously or not.</summary>
        /// <param name="buffer">The data to write to the socket as a list of byte array segments.</param>
        /// <param name="offset">The zero based byte offset into the buffer at what start writing.</param>
        /// <param name="callback">The asynchronous completion callback.</param>
        /// <param name="state">A state object that is associated with the asynchronous operation.</param>
        /// <param name="completed">True if the write operation accounts for the buffer remaining data, from
        /// offset to the end of the buffer.</param>
        /// <returns>True if the asynchronous operation completed synchronously otherwise false.</returns>
        public bool StartWrite(IList<ArraySegment<byte>> buffer, int offset, AsyncCallback callback, object state,
            out bool completed)
        {
            Debug.Assert(_fd != null && _writeEventArgs != null);
            if (_state == StateConnectPending)
            {
                completed = false;
                _writeCallback = callback;
                try
                {
                    if (_sourceAddr != null)
                    {
                        Network.DoBind(_fd, new IPEndPoint(_sourceAddr, 0));
                    }

                    EndPoint? addr = _proxy != null ? _proxy.GetAddress() : _addr;
                    Debug.Assert(addr != null);

                    _writeEventArgs.RemoteEndPoint = addr;
                    _writeEventArgs.UserToken = state;
                    return !_fd.ConnectAsync(_writeEventArgs);
                }
                catch (Exception ex)
                {
                    throw new TransportException(ex);
                }
            }

            try
            {
                int count = buffer.GetByteCount();
                int remaining = count - offset;
                buffer.FillSegments(offset, _sendSegments, Math.Min(remaining, _maxSendPacketSize));
                _writeCallback = callback;
                _writeEventArgs.UserToken = state;
                _writeEventArgs.BufferList = _sendSegments;
                bool completedSynchronously = !_fd.SendAsync(_writeEventArgs);
                completed = _maxSendPacketSize >= remaining;
                return completedSynchronously;
            }
            catch (SocketException ex)
            {
                if (Network.ConnectionLost(ex))
                {
                    throw new ConnectionLostException(ex);
                }
                throw new Ice.TransportException(ex);
            }
            catch (ObjectDisposedException ex)
            {
                throw new ConnectionLostException(ex);
            }
        }

        /// <summary>Finish an asynchronous write operation, the offset is increase with the
        /// number of bytes actually wrote to the socket.</summary>
        /// <param name="buffer">The buffer of data to write to the socket.</param>
        /// <param name="offset">The offset at what the write operation starts, the offset is increase
        /// with the number of bytes successfully wrote to the socket.</param>
        public void FinishWrite(IList<ArraySegment<byte>> buffer, ref int offset)
        {
            if (_writeEventArgs.BufferList != null)
            {
                _writeEventArgs.BufferList.Clear();
                _writeEventArgs.BufferList = null;
            }

            if (_fd == null) // Transceiver was closed
            {
                int remaining = buffer.GetByteCount() - offset;
                if (remaining <= _maxSendPacketSize)
                {
                    offset += remaining; // Assume all the data was sent for at-most-once semantics.
                }
                return;
            }

            Debug.Assert(_fd != null && _writeEventArgs != null);

            if (_state < StateConnected && _state != StateProxyWrite)
            {
                return;
            }

            try
            {
                if (_writeEventArgs.SocketError != SocketError.Success)
                {
                    throw new SocketException((int)_writeEventArgs.SocketError);
                }
                int ret = _writeEventArgs.BytesTransferred;
                if (ret == 0)
                {
                    throw new ConnectionLostException();
                }
                Debug.Assert(ret > 0);
                if (_state == StateProxyWrite)
                {
                    Debug.Assert(_proxy != null);
                    _state = ToState(_proxy.EndWrite(buffer, ret));
                }
                offset += ret;
            }
            catch (SocketException ex)
            {
                if (Network.ConnectionLost(ex))
                {
                    throw new ConnectionLostException(ex);
                }

                throw new TransportException(ex);
            }
            catch (ObjectDisposedException ex)
            {
                throw new ConnectionLostException(ex);
            }
        }

        public void Close()
        {
            Debug.Assert(_fd != null);
            try
            {
                Network.CloseSocket(_fd);
            }
            finally
            {
                _fd = null;
            }
        }

        public void Destroy()
        {
            Debug.Assert(_readEventArgs != null && _writeEventArgs != null);
            _readEventArgs.Dispose();
            _writeEventArgs.Dispose();
        }

        public override string ToString() => _desc;

        private int ReadData(ArraySegment<byte> buffer, int offset)
        {
            Debug.Assert(_fd != null);
            if (AssemblyUtil.IsMono)
            {
                //
                // Mono on Android and iOS don't support the use of synchronous socket
                // operations on a non-blocking socket. Returning 0 here forces the caller to schedule
                // an asynchronous operation.
                //
                return 0;
            }

            int bytesTransferred = 0;
            int bufferOffset = buffer.Offset + offset;
            while (buffer.Count - (offset + bytesTransferred) > 0)
            {
                try
                {
                    int ret = _fd.Receive(buffer.Array, bufferOffset + bytesTransferred,
                        GetRecvPacketSize(buffer.Count - (offset + bytesTransferred)),
                        SocketFlags.None);
                    if (ret == 0)
                    {
                        throw new ConnectionLostException();
                    }
                    bytesTransferred += ret;
                }
                catch (SocketException ex)
                {
                    if (Network.WouldBlock(ex))
                    {
                        return bytesTransferred;
                    }
                    else if (Network.Interrupted(ex))
                    {
                        continue;
                    }
                    else if (Network.ConnectionLost(ex))
                    {
                        throw new ConnectionLostException(ex);
                    }

                    throw new TransportException(ex);
                }
            }
            return bytesTransferred;
        }

        private int WriteData(IList<ArraySegment<byte>> buffer, int offset, int count)
        {
            Debug.Assert(_fd != null);
            if (AssemblyUtil.IsMono)
            {
                //
                // Mono on Android and iOS don't support the use of synchronous socket
                // operations on a non-blocking socket. Returning 0 here forces the caller to schedule
                // an asynchronous operation.
                //
                return 0;
            }

            int packetSize = _maxSendPacketSize;
            if (AssemblyUtil.IsWindows)
            {
                //
                // On Windows, limiting the buffer size is important to prevent
                // poor throughput performances when transferring large amount of
                // data. See Microsoft KB article KB823764.
                //
                if (_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize / 2)
                {
                    packetSize = _maxSendPacketSize / 2;
                }
            }

            int remaining = count - offset;
            int sent = 0;
            while (remaining - sent > 0)
            {
                try
                {
                    buffer.FillSegments(offset + sent, _sendSegments, Math.Min(remaining - sent, packetSize));
                    int ret = _fd.Send(_sendSegments, SocketFlags.None);
                    _sendSegments.Clear();
                    Debug.Assert(ret > 0);
                    sent += ret;
                }
                catch (SocketException ex)
                {
                    if (Network.WouldBlock(ex))
                    {
                        return sent;
                    }
                    else if (Network.ConnectionLost(ex))
                    {
                        throw new ConnectionLostException(ex);
                    }
                    throw new TransportException(ex);
                }
            }
            return sent;
        }
        private void IoCompleted(object? sender, SocketAsyncEventArgs e)
        {
            switch (e.LastOperation)
            {
                case SocketAsyncOperation.Receive:
                    Debug.Assert(_readCallback != null);
                    _readCallback(e.UserToken);
                    break;
                case SocketAsyncOperation.Send:
                case SocketAsyncOperation.Connect:
                    Debug.Assert(_writeCallback != null);
                    _writeCallback(e.UserToken);
                    break;
                default:
                    throw new ArgumentException("the last operation completed on the socket was not a receive or send",
                        nameof(e.LastOperation));
            }
        }

        private int ToState(int operation)
        {
            return operation switch
            {
                SocketOperation.Read => StateProxyRead,
                SocketOperation.Write => StateProxyWrite,
                _ => StateProxyConnected,
            };
        }

        private readonly TransportInstance _instance;
        private readonly INetworkProxy? _proxy;
        private readonly EndPoint? _addr;
        private readonly IPAddress? _sourceAddr;

        private Socket? _fd;
        private readonly int _maxSendPacketSize;
        private readonly int _maxRecvPacketSize;
        private int _state;
        private string _desc = "";

        private readonly SocketAsyncEventArgs _writeEventArgs;
        private readonly SocketAsyncEventArgs _readEventArgs;

        private AsyncCallback? _writeCallback;
        private AsyncCallback? _readCallback;

        private readonly List<ArraySegment<byte>> _sendSegments = new List<ArraySegment<byte>>();

        private const int StateNeedConnect = 0;
        private const int StateConnectPending = 1;
        private const int StateProxyWrite = 2;
        private const int StateProxyRead = 3;
        private const int StateProxyConnected = 4;
        private const int StateConnected = 5;
    }
}
