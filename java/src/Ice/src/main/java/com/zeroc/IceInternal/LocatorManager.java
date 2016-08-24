// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public final class LocatorManager
{
    static private final class LocatorKey implements Cloneable
    {
        @Override
        public boolean
        equals(Object o)
        {
            assert(o instanceof LocatorKey);
            LocatorKey k = (LocatorKey)o;
            if(!k._id.equals(_id))
            {
                return false;
            }
            if(!k._encoding.equals(_encoding))
            {
                return false;
            }
            return true;
        }

        @Override
        public int
        hashCode()
        {
            int h = 5381;
            h = HashUtil.hashAdd(h, _id);
            h = HashUtil.hashAdd(h, _encoding);
            return h;
        }

        @Override
        public LocatorKey
        clone()
        {
            LocatorKey c = null;
            try
            {
                c = (LocatorKey)super.clone();
            }
            catch(CloneNotSupportedException ex)
            {
                    assert false; // impossible
            }
            return c;
        }

        LocatorKey set(com.zeroc.Ice.LocatorPrx locator)
        {
            Reference r = ((com.zeroc.Ice._ObjectPrxI)locator).__reference();
            _id = r.getIdentity();
            _encoding = r.getEncoding();
            return this;
        }

        private com.zeroc.Ice.Identity _id;
        private com.zeroc.Ice.EncodingVersion _encoding;
    }

    LocatorManager(com.zeroc.Ice.Properties properties)
    {
        _background = properties.getPropertyAsInt("Ice.BackgroundLocatorCacheUpdates") > 0;
    }

    synchronized void
    destroy()
    {
        for(LocatorInfo info : _table.values())
        {
            info.destroy();
        }
        _table.clear();
        _locatorTables.clear();
    }

    //
    // Returns locator info for a given locator. Automatically creates
    // the locator info if it doesn't exist yet.
    //
    public LocatorInfo
    get(com.zeroc.Ice.LocatorPrx loc)
    {
        if(loc == null)
        {
            return null;
        }

        //
        // The locator can't be located.
        //
        com.zeroc.Ice.LocatorPrx locator = com.zeroc.Ice.LocatorPrx.uncheckedCast(loc.ice_locator(null));

        //
        // TODO: reap unused locator info objects?
        //

        synchronized(this)
        {
            LocatorInfo info = _table.get(locator);
            if(info == null)
            {
                //
                // Rely on locator identity for the adapter table. We want to
                // have only one table per locator (not one per locator
                // proxy).
                //
                LocatorTable table = _locatorTables.get(_lookupKey.set(locator));
                if(table == null)
                {
                    table = new LocatorTable();
                    _locatorTables.put(_lookupKey.clone(), table);
                }

                info = new LocatorInfo(locator, table, _background);
                _table.put(locator, info);
            }

            return info;
        }
    }

    final private boolean _background;

    private java.util.HashMap<com.zeroc.Ice.LocatorPrx, LocatorInfo> _table = new java.util.HashMap<>();
    private java.util.HashMap<LocatorKey, LocatorTable> _locatorTables = new java.util.HashMap<>();
    private LocatorKey _lookupKey = new LocatorKey(); // A key used for the lookup
}
