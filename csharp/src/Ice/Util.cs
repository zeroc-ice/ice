//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    /// <summary>
    /// Utility methods for the Ice run time.
    /// </summary>
    public static class Util
    {
        public static ITransportPluginFacade GetTransportPluginFacade(Communicator communicator) =>
            new TransportPluginFacade(communicator);
    }
}
