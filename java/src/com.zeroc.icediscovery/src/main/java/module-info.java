// Copyright (c) ZeroC, Inc.

/**
 * APIs for the IceDiscovery plug-in. IceDiscovery lets client applications discover servers using
 * UDP multicast.
 */
module com.zeroc.icediscovery {
    exports com.zeroc.IceDiscovery;

    requires transitive com.zeroc.ice;
}
