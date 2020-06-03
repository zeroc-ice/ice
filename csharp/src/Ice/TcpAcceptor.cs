//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace ZeroC.Ice
{
    internal class TcpAcceptor : IAcceptor
    {
        public virtual void Close()
        {
            if (_acceptFd != null)
            {
                Network.CloseSocketNoThrow(_acceptFd);
            }
            if (_fd != null)
            {
                Network.CloseSocketNoThrow(_fd);
            }
        }

        public virtual Endpoint Listen()
        {
            try
            {
                Debug.Assert(_fd != null);
                _addr = Network.DoBind(_fd, _addr);
                Network.DoListen(_fd, _backlog);
            }
            catch (SystemException)
            {
                _fd = null;
                throw;
            }
            _endpoint = (TcpEndpoint)_endpoint.NewPort(EffectivePort());
            return _endpoint;
        }

        public virtual bool StartAccept(AsyncCallback callback, object state)
        {
            try
            {
                Debug.Assert(_fd != null);
                _result = _fd.BeginAccept(delegate(IAsyncResult result)
                                          {
                                              if (!result.CompletedSynchronously)
                                              {
                                                  Debug.Assert(result.AsyncState != null);
                                                  callback(result.AsyncState);
                                              }
                                          }, state);
            }
            catch (SocketException ex)
            {
                throw new TransportException(ex);
            }
            return _result.CompletedSynchronously;
        }

        public virtual void FinishAccept()
        {
            if (_fd != null)
            {
                _acceptFd = null;
                try
                {
                    _acceptFd = _fd.EndAccept(_result);
                }
                catch (SocketException ex)
                {
                    _acceptError = new TransportException(ex);
                }
            }
        }

        public virtual ITransceiver Accept()
        {
            if (_acceptFd == null)
            {
                Debug.Assert(_acceptError != null);
                throw _acceptError;
            }

            Socket acceptFd = _acceptFd;
            _acceptFd = null;
            _acceptError = null;
            return _endpoint.CreateTransceiver(Transport,
                new StreamSocket(_communicator, acceptFd), _adapterName);
        }

        public string Transport { get; }

        public override string ToString() => Network.AddrToString(_addr);

        public string ToDetailedString()
        {
            var s = new StringBuilder("local address = ");
            s.Append(ToString());

            List<string> intfs =
                Network.GetHostsForEndpointExpand(_addr.Address.ToString(), _communicator.IPVersion, true);
            if (intfs.Count != 0)
            {
                s.Append("\nlocal interfaces = ");
                s.Append(string.Join(", ", intfs.ToArray()));
            }
            return s.ToString();
        }

        public int EffectivePort() => _addr.Port;

        internal TcpAcceptor(
            TcpEndpoint endpoint,
            Communicator communicator,
            string transport,
            string host,
            int port,
            string adapterName)
        {
            _adapterName = adapterName;
            _endpoint = endpoint;
            _communicator = communicator;
            Transport = transport;
            _backlog = communicator.GetPropertyAsInt("Ice.TCP.Backlog") ?? 511;

            try
            {
                int ipVersion = _communicator.IPVersion;
                _addr = Network.GetAddressForServerEndpoint(host, port, ipVersion, _communicator.PreferIPv6);
                _fd = Network.CreateServerSocket(false, _addr.AddressFamily, ipVersion);
                Network.SetBlock(_fd, false);
                Network.SetTcpBufSize(_fd, _communicator);
            }
            catch (Exception)
            {
                _fd = null;
                throw;
            }
        }

        private readonly string _adapterName;
        private TcpEndpoint _endpoint;
        private readonly Communicator _communicator;
        private Socket? _fd;
        private Socket? _acceptFd;
        private Exception? _acceptError;
        private readonly int _backlog;
        private IPEndPoint _addr;
        private IAsyncResult? _result;
    }
}
