//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    public class ConnectionRequestHandler : IRequestHandler
    {
        public IRequestHandler? Update(IRequestHandler previousHandler, IRequestHandler? newHandler)
        {
            try
            {
                if (previousHandler == this)
                {
                    return newHandler;
                }
                else if (previousHandler.GetConnection() == _connection)
                {
                    //
                    // If both request handlers point to the same connection, we also
                    // update the request handler. See bug ICE-5489 for reasons why
                    // this can be useful.
                    //
                    return newHandler;
                }
            }
            catch (System.Exception)
            {
                // Ignore
            }
            return this;
        }

        public int SendAsyncRequest(ProxyOutgoingAsyncBase outAsync) =>
            outAsync.InvokeRemote(_connection, _compress, !outAsync.IsOneway);

        public void AsyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex) =>
            _connection.AsyncRequestCanceled(outAsync, ex);

        public Reference GetReference() => _reference;

        public Ice.Connection GetConnection() => _connection;

        public ConnectionRequestHandler(Reference @ref, Ice.Connection connection, bool compress)
        {
            _reference = @ref;
            _connection = connection;
            _compress = compress;
        }

        private readonly Reference _reference;

        private readonly Ice.Connection _connection;
        private readonly bool _compress;
    }
}
