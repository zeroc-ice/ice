//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    public interface ICancellationHandler
    {
        void AsyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex);
    }

    public interface IRequestHandler : ICancellationHandler
    {
        IRequestHandler? Update(IRequestHandler previousHandler, IRequestHandler? newHandler);

        int SendAsyncRequest(ProxyOutgoingAsyncBase @out);

        Reference GetReference();

        Ice.Connection? GetConnection();
    }
}
