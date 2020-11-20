// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections;
using System.Collections.Generic;

namespace ZeroC.Ice.Test.SeqMapping
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

        public int Count => _list.Count;

        public T this[int index]
        {
            get => _list[index];
            set => _list[index] = value;
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
                var tmp = (Custom<T>)o;
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
            catch
            {
                return false;
            }
        }

        public override int GetHashCode() => base.GetHashCode();

        private readonly List<T> _list = new();
    }
}
