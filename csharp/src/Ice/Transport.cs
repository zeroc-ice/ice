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
        /// <summary>TCP transport.</summary>
        TCP = 1,
        /// <summary>SSL transport.</summary>
        SSL = 2,
        /// <summary>UDP transport.</summary>
        UDP = 3,
        /// <summary>Web Socket transport.</summary>
        WS = 4,
        /// <summary>Secure Web Socket transport.</summary>
        WSS = 5,
        /// <summary>Bluetooth transport.</summary>
        BT = 6,
        /// <summary>Secure Bluetooth transport.</summary>
        BTS = 7,
        /// <summary>Apple iAP transport.</summary>
        iAP = 8,
        /// <summary>Secure Apple iAP transport.</summary>
        iAPS = 9,
        /// <summary>Colocated transport.</summary>
        Colocated = 10
    }
}
