//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace ZeroC.Ice
{
    public interface IEndpointFactory
    {
        EndpointType Type { get; }
        string Transport { get; }
        Endpoint Create(string endpointString, Dictionary<string, string?> options, bool oaEndpoint);
        Endpoint Read(InputStream s);
    }
}
