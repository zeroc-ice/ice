//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    public interface IRequestHandler
    {
        ValueTask<Task<IncomingResponseFrame>?> SendRequestAsync(OutgoingRequestFrame frame, bool oneway,
            bool synchronous, IInvocationObserver? observer);

        Connection? GetConnection();
    }
}
