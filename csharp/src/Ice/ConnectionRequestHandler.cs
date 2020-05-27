//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    internal class ConnectionRequestHandler : IRequestHandler
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

        public void SendAsyncRequest(ProxyOutgoingAsyncBase outAsync) =>
            outAsync.InvokeRemote(_connection, _compress, !outAsync.IsOneway);

        public void AsyncRequestCanceled(OutgoingAsyncBase outAsync, System.Exception ex) =>
            _connection.AsyncRequestCanceled(outAsync, ex);

        public Connection GetConnection() => _connection;

        public ConnectionRequestHandler(Connection connection, bool compress)
        {
            _connection = connection;
            _compress = compress;
        }

        private readonly Connection _connection;
        private readonly bool _compress;
    }
}
