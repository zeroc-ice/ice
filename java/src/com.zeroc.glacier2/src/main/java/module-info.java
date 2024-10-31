// Copyright (c) ZeroC, Inc.

// Suppress module warnings: (module name component glacier2 should avoid terminal digits)
@SuppressWarnings("module")
module com.zeroc.glacier2 {
    exports com.zeroc.Glacier2;
    exports com.zeroc.Glacier2.IceMX;

    requires transitive com.zeroc.ice;
}
