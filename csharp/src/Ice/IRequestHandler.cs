//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    public interface ICancellationHandler
    {
        void asyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex);
    }

    public interface IRequestHandler : ICancellationHandler
    {
        IRequestHandler? update(IRequestHandler previousHandler, IRequestHandler? newHandler);

        int sendAsyncRequest(ProxyOutgoingAsyncBase @out);

        Reference getReference();

        Ice.ConnectionI getConnection();
    }
}
