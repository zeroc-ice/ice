//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;

namespace IceInternal
{
    public interface IConnector
    {
        //
        // Create a transceiver without blocking. The transceiver may not be fully connected
        // until its initialize method is called.
        //
        ITransceiver Connect();

        EndpointType Type();
    }
}
