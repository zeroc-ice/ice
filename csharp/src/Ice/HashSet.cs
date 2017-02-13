// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#if COMPACT

//
// System.Collections.Generic.HashSet is not available in the .NET Compact Framework.
// This class is a minimal implementation that provides only the methods required by
// Ice internals.
//
using System;
using System.Collections.Generic;

namespace IceInternal
{
    public class HashSet<T> : ICollection<T>
    {
        public HashSet()
        {
            entries_ = new Dictionary<T, bool>();
        }

        public HashSet(int capacity)
        {
            entries_ = new Dictionary<T, bool>(capacity);
        }

        void ICollection<T>.Add(T item)
        {
            try
            {
                entries_.Add(item, false);
            }
            catch(ArgumentException)
            {
                // Item already present.
            }
        }

        public bool Add(T item)
        {
            try
            {
                entries_.Add(item, false);
            }
            catch(ArgumentException)
            {
                return false; // Item already present.
            }
            return true;
        }

        public void Clear()
        {
            entries_.Clear();
        }

        public bool Contains(T item)
        {
            return entries_.ContainsKey(item);
        }

        public void CopyTo(T[] a, int idx)
        {
            entries_.Keys.CopyTo(a, idx);
        }

        public void CopyTo(T[] a)
        {
            entries_.Keys.CopyTo(a, 0);
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return entries_.Keys.GetEnumerator();
        }

        public IEnumerator<T> GetEnumerator()
        {
            return entries_.Keys.GetEnumerator();
        }

        public bool Remove(T item)
        {
            return entries_.Remove(item);
        }

        public int Count
        {
            get
            {
                return entries_.Count;
            }
        }

        public bool IsReadOnly
        {
            get
            {
                return false;
            }
        }

        private Dictionary<T, bool> entries_;
    }
}

#endif
