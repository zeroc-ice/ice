// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;

namespace ZeroC.Ice
{
    public static class EnumerableExtensions
    {
        /// <summary>Computes the hash code for a sequence, using each element's default comparer.</summary>
        /// <param name="sequence">The sequence.</param>
        /// <typeparam name="T">The type of sequence's element.</typeparam>
        /// <returns>A hash code computed using the sequence's elements.</returns>
        public static int GetSequenceHashCode<T>(this IEnumerable<T> sequence) =>
            GetSequenceHashCode(sequence, comparer: null);

        /// <summary>Computes the hash code for a sequence.</summary>
        /// <typeparam name="T">The type of sequence's element.</typeparam>
        /// <param name="sequence">The sequence.</param>
        /// <param name="comparer">The comparer used to get each element's hash code. When null, this method uses the
        /// default comparer.</param>
        /// <returns>A hash code computed using the sequence's elements.</returns>
        public static int GetSequenceHashCode<T>(this IEnumerable<T> sequence, IEqualityComparer<T>? comparer)
        {
            comparer ??= EqualityComparer<T>.Default;

            var hash = new HashCode();
            foreach (T element in sequence)
            {
                if (element != null)
                {
                    hash.Add(comparer.GetHashCode(element));
                }
            }
            return hash.ToHashCode();
        }
    }
}
