// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System;
    using System.Diagnostics;
    using System.Net;
    using System.Net.Sockets;

    sealed class TcpConnector : Connector
    {
        public Transceiver connect()
        {
            if(_instance.traceLevel() >= 2)
            {
                string s = "trying to establish " + _instance.protocol() + " connection to " + ToString();
                _instance.logger().trace(_instance.traceCategory(), s);
            }

            try
            {
#if SILVERLIGHT
                Socket fd = Network.createSocket(false, _addr.AddressFamily == AddressFamily.InterNetworkV6 ?
                                                 AddressFamily.InterNetworkV6 : AddressFamily.InterNetwork);
#else
                Socket fd = Network.createSocket(false, _addr.AddressFamily);
                Network.setBlock(fd, false);
#endif
#if !COMPACT
                Network.setTcpBufSize(fd, _instance.properties(), _instance.logger());
#endif

                //
                // Nonblocking connect is handled by the transceiver.
                //
                return new TcpTransceiver(_instance, fd, _addr, _proxy, _sourceAddr, false);
            }
            catch(Ice.LocalException ex)
            {
                if(_instance.traceLevel() >= 2)
                {
                    string s = "failed to establish " + _instance.protocol() + " connection to " + ToString() + "\n" +
                        ex;
                    _instance.logger().trace(_instance.traceCategory(), s);
                }
                throw;
            }
        }

        public short type()
        {
            return _instance.type();
        }

        //
        // Only for use by TcpEndpoint
        //
        internal TcpConnector(ProtocolInstance instance, EndPoint addr, NetworkProxy proxy, EndPoint sourceAddr,
                              int timeout, string connectionId)
        {
            _instance = instance;
            _addr = addr;
            _proxy = proxy;
            _sourceAddr = sourceAddr;
            _timeout = timeout;
            _connectionId = connectionId;

            _hashCode = 5381;
            IceInternal.HashUtil.hashAdd(ref _hashCode, _addr);
            if(_sourceAddr != null)
            {
                IceInternal.HashUtil.hashAdd(ref _hashCode, _sourceAddr);
            }
            IceInternal.HashUtil.hashAdd(ref _hashCode, _timeout);
            IceInternal.HashUtil.hashAdd(ref _hashCode, _connectionId);
        }

        public override bool Equals(object obj)
        {
            if(!(obj is TcpConnector))
            {
                return false;
            }

            if(this == obj)
            {
                return true;
            }

            TcpConnector p = (TcpConnector)obj;
            if(_timeout != p._timeout)
            {
                return false;
            }

            if(!Network.addressEquals(_sourceAddr, p._sourceAddr))
            {
                return false;
            }

            if(!_connectionId.Equals(p._connectionId))
            {
                return false;
            }

            return _addr.Equals(p._addr);
        }

        public override string ToString()
        {
            return Network.addrToString(_proxy == null ? _addr : _proxy.getAddress());
        }

        public override int GetHashCode()
        {
            return _hashCode;
        }

        private ProtocolInstance _instance;
        private EndPoint _addr;
        private NetworkProxy _proxy;
        private EndPoint _sourceAddr;
        private int _timeout;
        private string _connectionId;
        private int _hashCode;
    }
}
