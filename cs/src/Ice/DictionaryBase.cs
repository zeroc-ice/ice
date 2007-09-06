// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;

namespace Ice
{
    public abstract class DictionaryBase<KT, VT>
    {
        protected Dictionary<KT, VT> dict_;

        public DictionaryBase()
        {
            dict_ = new Dictionary<KT, VT>();
        }

        /* TODO
        public static implicit operator Dictionary<T>(DictionaryBase<T> l)
        {
            return l.dict_;
        }
        */

        public int Count
        {
            get
            {
                return dict_.Count;
            }
        }

        public void Clear()
        {
            dict_.Clear();
        }

        public void CopyTo(System.Array a__, int index)
        {
            if(a__ == null)
            {
                throw new ArgumentNullException("a__", "Cannot copy to null array");
            }
            if(index < 0)
            {
                throw new ArgumentException("index", "Array index cannot be less than zero");
            }
            if(index >= a__.Length)
            {
                throw new ArgumentException("Array index must less than array length");
            }
            if(dict_.Count > a__.Length - index)
            {
                throw new ArgumentException("Insufficient room in target array beyond index");
            }
            if(a__.Rank > 1)
            {
                throw new ArgumentException("a__", "Cannot copy to multidimensional array");
            }
            Type t = a__.GetType().GetElementType();
            if(!t.IsAssignableFrom(typeof(System.Collections.DictionaryEntry)))
            {
                throw new ArgumentException("a__", "Cannot assign DictionaryEntry to target array");
            }

            IEnumerator<KeyValuePair<KT, VT>> e = dict_.GetEnumerator();
            while(e.MoveNext())
            {
                 a__.SetValue(new System.Collections.DictionaryEntry(e.Current.Key, e.Current.Value), index++);
            }
        }

        public override bool Equals(object other)
        {
            if(object.ReferenceEquals(this, other))
            {
                return true;
            }
            if(other == null)
            {
                return false;
            }

            try
            {
                DictionaryBase<KT, VT> d2 = (DictionaryBase<KT, VT>)other;

                if(dict_.Count != d2.dict_.Count)
                {
                    return false;
                }
                if(Count == 0)
                {
                    return true;
                }

                //
                // Compare both sets of keys. Keys are unique and non-null.
                //
                Dictionary<KT, VT>.KeyCollection keys1 = dict_.Keys;
                Dictionary<KT, VT>.KeyCollection keys2 = d2.dict_.Keys;
                KT[] ka1 = new KT[dict_.Count];
                KT[] ka2 = new KT[d2.dict_.Count];
                keys1.CopyTo(ka1, 0);
                keys2.CopyTo(ka2, 0);
                Array.Sort(ka1);
                Array.Sort(ka2);

                for(int i = 0; i < ka1.Length; ++i)
                {
                    if(!Equals(ka1[i], ka2[i]))
                    {
                        return false;
                    }
                }

                //
                // Compare values.
                //
                foreach(KT k1 in ka1)
                {
                    if(!Equals(dict_[k1], d2.dict_[k1]))
                    {
                        return false;
                    }
                }

                return true;
            }
            catch(System.Exception)
            {
                return false;
            }
        }

        public static bool operator==(DictionaryBase<KT, VT> lhs__, DictionaryBase<KT, VT> rhs__)
        {
            return Equals(lhs__, rhs__);
        }

        public static bool operator!=(DictionaryBase<KT, VT> lhs__, DictionaryBase<KT, VT> rhs__)
        {
            return !Equals(lhs__, rhs__);
        }

        public override int GetHashCode()
        {
            int hash = 0;
            foreach(KeyValuePair<KT, VT> kvp in dict_)
            {
                hash = 5 * hash + kvp.Key.GetHashCode();
                hash = 5 * hash + kvp.Value.GetHashCode();
            }
            return hash;
        }

        public class CEnumerator : System.Collections.IDictionaryEnumerator
        {
            public CEnumerator(IEnumerator<KeyValuePair<KT, VT>> e)
            {
                _e = e;
            }

            public bool MoveNext()
            {
                return _e.MoveNext();
            }

            public object Current
            {
                get
                {
                    return new System.Collections.DictionaryEntry(_e.Current.Key, _e.Current.Value);
                }
            }

            public System.Collections.DictionaryEntry Entry
            {
                get
                {
                    return new System.Collections.DictionaryEntry(_e.Current.Key, _e.Current.Value);
                }
            }

            public object Key
            {
                get
                {
                    return _e.Current.Key;
                }
            }

            public object Value
            {
                get
                {
                    return _e.Current.Value;
                }
            }

            public void Reset()
            {
                _e.Reset();
            }

            private IEnumerator<KeyValuePair<KT, VT>> _e;
        }

        public System.Collections.IEnumerator GetEnumerator()
        {
            return new CEnumerator(dict_.GetEnumerator());
        }

        public bool IsFixedSize
        {
            get
            {
                return false;
            }
        }

        public bool IsReadOnly
        {
            get
            {
                return false;
            }
        }

        public bool IsSynchronized
        {
            get
            {
                return false;
            }
        }

        public object SyncRoot
        {
            get
            {
                return this;
            }
        }

        public VT this[KT key]
        {
            get
            {
                return dict_[key];
            }
            set
            {
                dict_[key] = value;
            }
        }

        public System.Collections.ICollection Keys
        {
            get
            {
                return dict_.Keys;
            }
        }

        public System.Collections.ICollection Values
        {
            get
            {
                return dict_.Values;
            }
        }

        public void Add(KT key, VT value)
        {
            try
            {
                dict_.Add(key, value);
            }
            catch(ArgumentException)
            {
                // Ignore.
            }
        }

        public void Remove(KT key)
        {
            dict_.Remove(key);
        }

        public bool Contains(KT key)
        {
            return dict_.ContainsKey(key);
        }
    }

}
