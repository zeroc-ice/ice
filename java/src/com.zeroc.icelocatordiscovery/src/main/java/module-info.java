// Copyright (c) ZeroC, Inc.

/**
 * APIs for the IceLocatorDiscovery plug-in. IceLocatorDiscovery discovers Ice locators (such as
 * IceGrid registries) using UDP multicast.
 */
module com.zeroc.icelocatordiscovery {
    exports com.zeroc.IceLocatorDiscovery;

    requires transitive com.zeroc.ice;
}
