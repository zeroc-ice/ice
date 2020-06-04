//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Background
{
    internal class Acceptor : IAcceptor
    {
        public void Close() => _acceptor.Close();

        public ZeroC.Ice.Endpoint Listen()
        {
            _endpoint = _endpoint.GetEndpoint(_acceptor.Listen());
            return _endpoint;
        }

        public bool StartAccept(AsyncCallback callback, object state) => _acceptor.StartAccept(callback, state);

        public void FinishAccept() => _acceptor.FinishAccept();

        public ITransceiver Accept()
        {
            return new Transceiver(_acceptor.Accept());
        }

        public string Transport => _acceptor.Transport;

        public override string ToString() => _acceptor.ToString();

        public string ToDetailedString() => _acceptor.ToDetailedString();

        internal Acceptor(Endpoint endpoint, IAcceptor acceptor)
        {
            _endpoint = endpoint;
            _acceptor = acceptor;
        }

        private Endpoint _endpoint;
        private IAcceptor _acceptor;
    }
}
