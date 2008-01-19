namespace Evictor
{
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

        protected abstract Ice.Object add(Ice.Current c, out System.Object cookie);

        protected abstract void evict(Ice.Object servant, out System.Object xcookie);

        public Ice.Object locate(Ice.Current c, out System.Object cookie)
        {
            lock(this)
            {
                //
                // Check if we a servant in the map already.
                //
                EvictorEntry entry = (EvictorEntry)_map[c.id];
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
                entry.queuePos = (LinkedList.Enumerator)_queue.GetEnumerator();
                entry.queuePos.MoveNext();

                cookie = entry;

                return entry.servant;
            }
        }

        public void finished(Ice.Current c, Ice.Object o, System.Object cookie)
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
            internal System.Object userCookie;
            internal LinkedList.Enumerator queuePos;
            internal int useCount;
        }

        private void evictServants()
        {
            //
            // If the evictor queue has grown larger than the limit,
            // look at the excess elements to see whether any of them
            // can be evicted.
            //
            LinkedList.Enumerator p = (LinkedList.Enumerator)_queue.GetEnumerator();
            int excessEntries = _map.Count - _size;
            for(int i = 0; i < excessEntries; ++i)
            {
                p.MovePrev();
                Ice.Identity id = (Ice.Identity)p.Current;
                EvictorEntry e = (EvictorEntry)_map[id];
                if(e.useCount == 0)
                {
                    evict(e.servant, e.userCookie); // Down-call
                    p.Remove();
                    _map.Remove(id);
                }
            }
        }

        private System.Collections.Hashtable _map = new System.Collections.Hashtable();
        private LinkedList _queue = new LinkedList();
        private int _size;
    }
}
