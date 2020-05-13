//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading;
using System.Collections.Generic;
using System.Linq;
using System.Diagnostics;

namespace Ice
{
    public static class EnumerableExtensions
    {
        private static readonly ThreadLocal<Random> _rand = new ThreadLocal<Random>(() => new Random());

        /// <summary>Shuffle the elements of a sequence.</summary>
        /// <typeparam name="TSource">The type of the elements of source.</typeparam>
        /// <param name="source">A sequence of values to shuffle.</param>
        /// <returns>An IEnumerable&lt;TSouce&gt; whose elements are shuffled.</returns>
        public static IEnumerable<TSource> Shuffle<TSource>(this IEnumerable<TSource> source)
        {
            Random? rand = _rand.Value;
            Debug.Assert(rand != null);
            return source.OrderBy(element => rand.Next());
        }
    }
}
