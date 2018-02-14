// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;

namespace IceInternal
{
#if !SILVERLIGHT
    [Serializable]
#endif
    public abstract class CollectionBase<T> : System.Collections.IList
    {
        protected List<T> list_;

        public CollectionBase()
        {
            list_ = new List<T>();
        }

        public CollectionBase(int capacity)
        {
            list_ = new List<T>(capacity);
        }

        public CollectionBase(T[] a)
        {
            if(a == null)
            {
                throw new ArgumentNullException("a", "Cannot construct collection from null array");
            }

            list_ = new List<T>(a.Length);
            list_.AddRange(a);
        }

        public CollectionBase(IEnumerable<T> l)
        {
            if(l == null)
            {
                throw new ArgumentNullException("l", "Cannot construct collection from null collection");
            }

            list_ = new List<T>();
            list_.AddRange(l);
        }

        public static implicit operator List<T>(CollectionBase<T> l)
        {
            return l.list_;
        }


        public void CopyTo(T[] a__)
        {
            list_.CopyTo(a__);
        }

        public void CopyTo(T[] a__, int i__)
        {
            list_.CopyTo(a__, i__);
        }

        public void CopyTo(int i__, T[] a__, int ai__, int _c_)
        {
            list_.CopyTo(i__, a__, ai__, _c_);
        }

        public T[] ToArray()
        {
            return list_.ToArray();
        }

        public virtual void TrimToSize()
        {
            list_.TrimExcess();
        }

        public virtual void Sort()
        {
            list_.Sort();
        }

        public virtual void Sort(System.Collections.IComparer comparer)
        {
            list_.Sort(new Comparer(comparer));
        }

        public virtual void Sort(int index, int count, System.Collections.IComparer comparer)
        {
            list_.Sort(index, count, new Comparer(comparer));
        }

        public virtual void Reverse()
        {
            list_.Reverse();
        }

        public virtual void Reverse(int index, int count)
        {
            list_.Reverse(index, count);
        }

        public virtual int BinarySearch(T value)
        {
            return list_.BinarySearch(value);
        }

        public virtual int BinarySearch(T value, System.Collections.IComparer comparer)
        {
            return list_.BinarySearch(value, new Comparer(comparer));
        }

        public virtual int BinarySearch(int index, int count, T value, System.Collections.IComparer comparer)
        {
            return list_.BinarySearch(index, count, value, new Comparer(comparer));
        }

        public virtual void InsertRange(int index, CollectionBase<T> c)
        {
            list_.InsertRange(index, c.list_);
        }

        public virtual void InsertRange(int index, T[] c)
        {
            list_.InsertRange(index, c);
        }

        public virtual void RemoveRange(int index, int count)
        {
            list_.RemoveRange(index, count);
        }

        public virtual void SetRange(int index, CollectionBase<T> c)
        {
            if(c == null)
            {
                throw new ArgumentNullException("c", "Collection must not be null for SetRange()");
            }
            if(index < 0 || index + c.Count > list_.Count)
            {
                throw new ArgumentOutOfRangeException("index", "Index out of range");
            }
            for(int i = index; i < list_.Count; ++i)
            {
                list_[i] = c[i - index];
            }
        }

        public virtual void SetRange(int index, T[] c)
        {
            if(c == null)
            {
                throw new ArgumentNullException("c", "Collection must not be null for SetRange()");
            }
            if(index < 0 || index + c.Length > list_.Count)
            {
                throw new ArgumentOutOfRangeException("index", "Index out of range");
            }
            for(int i = index; i < list_.Count; ++i)
            {
                list_[i] = c[i - index];
            }
        }

        public virtual int LastIndexOf(T value)
        {
            return list_.LastIndexOf(value);
        }

        public virtual int LastIndexOf(T value, int startIndex)
        {
            return list_.LastIndexOf(value, startIndex);
        }

        public virtual int LastIndexOf(T value, int startIndex, int count)
        {
            return list_.LastIndexOf(value, startIndex, count);
        }

        public void AddRange(CollectionBase<T> s__)
        {
            list_.AddRange(s__.list_);
        }

        public void AddRange(T[] a__)
        {
            list_.AddRange(a__);
        }

        public int Capacity
        {
            get
            {
                return list_.Capacity;
            }

            set
            {
                list_.Capacity = value;
            }
        }

        public int Count
        {
            get
            {
                return list_.Count;
            }
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return list_.GetEnumerator();
        }

        public IEnumerator<T> GetEnumerator()
        {
            return list_.GetEnumerator();
        }

        public void RemoveAt(int index)
        {
            list_.RemoveAt(index);
        }

        public int Add(T value)
        {
            list_.Add(value);
            return list_.Count - 1;
        }

        public void Clear()
        {
            list_.Clear();
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

        public int IndexOf(T value)
        {
            return list_.IndexOf(value);
        }

        public void Insert(int index, T value)
        {
            list_.Insert(index, value);
        }

        public void Remove(T value)
        {
            list_.Remove(value);
        }

        public bool Contains(T value)
        {
            return list_.Contains(value);
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

        public T this[int index]
        {
            get
            {
                return list_[index];
            }
            set
            {
                list_[index] = value;
            }
        }

        public override int GetHashCode()
        {
            int h = 5381;
            for(int i = 0; i < Count; ++i)
            {
                T v__ = list_[i];
                IceInternal.HashUtil.hashAdd(ref h, v__);
            }
            return h;
        }

        public override bool Equals(object other)
        {
            if(object.ReferenceEquals(this, other))
            {
                return true;
            }
            try
            {
                CollectionBase<T> c = (CollectionBase<T>)other;
                if(list_.Count != c.list_.Count)
                {
                    return false;
                }
                if(list_.Count == 0)
                {
                    return true;
                }
                for(int i = 0; i < list_.Count; ++i)
                {
                   if(!Equals(list_[i], c.list_[i]))
                   {
                       return false;
                   }
                }
            }
            catch(System.Exception)
            {
                return false;
            }

            return true;
        }

        public static bool operator==(CollectionBase<T> lhs__, CollectionBase<T> rhs__)
        {
            return Equals(lhs__, rhs__);
        }

        public static bool operator!=(CollectionBase<T> lhs__, CollectionBase<T> rhs__)
        {
            return !Equals(lhs__, rhs__);
        }

        private class Comparer : IComparer<T>
        {
            private System.Collections.IComparer _c;

            public Comparer(System.Collections.IComparer c)
            {
                _c = c;
            }

            public virtual int Compare(T l, T r)
            {
                return _c.Compare(l, r);
            }
        }

        public int Add(object o)
        {
            checkType(o);
            return Add((T)o);
        }

        public bool Contains(object o)
        {
            checkType(o);
            return Contains((T)o);
        }

        public int IndexOf(object o)
        {
            checkType(o);
            return IndexOf((T)o);
        }

        public void Insert(int i, object o)
        {
            checkType(o);
            Insert(i, (T)o);
        }

        public void Remove(object o)
        {
            checkType(o);
            Remove((T)o);
        }

        object System.Collections.IList.this[int index]
        {
            get
            {
                return this[index];
            }
            set
            {
                checkType(value);
                this[index] = (T)value;
            }
        }

        public void CopyTo(Array a, int index)
        {
            Type t = a.GetType().GetElementType();
            if(!t.IsAssignableFrom(typeof(T)))
            {
                throw new ArgumentException("a__", "Cannot assign " + typeof(T).ToString() + " to array of "
                                            + t.ToString());
            }
            CopyTo((T[])a, index);
        }

        private void checkType(object o)
        {

            if(o != null && !(o is T))
            {
                throw new ArgumentException("Cannot use an object of type " + o.GetType().ToString()
                                            + " with a collection of " + typeof(T).ToString());
            }
        }
    }

}

namespace Ice
{
    [Obsolete("This class is deprecated.")]
    public abstract class CollectionBase<T> : IceInternal.CollectionBase<T>
    {
    }
}
