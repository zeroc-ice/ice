//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections;
using System.Collections.Generic;

namespace Ice
{
    public static class Collections
    {
        public static bool Equals<Key, Value>(Dictionary<Key, Value>? lhs, Dictionary<Key, Value>? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs == null || rhs == null || lhs.Count != rhs.Count)
            {
                return false;
            }

            var comparer = EqualityComparer<Value>.Default;
            foreach (var entry in lhs)
            {
                if (!rhs.TryGetValue(entry.Key, out Value value) || !comparer.Equals(entry.Value, value))
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

            var comparer = EqualityComparer<T>.Default;
            for (var i = 0; i < arr1.Length; ++i)
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

            var comparer = EqualityComparer<T>.Default;
            var i = lhs.GetEnumerator();
            var j = rhs.GetEnumerator();
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

            var comparer = EqualityComparer<T>.Default;
            var i = lhs.GetEnumerator();
            var j = rhs.GetEnumerator();
            while (i.MoveNext())
            {
                if (!j.MoveNext() || !comparer.Equals(i.Current, j.Current))
                {
                    return false;
                }
            }
            return !j.MoveNext();
        }

        public static int GetHashCode<Key, Value>(Dictionary<Key, Value> d)
        {
            int h = 5381;
            foreach (var e in d)
            {
                IceInternal.HashUtil.hashAdd(ref h, e.Key);
                IceInternal.HashUtil.hashAdd(ref h, e.Value);
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
                    IceInternal.HashUtil.hashAdd(ref h, arr[i]);
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
                IceInternal.HashUtil.hashAdd(ref h, e.Current);
            }
            return h;
        }
    }
}
