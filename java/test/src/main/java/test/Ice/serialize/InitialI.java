// Copyright (c) ZeroC, Inc.

package test.Ice.serialize;

import test.Ice.serialize.Test.*;

import java.io.*;

public final class InitialI implements Initial {
    InitialI(com.zeroc.Ice.ObjectAdapter adapter, com.zeroc.Ice.Identity ident) {
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
        _d.seq1 = new byte[] {0, 1, 2, 3, 4};
        _d.seq2 = new int[] {5, 6, 7, 8, 9};
        _d.seq3 = new MyEnum[] {MyEnum.enum3, MyEnum.enum2, MyEnum.enum1};
        _d.seq4 = new Base[] {_d};
        _d.d1 = new java.util.HashMap<>();
        _d.d1.put((byte) 1, true);
        _d.d2 = new java.util.HashMap<>();
        _d.d2.put((short) 2, 3);
        _d.d3 = new java.util.HashMap<>();
        _d.d3.put("enum3", MyEnum.enum3);
        _d.d4 = new java.util.HashMap<>();
        _d.d4.put("b", _d);
        _d.p = _s.p;
    }

    @Override
    public byte[] getStruct1(com.zeroc.Ice.Current current) {
        try {
            ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
            ObjectOutputStream stream = new ObjectOutputStream(byteStream);
            stream.writeObject(_s);
            return byteStream.toByteArray();
        } catch (IOException ex) {
            throw new com.zeroc.Ice.UnknownException("dummy", ex);
        }
    }

    @Override
    public byte[] getBase(com.zeroc.Ice.Current current) {
        try {
            ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
            ObjectOutputStream stream = new ObjectOutputStream(byteStream);
            stream.writeObject(_d);
            return byteStream.toByteArray();
        } catch (IOException ex) {
            throw new com.zeroc.Ice.UnknownException("reason", ex);
        }
    }

    @Override
    public byte[] getEx(com.zeroc.Ice.Current current) {
        try {
            Ex ex = new Ex();
            ex.s = _s;
            ex.b = _d;

            ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
            ObjectOutputStream stream = new ObjectOutputStream(byteStream);
            stream.writeObject(ex);
            return byteStream.toByteArray();
        } catch (IOException ex) {
            throw new com.zeroc.Ice.UnknownException("reason", ex);
        }
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    private Struct1 _s;
    private Derived _d;
}
