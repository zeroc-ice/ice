//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Test
{
    public static class DictionaryExtensions
    {
        /// <summary>Compares two dictionaries for equality. Enumerable.SequenceEqual provides the equivalent
        /// functionality for sequences.</summary>
        /// <param name="lhs">The first dictionary to compare.</param>
        /// <param name="rhs">The second dictionary to compare.</param>
        /// <returns>True if the two dictionaries have the exact same entries using the value's default equality
        /// comparison; otherwise, false.</returns>
        public static bool DictionaryEqual<TKey, TValue>(this IReadOnlyDictionary<TKey, TValue>? lhs,
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
    }
}
