// Copyright (c) ZeroC, Inc.

package test.Ice.optional;

import java.io.Serializable;

public class SerializableClass implements Serializable {
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

    private final int _v;
    private static final long serialVersionUID = 1;
}
