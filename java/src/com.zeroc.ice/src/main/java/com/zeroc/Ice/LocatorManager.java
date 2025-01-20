// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

final class LocatorManager {
    private static final class LocatorKey implements Cloneable {
        @Override
        public boolean equals(java.lang.Object o) {
            assert (o instanceof LocatorKey);
            LocatorKey k = (LocatorKey) o;
            if (!k._id.equals(_id)) {
                return false;
            }
            if (!k._encoding.equals(_encoding)) {
                return false;
            }
            return true;
        }

        @Override
        public int hashCode() {
            int h = 5381;
            h = HashUtil.hashAdd(h, _id);
            h = HashUtil.hashAdd(h, _encoding);
            return h;
        }

        @Override
        public LocatorKey clone() {
            LocatorKey c = null;
            try {
                c = (LocatorKey) super.clone();
            } catch (CloneNotSupportedException ex) {
                assert false; // impossible
            }
            return c;
        }

        LocatorKey set(LocatorPrx locator) {
            Reference r = ((_ObjectPrxI) locator)._getReference();
            _id = r.getIdentity();
            _encoding = r.getEncoding();
            return this;
        }

        private Identity _id;
        private EncodingVersion _encoding;
    }

    LocatorManager(Properties properties) {
        _background = properties.getIcePropertyAsInt("Ice.BackgroundLocatorCacheUpdates") > 0;
    }

    synchronized void destroy() {
        for (LocatorInfo info : _table.values()) {
            info.destroy();
        }
        _table.clear();
        _locatorTables.clear();
    }

    //
    // Returns locator info for a given locator. Automatically creates
    // the locator info if it doesn't exist yet.
    //
    public LocatorInfo get(LocatorPrx loc) {
        if (loc == null) {
            return null;
        }

        //
        // The locator can't be located.
        //
        LocatorPrx locator = LocatorPrx.uncheckedCast(loc.ice_locator(null));

        //
        // TODO: reap unused locator info objects?
        //

        synchronized (this) {
            LocatorInfo info = _table.get(locator);
            if (info == null) {
                //
                // Rely on locator identity for the adapter table. We want to
                // have only one table per locator (not one per locator
                // proxy).
                //
                LocatorTable table = _locatorTables.get(_lookupKey.set(locator));
                if (table == null) {
                    table = new LocatorTable();
                    _locatorTables.put(_lookupKey.clone(), table);
                }

                info = new LocatorInfo(locator, table, _background);
                _table.put(locator, info);
            }

            return info;
        }
    }

    private final boolean _background;

    private final java.util.HashMap<LocatorPrx, LocatorInfo> _table = new java.util.HashMap<>();
    private final java.util.HashMap<LocatorKey, LocatorTable> _locatorTables =
            new java.util.HashMap<>();
    private final LocatorKey _lookupKey = new LocatorKey(); // A key used for the lookup
}
