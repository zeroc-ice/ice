// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Evictor
{
    using System.Collections.Generic;

    public abstract class EvictorBase : Ice.ServantLocator
    {
        public EvictorBase()
        {
            _size = 1000;
        }

        public EvictorBase(int size)
        {
            _size = size < 0 ? 1000 : size;
        }

        protected abstract Ice.Object add(Ice.Current c, out object cookie);

        protected abstract void evict(Ice.Object servant, object cookie);

        public Ice.Object locate(Ice.Current c, out object cookie)
        {
            lock(this)
            {
                //
                // Check if we a servant in the map already.
                //
                EvictorEntry entry = _map[c.id];
                if(entry != null)
                {
                    //
                    // Got an entry already, dequeue the entry from
                    // its current position.
                    //
                    entry.queuePos.Remove();
                }
                else
                {
                    //
                    // We do not have an entry. Ask the derived class to
                    // instantiate a servant and add a new entry to the map.
                    //
                    entry = new EvictorEntry();
                    entry.servant = add(c, out entry.userCookie); // Down-call
                    if(entry.servant == null)
                    {
                        cookie = null;
                        return null;
                    }
                    entry.useCount = 0;
                    _map[c.id] = entry;
                }

                //
                // Increment the use count of the servant and enqueue
                // the entry at the front, so we get LRU order.
                //
                ++(entry.useCount);
                _queue.AddFirst(c.id);
                entry.queuePos = (LinkedList<Ice.Identity>.Enumerator)_queue.GetEnumerator();
                entry.queuePos.MoveNext();

                cookie = entry;

                return entry.servant;
            }
        }

        public void finished(Ice.Current c, Ice.Object o, object cookie)
        {
            lock(this)
            {
                EvictorEntry entry = (EvictorEntry)cookie;

                //
                // Decrement use count and check if
                // there is something to evict.
                //
                --(entry.useCount);
                evictServants();
            }
        }

        public void deactivate(string category)
        {
            lock(this)
            {
                _size = 0;
                evictServants();
            }
        }

        private class EvictorEntry
        {
            internal Ice.Object servant;
            internal object userCookie;
            internal LinkedList<Ice.Identity>.Enumerator queuePos;
            internal int useCount;
        }

        private void evictServants()
        {
            //
            // If the evictor queue has grown larger than the limit,
            // look at the excess elements to see whether any of them
            // can be evicted.
            //
            LinkedList<Ice.Identity>.Enumerator p = (LinkedList<Ice.Identity>.Enumerator)_queue.GetEnumerator();
            int excessEntries = _map.Count - _size;
            for(int i = 0; i < excessEntries; ++i)
            {
                p.MovePrev();
                Ice.Identity id = p.Current;
                EvictorEntry e = _map[id];
                if(e.useCount == 0)
                {
                    evict(e.servant, e.userCookie); // Down-call
                    p.Remove();
                    _map.Remove(id);
                }
            }
        }

        private Dictionary<Ice.Identity, EvictorEntry> _map = new Dictionary<Ice.Identity, EvictorEntry>();
        private LinkedList<Ice.Identity> _queue = new LinkedList<Ice.Identity>();
        private int _size;
    }
}
