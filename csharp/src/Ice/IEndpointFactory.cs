//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace ZeroC.Ice
{
    public interface IEndpointFactory
    {
        /// <summary>The <see cref="EndpointType">type</see> of this endpoint.</summary>
        EndpointType Type { get; }
        /// <summary>The name of the endpoint's transport in lowercase.</summary>
        string Name { get; }
        Endpoint Create(string endpointString, Dictionary<string, string?> options, bool oaEndpoint);
        /// <summary>Read and endpoint from the given input stream.</summary>
        /// <param name="istr">The input stream to read from.</param>
        /// <returns>Returns the endpoint read from the stream</returns>
        Endpoint Read(InputStream istr);
    }
}
