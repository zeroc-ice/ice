// Copyright (c) ZeroC, Inc.

package test.Ice.serialize;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.UnknownException;

import test.Ice.serialize.Test.Base;
import test.Ice.serialize.Test.Derived;
import test.Ice.serialize.Test.Ex;
import test.Ice.serialize.Test.Initial;
import test.Ice.serialize.Test.InitialPrx;
import test.Ice.serialize.Test.MyEnum;
import test.Ice.serialize.Test.Struct1;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.util.HashMap;

public final class InitialI implements Initial {
    InitialI(ObjectAdapter adapter, Identity ident) {
        _s = new Struct1();
        _s.bo = true;
        _s.by = (byte) 1;
        _s.sh = (short) 2;
        _s.i = 3;
        _s.l = 4;
        _s.f = (float) 5.0;
        _s.d = 6.0;
        _s.str = "7";
        _s.e = MyEnum.enum2;
        _s.p = InitialPrx.uncheckedCast(adapter.createProxy(ident));

        _d = new Derived();
        _d.b = _d;
        _d.o = _d;
        _d.s = _s;
        _d.seq1 = new byte[]{0, 1, 2, 3, 4};
        _d.seq2 = new int[]{5, 6, 7, 8, 9};
        _d.seq3 = new MyEnum[]{MyEnum.enum3, MyEnum.enum2, MyEnum.enum1};
        _d.seq4 = new Base[]{_d};
        _d.d1 = new HashMap<>();
        _d.d1.put((byte) 1, true);
        _d.d2 = new HashMap<>();
        _d.d2.put((short) 2, 3);
        _d.d3 = new HashMap<>();
        _d.d3.put("enum3", MyEnum.enum3);
        _d.d4 = new HashMap<>();
        _d.d4.put("b", _d);
        _d.p = _s.p;
    }

    @Override
    public byte[] getStruct1(Current current) {
        try {
            ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
            ObjectOutputStream stream = new ObjectOutputStream(byteStream);
            stream.writeObject(_s);
            return byteStream.toByteArray();
        } catch (IOException ex) {
            throw new UnknownException("dummy", ex);
        }
    }

    @Override
    public byte[] getBase(Current current) {
        try {
            ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
            ObjectOutputStream stream = new ObjectOutputStream(byteStream);
            stream.writeObject(_d);
            return byteStream.toByteArray();
        } catch (IOException ex) {
            throw new UnknownException("reason", ex);
        }
    }

    @Override
    public byte[] getEx(Current current) {
        try {
            Ex ex = new Ex();
            ex.s = _s;
            ex.b = _d;

            ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
            ObjectOutputStream stream = new ObjectOutputStream(byteStream);
            stream.writeObject(ex);
            return byteStream.toByteArray();
        } catch (IOException ex) {
            throw new UnknownException("reason", ex);
        }
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    private final Struct1 _s;
    private final Derived _d;
}
