// Copyright (c) ZeroC, Inc.

/**
 * APIs for clients of the Glacier2 firewall traversal service.
 */
// Suppress module warnings: (module name component glacier2 should avoid terminal digits)
@SuppressWarnings("module")
module com.zeroc.glacier2 {
    exports com.zeroc.Glacier2;

    requires transitive com.zeroc.ice;
}
