// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;

namespace Ice
{
    public class Comparer
    {
        //
        // Try to compare two collections efficiently, by doing a reference
        // and count comparison. If equality or inequality can be determined
        // this way, 'result' contains the outcome of the comparison and the
        // return value is true. Otherwise, if equality or inequality cannot
        // be determined this way, 'result' and return value are both false.
        //
        private static bool cheapComparison(System.Collections.ICollection c1,
                                            System.Collections.ICollection c2,
                                            out bool result)
        {
            if(object.ReferenceEquals(c1, c2))
            {
                result = true;
                return true; // Equal references means the collections are equal.
            }
            if(c1 == null || c2 == null)
            {
                result = false;
                return true; // The references are not equal and one of them is null, so c1 and c2 are not equal.
            }
            if(c1.Count != c2.Count)
            {
                result = false;
                return true; // Different number of elements, so c1 and c2 are not equal.
            }
            if(c1.Count == 0)
            {
                result = true;
                return true; // Same number of elements, both zero, so c1 and c2 are equal.
            }

            result = false; // Couldn't get a result cheaply.
            return false;   // c1 and c2 may still be equal, but we have to compare elements to find out.
        }

        //
        // Compare two dictionaries for equality.
        //
        public static bool ValueEquals(System.Collections.IDictionary d1, System.Collections.IDictionary d2)
        {
            bool result;
            if(cheapComparison(d1, d2, out result))
            {
                return result;
            }

            //
            // Compare both sets of keys. Keys are unique and non-null.
            //
            System.Collections.ICollection keys1 = d1.Keys;
            System.Collections.ICollection keys2 = d2.Keys;
            object[] ka1 = new object[d1.Count];
            object[] ka2 = new object[d2.Count];
            keys1.CopyTo(ka1, 0);
            keys2.CopyTo(ka2, 0);
            Array.Sort(ka1);
            Array.Sort(ka2);

            System.Collections.IEnumerator e = ka2.GetEnumerator();
            foreach(object o in ka1)
            {
                e.MoveNext();
                if(!o.Equals(e.Current))
                {
                    return false;
                }
            }

            //
            // Compare values.
            //
            foreach(object o in ka1)
            {
                object v1 = d1[o];
                object v2 = d2[o];
                if(v1 == null)
                {
                    if(v2 != null)
                    {
                        return false;
                    }
                }
                else
                {
                    System.Collections.IDictionary vl = null;
                    System.Collections.IDictionary vr = null;
                    try
                    {
                        vl = (System.Collections.IDictionary)v1;
                        vr = (System.Collections.IDictionary)v2;
                    }
                    catch(InvalidCastException)
                    {
                       // Ignore
                    }
                    if(vl != null)
                    {
                        if(!ValueEquals(vl, vr))
                        {
                            return false;
                        }
                    }
                    else
                    {
                        if(!v1.Equals(v2))
                        {
                            return false;
                        }
                    }
                }
            }
            return true;
        }

        //
        // Compare two collections for equality.
        //
        public static bool Foo(System.Collections.ICollection c1, System.Collections.ICollection c2)
        {
            bool result;
            if(cheapComparison(c1, c2, out result))
            {
                return result;
            }

            System.Collections.IEnumerator e = c2.GetEnumerator();
            foreach(object o in c1)
            {
                e.MoveNext();
                if(!o.Equals(e.Current))
                {
                    return false;
                }
            }
            return true;
        }
    }
}
