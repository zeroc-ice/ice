//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    public class ConnectionRequestHandler : IRequestHandler
    {
        public IRequestHandler? update(IRequestHandler previousHandler, IRequestHandler? newHandler)
        {
            try
            {
                if (previousHandler == this)
                {
                    return newHandler;
                }
                else if (previousHandler.getConnection() == _connection)
                {
                    //
                    // If both request handlers point to the same connection, we also
                    // update the request handler. See bug ICE-5489 for reasons why
                    // this can be useful.
                    //
                    return newHandler;
                }
            }
            catch (Ice.Exception)
            {
                // Ignore
            }
            return this;
        }

        public int sendAsyncRequest(ProxyOutgoingAsyncBase outAsync)
        {
            return outAsync.invokeRemote(_connection, _compress, _response);
        }

        public void AsyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex)
        {
            _connection.AsyncRequestCanceled(outAsync, ex);
        }

        public Reference getReference()
        {
            return _reference;
        }

        public Ice.Connection getConnection()
        {
            return _connection;
        }

        public ConnectionRequestHandler(Reference @ref, Ice.Connection connection, bool compress)
        {
            _reference = @ref;
            _response = _reference.getMode() == Ice.InvocationMode.Twoway;
            _connection = connection;
            _compress = compress;
        }

        private Reference _reference;
        private bool _response;
        private Ice.Connection _connection;
        private bool _compress;
    }
}
