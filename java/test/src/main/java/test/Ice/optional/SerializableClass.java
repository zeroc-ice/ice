// Copyright (c) ZeroC, Inc.

package test.Ice.optional;

public class SerializableClass implements java.io.Serializable {
    public SerializableClass(int v) {
        _v = v;
    }

    @Override
    public boolean equals(java.lang.Object obj) {
        if (obj instanceof SerializableClass) {
            return _v == ((SerializableClass) obj)._v;
        }

        return false;
    }

    private int _v;
    private static final long serialVersionUID = 1;
}
