//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    internal static class DictionaryExtensions
    {
        /// <summary>Compares two dictionaries for equality. Enumerable.SequenceEqual provides the equivalent
        /// functionality for sequences.</summary>
        /// <param name="lhs">The first dictionary to compare.</param>
        /// <param name="rhs">The second dictionary to compare.</param>
        /// <returns>True if the two dictionaries have the exact same entries using the value's default equality
        /// comparison; otherwise, false.</returns>
        internal static bool DictionaryEqual<TKey, TValue>(this IReadOnlyDictionary<TKey, TValue>? lhs,
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

        /// <summary>Computes the hash code for a dictionary.</summary>
        /// <param name="dict">The dictionary.</param>
        /// <returns>A hash code computed using the dictionary's entries.</returns>
        internal static int GetDictionaryHashCode<TKey, TValue>(this IReadOnlyDictionary<TKey, TValue> dict)
            where TKey : notnull
        {
            var hash = new System.HashCode();
            foreach (KeyValuePair<TKey, TValue> e in dict)
            {
                hash.Add(e.Key);
                hash.Add(e.Value);
            }
            return hash.ToHashCode();
        }
    }
}
