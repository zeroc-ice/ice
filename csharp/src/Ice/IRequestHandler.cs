//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    public interface ICancellationHandler
    {
        void AsyncRequestCanceled(OutgoingAsyncBase outAsync, System.Exception ex);
    }

    public interface IRequestHandler : ICancellationHandler
    {
        IRequestHandler? Update(IRequestHandler previousHandler, IRequestHandler? newHandler);

        void SendAsyncRequest(ProxyOutgoingAsyncBase @out);

        Connection? GetConnection();
    }
}
