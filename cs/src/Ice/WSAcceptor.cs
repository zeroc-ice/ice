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
    class WSAcceptor : IceInternal.Acceptor
    {
        public void close()
        {
            _delegate.close();
        }

        public void listen()
        {
            _delegate.listen();
        }

        public bool startAccept(IceInternal.AsyncCallback callback, object state)
        {
            return _delegate.startAccept(callback, state);
        }

        public void finishAccept()
        {
            _delegate.finishAccept();
        }

        public IceInternal.Transceiver accept()
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

        internal WSAcceptor(ProtocolInstance instance, IceInternal.Acceptor del)
        {
            _instance = instance;
            _delegate = del;
        }

        private ProtocolInstance _instance;
        private IceInternal.Acceptor _delegate;
    }
}
