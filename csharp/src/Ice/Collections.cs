//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections;
using System.Collections.Generic;

namespace Ice
{
    public static class Collections
    {
        public static bool Equals<Key, AnyClass>(Dictionary<Key, AnyClass>? lhs, Dictionary<Key, AnyClass>? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs == null || rhs == null || lhs.Count != rhs.Count)
            {
                return false;
            }

            EqualityComparer<AnyClass> comparer = EqualityComparer<AnyClass>.Default;
            foreach (KeyValuePair<Key, AnyClass> entry in lhs)
            {
                if (!rhs.TryGetValue(entry.Key, out AnyClass value) || !comparer.Equals(entry.Value, value))
                {
                    return false;
                }
            }
            return true;
        }

        public static bool Equals<T>(T[]? arr1, T[]? arr2)
        {
            if (ReferenceEquals(arr1, arr2))
            {
                return true;
            }

            if (arr1 == null || arr2 == null || arr1.Length != arr2.Length)
            {
                return false;
            }

            EqualityComparer<T> comparer = EqualityComparer<T>.Default;
            for (int i = 0; i < arr1.Length; ++i)
            {
                if (!comparer.Equals(arr1[i], arr2[i]))
                {
                    return false;
                }
            }
            return true;
        }

        public static bool Equals<T>(ICollection<T>? lhs, ICollection<T>? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs == null || rhs == null || lhs.Count != rhs.Count)
            {
                return false;
            }

            EqualityComparer<T> comparer = EqualityComparer<T>.Default;
            IEnumerator<T> i = lhs.GetEnumerator();
            IEnumerator<T> j = rhs.GetEnumerator();
            while (i.MoveNext())
            {
                if (!j.MoveNext() || !comparer.Equals(i.Current, j.Current))
                {
                    return false;
                }
            }
            return true;
        }

        public static bool Equals<T>(this IEnumerable<T>? lhs, IEnumerable<T>? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs == null || rhs == null)
            {
                return false;
            }

            EqualityComparer<T> comparer = EqualityComparer<T>.Default;
            IEnumerator<T> i = lhs.GetEnumerator();
            IEnumerator<T> j = rhs.GetEnumerator();
            while (i.MoveNext())
            {
                if (!j.MoveNext() || !comparer.Equals(i.Current, j.Current))
                {
                    return false;
                }
            }
            return !j.MoveNext();
        }

        public static int GetHashCode<Key, AnyClass>(Dictionary<Key, AnyClass> d)
        {
            int h = 5381;
            foreach (KeyValuePair<Key, AnyClass> e in d)
            {
                IceInternal.HashUtil.HashAdd(ref h, e.Key);
                IceInternal.HashUtil.HashAdd(ref h, e.Value);
            }
            return h;
        }

        public static int GetHashCode<T>(this T[]? arr)
        {
            int h = 5381;
            if (arr != null)
            {
                for (int i = 0; i < arr.Length; i++)
                {
                    IceInternal.HashUtil.HashAdd(ref h, arr[i]);
                }
            }
            return h;
        }

        public static int GetHashCode(IEnumerable seq)
        {
            int h = 5381;
            IEnumerator e = seq.GetEnumerator();
            while (e.MoveNext())
            {
                IceInternal.HashUtil.HashAdd(ref h, e.Current);
            }
            return h;
        }
    }
}
