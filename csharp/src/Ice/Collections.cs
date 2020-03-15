//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    internal static class Collections
    {
        public static bool Equals<TKey, TValue>(IReadOnlyDictionary<TKey, TValue>? lhs,
            IReadOnlyDictionary<TKey, TValue>? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs == null || rhs == null || lhs.Count != rhs.Count)
            {
                return false;
            }

            EqualityComparer<TValue> comparer = EqualityComparer<TValue>.Default;
            foreach (KeyValuePair<TKey, TValue> entry in lhs)
            {
                if (!rhs.TryGetValue(entry.Key, out TValue value) || !comparer.Equals(entry.Value, value))
                {
                    return false;
                }
            }
            return true;
        }

        public static int GetHashCode<TKey, TValue>(IReadOnlyDictionary<TKey, TValue> d)
        {
            var hash = new System.HashCode();
            foreach (KeyValuePair<TKey, TValue> e in d)
            {
                hash.Add(e.Key);
                hash.Add(e.Value);
            }
            return hash.ToHashCode();
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

        public static int GetHashCode<T>(this T[]? arr)
        {
            var hash = new System.HashCode();
            if (arr != null)
            {
                for (int i = 0; i < arr.Length; i++)
                {
                    hash.Add(arr[i]);
                }
            }
            return hash.ToHashCode();
        }
    }
}
