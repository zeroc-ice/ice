// Copyright (c) ZeroC, Inc.

/** APIs to communicate with the Glacier2 service. */
// Suppress module warnings: (module name component glacier2 should avoid terminal digits)
@SuppressWarnings("module")
module com.zeroc.glacier2 {
    exports com.zeroc.Glacier2;

    requires transitive com.zeroc.ice;
}
