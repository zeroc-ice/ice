//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Test
{
    public static class Collections
    {
        public static bool Equals<Key, Tvalue>(Dictionary<Key, Tvalue>? lhs, Dictionary<Key, Tvalue>? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs == null || rhs == null || lhs.Count != rhs.Count)
            {
                return false;
            }

            EqualityComparer<Tvalue> comparer = EqualityComparer<Tvalue>.Default;
            foreach (KeyValuePair<Key, Tvalue> entry in lhs)
            {
                if (!rhs.TryGetValue(entry.Key, out Tvalue value) || !comparer.Equals(entry.Value, value))
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
    }
}
