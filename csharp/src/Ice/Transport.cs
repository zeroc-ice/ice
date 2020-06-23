//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    /// <summary>Identifies a transport protocol that Ice can use to send requests and receive responses. The
    /// enumerators of Transport correspond to the transports that the Ice runtime knows and implements. Other
    /// transports, with short values not represented by these enumerators, can be implemented and registered using
    /// transport plug-ins.</summary>
    public enum Transport : short
    {
        TCP = 1,
        SSL = 2,
        UDP = 3,
        WS = 4,
        WSS = 5,
        BT = 6,
        BTS = 7,
#pragma warning disable SA1300 // Element should begin with upper-case letter
        iAP = 8,
        iAPS = 9
#pragma warning restore SA1300 // Element should begin with upper-case letter
    }
}
