// Copyright (c) ZeroC, Inc.

/**
 * IceStorm APIs. IceStorm is a broker-based pub/sub service, where subscribers connect to
 * publishers using topics.
 */
module com.zeroc.icestorm {
    exports com.zeroc.IceStorm;

    requires transitive com.zeroc.ice;
}
