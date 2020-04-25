//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections;
using System.Collections.Generic;

namespace Ice.seqMapping
{
    public class Custom<T> : IReadOnlyCollection<T>
    {
        public Custom(List<T> list) => _list = list;

        public Custom(IEnumerable<T> enumerable)
        {
            foreach (T v in enumerable)
            {
                Add(v);
            }
        }

        public Custom()
        {
        }

        IEnumerator IEnumerable.GetEnumerator() => _list.GetEnumerator();

        public IEnumerator<T> GetEnumerator() => _list.GetEnumerator();

        public int Count
        {
            get
            {
                return _list.Count;
            }
        }

        public T this[int index]
        {
            get
            {
                return _list[index];
            }

            set
            {
                _list[index] = value;
            }

        }

        public void Add(T elmt) => _list.Add(elmt);

        public override bool Equals(object? o)
        {
            if (o == null)
            {
                return false;
            }
            try
            {
                Custom<T> tmp = (Custom<T>)o;
                IEnumerator<T> e = tmp.GetEnumerator();
                foreach (T elmt in _list)
                {
                    if (!e.MoveNext())
                    {
                        return false;
                    }
                    if (elmt == null)
                    {
                        if (e.Current != null)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        if (!elmt.Equals(e.Current))
                        {
                            return false;
                        }
                    }
                }
                return true;
            }
            catch (System.Exception)
            {
                return false;
            }
        }

        public override int GetHashCode() => base.GetHashCode();

        private List<T> _list = new List<T>();
    }
}
