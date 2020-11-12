// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;

namespace ZeroC.Ice
{
    public static class EnumerableExtensions
    {
        /// <summary>Computes the hash code for a sequence.</summary>
        /// <param name="sequence">The sequence.</param>
        /// <returns>A hash code computed using the sequence's elements.</returns>
        public static int GetSequenceHashCode<T>(this IEnumerable<T> sequence)
        {
            var hash = new HashCode();
            foreach (T element in sequence)
            {
                hash.Add(element);
            }
            return hash.ToHashCode();
        }
    }
}
