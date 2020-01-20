//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System.Diagnostics;
    using System.Net;
    using System.Net.Sockets;

    internal sealed class TcpTransceiver : ITransceiver
    {
        public Socket? fd() => _stream.fd();

        public int initialize(Buffer readBuffer, Buffer writeBuffer, ref bool hasMoreData) =>
            _stream.connect(readBuffer, writeBuffer, ref hasMoreData);

        // If we are initiating the connection closure, wait for the peer
        // to close the TCP/IP connection. Otherwise, close immediately.
        public int closing(bool initiator, Ice.LocalException? ex) =>
            initiator ? SocketOperation.Read : SocketOperation.None;

        public void close() => _stream.close();

        public Endpoint bind()
        {
            Debug.Assert(false);
            throw new System.InvalidOperationException();
        }

        public void destroy() => _stream.destroy();

        public int write(Buffer buf) => _stream.write(buf);

        public int read(Buffer buf, ref bool hasMoreData) => _stream.read(buf);

        public bool startRead(Buffer buf, AsyncCallback callback, object state) =>
            _stream.startRead(buf, callback, state);

        public void finishRead(Buffer buf) => _stream.finishRead(buf);

        public bool startWrite(Buffer buf, AsyncCallback callback, object state, out bool completed) =>
            _stream.startWrite(buf, callback, state, out completed);

        public void finishWrite(Buffer buf) => _stream.finishWrite(buf);

        public string protocol() => _instance.protocol();

        public Ice.ConnectionInfo getInfo()
        {
            Ice.TCPConnectionInfo info = new Ice.TCPConnectionInfo();
            Socket? fd = _stream.fd();
            if (fd != null)
            {
                EndPoint localEndpoint = Network.getLocalAddress(fd);
                info.LocalAddress = Network.endpointAddressToString(localEndpoint);
                info.LocalPort = Network.endpointPort(localEndpoint);
                EndPoint? remoteEndpoint = Network.getRemoteAddress(fd);
                info.RemoteAddress = Network.endpointAddressToString(remoteEndpoint);
                info.RemotePort = Network.endpointPort(remoteEndpoint);
                info.RcvSize = Network.getRecvBufferSize(fd);
                info.SndSize = Network.getSendBufferSize(fd);
            }
            return info;
        }

        public void checkSendSize(Buffer buf)
        {
        }

        public void setBufferSize(int rcvSize, int sndSize) => _stream.setBufferSize(rcvSize, sndSize);

        public override string ToString() => _stream.ToString();

        public string toDetailedString() => ToString();

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
