// Copyright (c) ZeroC, Inc.

namespace Ice.seqMapping;

public class Custom<T> : IEnumerable<T>
{
    System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator() => _list.GetEnumerator();

    public IEnumerator<T> GetEnumerator() => _list.GetEnumerator();

    public int Count => _list.Count;

    public T this[int index]
    {
        get => _list[index];

        set => _list[index] = value;
    }

    public void Add(T element) => _list.Add(element);

    public override bool Equals(object obj) => obj is Custom<T> other && _list.SequenceEqual(other._list);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        foreach (T element in _list)
        {
            hash.Add(element);
        }
        return hash.ToHashCode();
    }

    private readonly List<T> _list = new List<T>();
}
