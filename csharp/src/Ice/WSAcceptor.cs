//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    internal class WSAcceptor : IAcceptor
    {
        public void close() => _delegate.close();

        public Endpoint listen()
        {
            _endpoint = _endpoint.endpoint(_delegate.listen());
            return _endpoint;
        }

        public bool startAccept(AsyncCallback callback, object state) => _delegate.startAccept(callback, state);

        public void finishAccept() => _delegate.finishAccept();

        public ITransceiver accept() => new WSTransceiver(_instance, _delegate.accept());

        public string protocol() => _delegate.protocol();

        public override string ToString() => _delegate.ToString();

        public string toDetailedString() => _delegate.toDetailedString();

        public IAcceptor getDelegate() => _delegate;

        internal WSAcceptor(WSEndpoint endpoint, ProtocolInstance instance, IAcceptor del)
        {
            _endpoint = endpoint;
            _instance = instance;
            _delegate = del;
        }

        private WSEndpoint _endpoint;
        private ProtocolInstance _instance;
        private IAcceptor _delegate;
    }
}
