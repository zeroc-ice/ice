// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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

    sealed class UdpConnector : Connector, IComparable
    {
        public Transceiver connect()
        {
            return new UdpTransceiver(instance_, _addr, _mcastInterface, _mcastTtl);
        }

        public short type()
        {
            return UdpEndpointI.TYPE;
        }

        public int CompareTo(object obj)
        {
            UdpConnector p = null;

            try
            {
                p = (UdpConnector)obj;
            }
            catch(InvalidCastException)
            {
                try
                {
                    Connector e = (Connector)obj;
                    return type() < e.type() ? -1 : 1;
                }
                catch(InvalidCastException)
                {
                    Debug.Assert(false);
                }
            }

            if(this == p)
            {
                return 0;
            }

            if(!_connectionId.Equals(p._connectionId))
            {
                return _connectionId.CompareTo(p._connectionId);
            }

            if(_protocolMajor < p._protocolMajor)
            {
                return -1;
            }
            else if(p._protocolMajor < _protocolMajor)
            {
                return 1;
            }

            if(_protocolMinor < p._protocolMinor)
            {
                return -1;
            }
            else if(p._protocolMinor < _protocolMinor)
            {
                return 1;
            }

            if(_encodingMajor < p._encodingMajor)
            {
                return -1;
            }
            else if(p._encodingMajor < _encodingMajor)
            {
                return 1;
            }

            if(_encodingMinor < p._encodingMinor)
            {
                return -1;
            }
            else if(p._encodingMinor < _encodingMinor)
            {
                return 1;
            }

            int rc = _mcastInterface.CompareTo(p._mcastInterface);
            if(rc != 0)
            {
                return rc;
            }

            if(_mcastTtl < p._mcastTtl)
            {
                return -1;
            }
            else if(p._mcastTtl < _mcastTtl)
            {
                return 1;
            }

            return Network.compareAddress(_addr, p._addr);
        }

        //
        // Only for use by TcpEndpoint
        //
        internal UdpConnector(Instance instance, IPEndPoint addr, string mcastInterface, int mcastTtl, 
                              byte protocolMajor, byte protocolMinor, byte encodingMajor, byte encodingMinor,
                              string connectionId)
        {
            instance_ = instance;
            _addr = addr;
            _mcastInterface = mcastInterface;
            _mcastTtl = mcastTtl;
            _protocolMajor = protocolMajor;
            _protocolMinor = protocolMinor;
            _encodingMajor = encodingMajor;
            _encodingMinor = encodingMinor;
            _connectionId = connectionId;

            _hashCode = _addr.GetHashCode();
            _hashCode = 5 * _hashCode + _mcastInterface.GetHashCode();
            _hashCode = 5 * _hashCode + _mcastTtl.GetHashCode();
            _hashCode = 5 * _hashCode + _connectionId.GetHashCode();
        }

        public override bool Equals(object obj)
        {
            return CompareTo(obj) == 0;
        }

        public override string ToString()
        {
            return Network.addrToString(_addr);
        }

        public override int GetHashCode()
        {
            return _hashCode;
        }

        private Instance instance_;
        private IPEndPoint _addr;
        private string _mcastInterface;
        private int _mcastTtl;
        private byte _protocolMajor;
        private byte _protocolMinor;
        private byte _encodingMajor;
        private byte _encodingMinor;
        private string _connectionId;
        private int _hashCode;
    }
}
