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
        public Socket? Fd() => _stream.fd();

        public int Initialize(Buffer readBuffer, Buffer writeBuffer, ref bool hasMoreData) =>
            _stream.connect(readBuffer, writeBuffer, ref hasMoreData);

        // If we are initiating the connection closure, wait for the peer
        // to close the TCP/IP connection. Otherwise, close immediately.
        public int Closing(bool initiator, Ice.LocalException? ex) =>
            initiator ? SocketOperation.Read : SocketOperation.None;

        public void Close() => _stream.close();

        public Endpoint Bind()
        {
            Debug.Assert(false);
            throw new System.InvalidOperationException();
        }

        public void Destroy() => _stream.destroy();

        public int Write(Buffer buf) => _stream.write(buf);

        public int Read(Buffer buf, ref bool hasMoreData) => _stream.read(buf);

        public bool StartRead(Buffer buf, AsyncCallback callback, object state) =>
            _stream.startRead(buf, callback, state);

        public void FinishRead(Buffer buf) => _stream.finishRead(buf);

        public bool StartWrite(Buffer buf, AsyncCallback callback, object state, out bool completed) =>
            _stream.startWrite(buf, callback, state, out completed);

        public void FinishWrite(Buffer buf) => _stream.finishWrite(buf);

        public string Protocol() => _instance.Protocol;

        public Ice.ConnectionInfo GetInfo()
        {
            var info = new Ice.TCPConnectionInfo();
            Socket? fd = _stream.fd();
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

        public void SetBufferSize(int rcvSize, int sndSize) => _stream.setBufferSize(rcvSize, sndSize);

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
