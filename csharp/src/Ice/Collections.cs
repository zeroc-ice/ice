//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    internal static class Collections
    {
        public static bool Equals<TKey, TValue>(IReadOnlyDictionary<TKey, TValue>? lhs,
                                                IReadOnlyDictionary<TKey, TValue>? rhs) where TKey : notnull
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

        public static int GetHashCode<TKey, TValue>(IReadOnlyDictionary<TKey, TValue> d) where TKey : notnull
        {
            var hash = new System.HashCode();
            foreach (KeyValuePair<TKey, TValue> e in d)
            {
                hash.Add(e.Key);
                hash.Add(e.Value);
            }
            return hash.ToHashCode();
        }
    }
}
