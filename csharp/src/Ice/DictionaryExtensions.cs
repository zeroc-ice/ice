// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;

namespace ZeroC.Ice
{
    public static class DictionaryExtensions
    {
        /// <summary>Compares two dictionaries for equality. <see
        /// cref="System.Linq.Enumerable.SequenceEqual{T}(IEnumerable{T}, IEnumerable{T})"/> provides the equivalent
        /// functionality for sequences.</summary>
        /// <typeparam name="TKey">The type of dictionary's key.</typeparam>
        /// <typeparam name="TValue">The type of dictionary's value.</typeparam>
        /// <param name="lhs">The first dictionary to compare.</param>
        /// <param name="rhs">The second dictionary to compare.</param>
        /// <returns>True if the two dictionaries have the exact same entries using the value's default equality
        /// comparison; otherwise, false.</returns>
        public static bool DictionaryEqual<TKey, TValue>(
            this IReadOnlyDictionary<TKey, TValue> lhs,
            IReadOnlyDictionary<TKey, TValue> rhs) where TKey : notnull =>
            DictionaryEqual(lhs, rhs, valueComparer: null);

        /// <summary>Compares two dictionaries for equality. <see
        /// cref="System.Linq.Enumerable.SequenceEqual{T}(IEnumerable{T}, IEnumerable{T}, IEqualityComparer{T}?)"/>
        /// provides the equivalent functionality for sequences.</summary>
        /// <typeparam name="TKey">The type of dictionary's key.</typeparam>
        /// <typeparam name="TValue">The type of dictionary's value.</typeparam>
        /// <param name="lhs">The first dictionary to compare.</param>
        /// <param name="rhs">The second dictionary to compare.</param>
        /// <param name="valueComparer">The comparer used to compare values for equality. When null, this method uses
        /// the default comparer.</param>
        /// <returns>True if the two dictionaries have the exact same entries; otherwise, false.</returns>
        public static bool DictionaryEqual<TKey, TValue>(
            this IReadOnlyDictionary<TKey, TValue> lhs,
            IReadOnlyDictionary<TKey, TValue> rhs,
            IEqualityComparer<TValue>? valueComparer) where TKey : notnull
        {
            if (lhs == null)
            {
                throw new ArgumentNullException($"{nameof(lhs)} is null", nameof(lhs));
            }
            if (rhs == null)
            {
                throw new ArgumentNullException($"{nameof(rhs)} is null", nameof(rhs));
            }

            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs.Count != rhs.Count)
            {
                return false;
            }

            valueComparer ??= EqualityComparer<TValue>.Default;

            foreach ((TKey key, TValue value) in lhs)
            {
                if (!rhs.TryGetValue(key, out TValue rhsValue) || !valueComparer.Equals(value, rhsValue))
                {
                    return false;
                }
            }
            return true;
        }

        /// <summary>Computes the hash code for a dictionary using the dictionary value's default comparer.</summary>
        /// <typeparam name="TKey">The type of dictionary's key.</typeparam>
        /// <typeparam name="TValue">The type of dictionary's value.</typeparam>
        /// <param name="dict">The dictionary.</param>
        /// <returns>A hash code computed using the dictionary's entries.</returns>
        public static int GetDictionaryHashCode<TKey, TValue>(this IReadOnlyDictionary<TKey, TValue> dict)
            where TKey : notnull =>
            GetDictionaryHashCode(dict, valueComparer: null);

        /// <summary>Computes the hash code for a dictionary.</summary>
        /// <typeparam name="TKey">The type of dictionary's key.</typeparam>
        /// <typeparam name="TValue">The type of dictionary's value.</typeparam>
        /// <param name="dict">The dictionary.</param>
        /// <param name="valueComparer">The comparer used to get the hash code of each value. When null, this method
        /// uses the default comparer.</param>
        /// <returns>A hash code computed using the dictionary's entries.</returns>
        public static int GetDictionaryHashCode<TKey, TValue>(
            this IReadOnlyDictionary<TKey, TValue> dict,
            IEqualityComparer<TValue>? valueComparer)
            where TKey : notnull
        {
            valueComparer ??= EqualityComparer<TValue>.Default;

            var hash = new HashCode();
            foreach ((TKey key, TValue value) in dict)
            {
                hash.Add(key);
                if (value != null)
                {
                    hash.Add(valueComparer.GetHashCode(value));
                }
            }
            return hash.ToHashCode();
        }
    }
}
