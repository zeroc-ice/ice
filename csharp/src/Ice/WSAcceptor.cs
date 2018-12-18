// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    class WSAcceptor : Acceptor
    {
        public void close()
        {
            _delegate.close();
        }

        public EndpointI listen()
        {
            _endpoint = _endpoint.endpoint(_delegate.listen());
            return _endpoint;
        }

        public bool startAccept(AsyncCallback callback, object state)
        {
            return _delegate.startAccept(callback, state);
        }

        public void finishAccept()
        {
            _delegate.finishAccept();
        }

        public Transceiver accept()
        {
            return new WSTransceiver(_instance, _delegate.accept());
        }

        public string protocol()
        {
            return _delegate.protocol();
        }

        public override string ToString()
        {
            return _delegate.ToString();
        }

        public string toDetailedString()
        {
            return _delegate.toDetailedString();
        }

        public Acceptor getDelegate()
        {
            return _delegate;
        }

        internal WSAcceptor(WSEndpoint endpoint, ProtocolInstance instance, Acceptor del)
        {
            _endpoint = endpoint;
            _instance = instance;
            _delegate = del;
        }

        private WSEndpoint _endpoint;
        private ProtocolInstance _instance;
        private Acceptor _delegate;
    }
}
