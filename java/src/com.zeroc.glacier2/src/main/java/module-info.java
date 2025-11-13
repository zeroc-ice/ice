// Copyright (c) ZeroC, Inc.

/** Client APIs to communicate through firewalls and across NATs with Glacier2. */
// Suppress module warnings: (module name component glacier2 should avoid terminal digits)
@SuppressWarnings("module")
module com.zeroc.glacier2 {
    exports com.zeroc.Glacier2;

    requires transitive com.zeroc.ice;
}
