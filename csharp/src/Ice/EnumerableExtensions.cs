//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Linq;

namespace Ice
{
    public static class EnumerableExtensions
    {
        /// <summary>Shuffle the elements of a sequence.</summary>
        /// <typeparam name="TSource">The type of the elements of source.</typeparam>
        /// <param name="source">A sequence of values to shuffle.</param>
        /// <returns>An IEnumerable&lt;TSouce&gt; whose elements are shuffled.</returns>
        public static IEnumerable<TSource> Shuffle<TSource>(this IEnumerable<TSource> source)
        {
            lock (_rand)
            {
                return source.OrderBy(element => _rand.Next());
            }
        }

        private static readonly Random _rand = new Random();
    }
}
