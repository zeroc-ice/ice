//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    internal class WSAcceptor : IAcceptor
    {
        public void Close() => _delegate.Close();

        public Endpoint Listen()
        {
            _endpoint = (IPEndpoint)_delegate.Listen();
            return _endpoint;
        }

        public bool StartAccept(AsyncCallback callback, object state) => _delegate.StartAccept(callback, state);

        public void FinishAccept() => _delegate.FinishAccept();

        public ITransceiver Accept() => new WSTransceiver(_communicator, _delegate.Accept());

        public string Transport => _delegate.Transport;

        public override string ToString() => _delegate.ToString();

        public string ToDetailedString() => _delegate.ToDetailedString();

        public int EffectivePort() => _delegate.EffectivePort();

        public IAcceptor GetDelegate() => _delegate;

        internal WSAcceptor(WSEndpoint endpoint, Communicator communicator, IAcceptor del)
        {
            _endpoint = endpoint;
            _communicator = communicator;
            _delegate = del;
        }

        internal WSAcceptor(WSSEndpoint endpoint, Communicator communicator, IAcceptor del)
        {
            _endpoint = endpoint;
            _communicator = communicator;
            _delegate = del;
        }

        private IPEndpoint _endpoint;
        private readonly Communicator _communicator;
        private readonly IAcceptor _delegate;
    }
}
