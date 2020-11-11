// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Diagnostics;
using System.Net.Sockets;

namespace ZeroC.Ice
{
    internal static class ExceptionExtensions
    {
        internal static RetryPolicy GetRetryPolicy(this Exception exception, bool isIdempotent, bool sent)
        {
            RetryPolicy retryPolicy = RetryPolicy.NoRetry;
            if (exception is TransportException transportException)
            {
                // Apply transport exception retry policy if the request is idempotent, the request was not sent or
                // the connection was gracefully closed by the peer.
                var closedException = exception as ConnectionClosedException;
                if ((closedException?.IsClosedByPeer ?? false) || isIdempotent || !sent)
                {
                    retryPolicy = transportException.RetryPolicy;
                }
            }
            return retryPolicy;
        }

        /// <summary>This method tries to distinguish connection loss error conditions from other error conditions.
        /// It's a bit tedious since it's difficult to have an exhaustive list of errors that match this condition.
        /// An alternative would be to change the transports to always throw ConnectionLostException on failure to
        /// receive or send data.</summary>
        internal static bool IsConnectionLost(this Exception ex)
        {
            Debug.Assert(!(ex is TransportException));

            // Check the inner exceptions if the given exception isn't a socket exception. Streams wrapping a socket
            // typically throw an IOException with the SocketException as the InnerException.
            while (!(ex is SocketException || ex is System.ComponentModel.Win32Exception) &&
                   ex.InnerException != null)
            {
                ex = ex.InnerException;
            }

            if (ex is SocketException socketException)
            {
                SocketError error = socketException.SocketErrorCode;
                return error == SocketError.ConnectionReset ||
                       error == SocketError.Shutdown ||
                       error == SocketError.ConnectionAborted ||
                       error == SocketError.NetworkDown ||
                       error == SocketError.NetworkReset;
            }
            else if (ex is System.ComponentModel.Win32Exception)
            {
                // "Authentication failed because the remote party has closed the transport stream"
                // "An authentication error has occurred"
                return ex.HResult == -2146232800 || ex.HResult == -2147467259;
            }
            return false;
        }
    }
}
