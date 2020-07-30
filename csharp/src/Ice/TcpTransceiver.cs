//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net.Sockets;

namespace ZeroC.Ice
{
    internal sealed class TcpTransceiver : ITransceiver
    {
        public Socket? Fd() => _stream.Fd();

        public int Initialize(ref ArraySegment<byte> readBuffer, IList<ArraySegment<byte>> writeBuffer) =>
            _stream.Connect(ref readBuffer, writeBuffer);

        // If we are initiating the connection closure, wait for the peer
        // to close the TCP/IP connection. Otherwise, close immediately.
        public int Closing(bool initiator, System.Exception? ex) =>
            initiator ? SocketOperation.Read : SocketOperation.None;

        public void Close() => _stream.Close();

        public Endpoint Bind()
        {
            Debug.Assert(false);
            throw new InvalidOperationException();
        }

        public void Destroy() => _stream.Destroy();

        public int Write(IList<ArraySegment<byte>> buffer, ref int offset) => _stream.Write(buffer, ref offset);

        public int Read(ref ArraySegment<byte> buffer, ref int offset) =>
             _stream.Read(ref buffer, ref offset);

        public bool StartRead(ref ArraySegment<byte> buffer, ref int offset, AsyncCallback callback, object state) =>
            _stream.StartRead(buffer, offset, callback, state);

        public void FinishRead(ref ArraySegment<byte> buffer, ref int offset) =>
            _stream.FinishRead(ref buffer, ref offset);

        public bool
        StartWrite(IList<ArraySegment<byte>> buffer, int offset, AsyncCallback callback, object state, out bool completed) =>
            _stream.StartWrite(buffer, offset, callback, state, out completed);

        public void FinishWrite(IList<ArraySegment<byte>> buffer, ref int offset) =>
            _stream.FinishWrite(buffer, ref offset);

        public void CheckSendSize(int size)
        {
        }

        public void SetBufferSize(int rcvSize, int sndSize) => _stream.SetBufferSize(rcvSize, sndSize);

        public override string ToString() => _stream.ToString();

        public string ToDetailedString() => ToString();

        //
        // Only for use by TcpConnector, TcpAcceptor
        //
        internal TcpTransceiver(StreamSocket stream) => _stream = stream;

        private readonly StreamSocket _stream;
    }
}
