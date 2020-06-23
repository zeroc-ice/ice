//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    public interface IConnector
    {
        /// <summary>Creates a transceiver without blocking. The transceiver may not be fully connected until its
        /// Initialize method is called.</summary>
        ITransceiver Connect();
    }
}
