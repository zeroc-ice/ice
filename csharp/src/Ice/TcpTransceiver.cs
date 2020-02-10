//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.Net;
using System.Net.Sockets;

namespace IceInternal
{
    internal sealed class TcpTransceiver : ITransceiver
    {
        public Socket? Fd() => _stream.Fd();

        public int Initialize(Buffer readBuffer, Buffer writeBuffer, ref bool hasMoreData) =>
            _stream.Connect(readBuffer, writeBuffer, ref hasMoreData);

        // If we are initiating the connection closure, wait for the peer
        // to close the TCP/IP connection. Otherwise, close immediately.
        public int Closing(bool initiator, Ice.LocalException? ex) =>
            initiator ? SocketOperation.Read : SocketOperation.None;

        public void Close() => _stream.Close();

        public Endpoint Bind()
        {
            Debug.Assert(false);
            throw new System.InvalidOperationException();
        }

        public void Destroy() => _stream.Destroy();

        public int Write(Buffer buf) => _stream.Write(buf);

        public int Read(Buffer buf, ref bool hasMoreData) => _stream.Read(buf);

        public bool StartRead(Buffer buf, AsyncCallback callback, object state) =>
            _stream.StartRead(buf, callback, state);

        public void FinishRead(Buffer buf) => _stream.FinishRead(buf);

        public bool StartWrite(Buffer buf, AsyncCallback callback, object state, out bool completed) =>
            _stream.StartWrite(buf, callback, state, out completed);

        public void FinishWrite(Buffer buf) => _stream.FinishWrite(buf);

        public string Protocol() => _instance.Protocol;

        public Ice.ConnectionInfo GetInfo()
        {
            var info = new Ice.TCPConnectionInfo();
            Socket? fd = _stream.Fd();
            if (fd != null)
            {
                EndPoint localEndpoint = Network.GetLocalAddress(fd);
                info.LocalAddress = Network.EndpointAddressToString(localEndpoint);
                info.LocalPort = Network.EndpointPort(localEndpoint);
                EndPoint? remoteEndpoint = Network.GetRemoteAddress(fd);
                info.RemoteAddress = Network.EndpointAddressToString(remoteEndpoint);
                info.RemotePort = Network.EndpointPort(remoteEndpoint);
                info.RcvSize = Network.GetRecvBufferSize(fd);
                info.SndSize = Network.GetSendBufferSize(fd);
            }
            return info;
        }

        public void CheckSendSize(Buffer buf)
        {
        }

        public void SetBufferSize(int rcvSize, int sndSize) => _stream.SetBufferSize(rcvSize, sndSize);

        public override string ToString() => _stream.ToString();

        public string ToDetailedString() => ToString();

        //
        // Only for use by TcpConnector, TcpAcceptor
        //
        internal TcpTransceiver(ProtocolInstance instance, StreamSocket stream)
        {
            _instance = instance;
            _stream = stream;
        }

        private readonly ProtocolInstance _instance;
        private readonly StreamSocket _stream;
    }
}
