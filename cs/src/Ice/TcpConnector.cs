// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System.Diagnostics;
    using System.Net;
    using System.Net.Sockets;

    sealed class TcpConnector : Connector, System.IComparable
    {
        internal const short TYPE = 1;

        public Transceiver connect(int timeout)
        {
            if(_traceLevels.network >= 2)
            {
                string s = "trying to establish tcp connection to " + ToString();
                _logger.trace(_traceLevels.networkCat, s);
            }
            
            Socket fd = Network.createSocket(false);
            Network.setBlock(fd, false);
            Network.setTcpBufSize(fd, instance_.initializationData().properties, _logger);
            Network.doConnect(fd, _addr, timeout);
            
            if(_traceLevels.network >= 1)
            {
                string s = "tcp connection established\n" + Network.fdToString(fd);
                _logger.trace(_traceLevels.networkCat, s);
            }
            
            return new TcpTransceiver(instance_, fd);
        }

        public short type()
        {
            return TYPE;
        }
        
        public override string ToString()
        {
            return Network.addrToString(_addr);
        }

        //
        // Only for use by TcpEndpoint
        //
        internal TcpConnector(Instance instance, IPEndPoint addr, int timeout, string connectionId)
        {
            instance_ = instance;
            _traceLevels = instance.traceLevels();
            _logger = instance.initializationData().logger;
            _addr = addr;
            _timeout = timeout;
            _connectionId = connectionId;

            _hashCode = _addr.GetHashCode();
            _hashCode = 5 * _hashCode + _timeout;
            _hashCode = 5 * _hashCode + _connectionId.GetHashCode();
        }

        public override int GetHashCode()
        {
            return _hashCode;
        }

        //
        // Compare endpoints for sorting purposes
        //
        public override bool Equals(object obj)
        {
            return CompareTo(obj) == 0;
        }

        public int CompareTo(object obj)
        {
            TcpConnector p = null;

            try
            {
                p = (TcpConnector)obj;
            }
            catch(System.InvalidCastException)
            {
                try
                {
                    Connector e = (Connector)obj;
                    return type() < e.type() ? -1 : 1;
                }
                catch(System.InvalidCastException)
                {
                    Debug.Assert(false);
                }
            }

            if(this == p)
            {
                return 0;
            }

            if(_timeout < p._timeout)
            {
                return -1;
            }
            else if(p._timeout < _timeout)
            {
                return 1;
            }

            if(!_connectionId.Equals(p._connectionId))
            {
                return _connectionId.CompareTo(p._connectionId);
            }

            return Network.compareAddress(_addr, p._addr);
        }
        
        private Instance instance_;
        private TraceLevels _traceLevels;
        private Ice.Logger _logger;
        private IPEndPoint _addr;
        private int _timeout;
        private string _connectionId;
        private int _hashCode;
    }
}
