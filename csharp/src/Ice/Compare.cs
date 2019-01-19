//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    public class CollectionComparer
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
            if(ReferenceEquals(c1, c2))
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
        // Compare two dictionaries for value equality (as implemented by the Equals() method of its elements).
        //
        public static bool Equals(System.Collections.IDictionary d1, System.Collections.IDictionary d2)
        {
            try
            {
                bool result;
                if(cheapComparison(d1, d2, out result))
                {
                    return result;
                }

                System.Collections.ICollection keys1 = d1.Keys;
                foreach(object k in keys1)
                {
                    if(d2.Contains(k))
                    {
                        object v1 = d1[k];
                        object v2 = d2[k];
                        if(v1 == null)
                        {
                            if(v2 != null)
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
                    else
                    {
                        return false;
                    }
                }
            }
            catch(System.Exception)
            {
                return false;
            }

            return true;
        }

        //
        // Compare two collections for equality (as implemented by the Equals() method of its elements).
        // Order is significant.
        //
        public static bool Equals(System.Collections.ICollection c1, System.Collections.ICollection c2)
        {
            try
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
                    if(!Equals(o, e.Current))
                    {
                        return false;
                    }
                }
                return true;
            }
            catch(System.Exception)
            {
                return false;
            }
        }

        //
        // Compare two collections for equality (as implemented by the Equals() method of its elements).
        // Order is significant.
        //
        public static bool Equals(System.Collections.IEnumerable c1, System.Collections.IEnumerable c2)
        {
            try
            {
                if(ReferenceEquals(c1, c2))
                {
                    return true; // Equal references means the collections are equal.
                }
                if(c1 == null || c2 == null)
                {
                    return false; // The references are not equal and one of them is null, so c1 and c2 are not equal.
                }

                System.Collections.IEnumerator e1 = c1.GetEnumerator();
                System.Collections.IEnumerator e2 = c2.GetEnumerator();
                while(e1.MoveNext())
                {
                    if(!e2.MoveNext())
                    {
                        return false; // c2 has fewer elements than c1.
                    }
                    if(e1.Current == null)
                    {
                        if(e2.Current != null)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        if(!e1.Current.Equals(e2.Current))
                        {
                            return false;
                        }
                    }
                }
                if(e2.MoveNext())
                {
                    return false; // c2 has more elements than c1.
                }
                return true;
            }
            catch(System.Exception)
            {
                return false;
            }
        }
    }
}
