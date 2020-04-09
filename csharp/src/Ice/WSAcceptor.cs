//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;

namespace IceInternal
{
    internal class WSAcceptor : IAcceptor
    {
        public void Close() => _delegate.Close();

        public Endpoint Listen()
        {
            _endpoint = _endpoint.GetEndpoint(_delegate.Listen());
            return _endpoint;
        }

        public bool StartAccept(AsyncCallback callback, object state) => _delegate.StartAccept(callback, state);

        public void FinishAccept() => _delegate.FinishAccept();

        public ITransceiver Accept() => new WSTransceiver(_instance, _delegate.Accept());

        public string Transport() => _delegate.Transport();

        public override string ToString() => _delegate.ToString();

        public string ToDetailedString() => _delegate.ToDetailedString();

        public IAcceptor GetDelegate() => _delegate;

        internal WSAcceptor(WSEndpoint endpoint, TransportInstance instance, IAcceptor del)
        {
            _endpoint = endpoint;
            _instance = instance;
            _delegate = del;
        }

        private WSEndpoint _endpoint;
        private readonly TransportInstance _instance;
        private readonly IAcceptor _delegate;
    }
}
