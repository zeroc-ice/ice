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
    using System.Diagnostics;

    class WSAcceptor : Acceptor
    {
        public void close()
        {
            _delegate.close();
        }

        public EndpointI listen(EndpointI endp)
        {
            Debug.Assert(endp is WSEndpoint);
            WSEndpoint p = (WSEndpoint)endp;
            EndpointI endpoint = _delegate.listen(p.getDelegate());
            return endp.endpoint(this);
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

        internal WSAcceptor(ProtocolInstance instance, Acceptor del)
        {
            _instance = instance;
            _delegate = del;
        }

        private ProtocolInstance _instance;
        private Acceptor _delegate;
    }
}
