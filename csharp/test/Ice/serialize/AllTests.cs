// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;
using Test;

public class AllTests : TestCommon.TestApp
{
    //
    // There does not appear to be any way to compare collections
    // in either C# or with the .NET framework. Something like
    // C++ STL EqualRange would be nice...
    //
    private static bool Compare(ICollection c1, ICollection c2)
    {
        if (c1 == null)
        {
            return c2 == null;
        }
        if (c2 == null)
        {
            return false;
        }
        if (!c1.GetType().Equals(c2.GetType()))
        {
            return false;
        }

        if (c1.Count != c2.Count)
        {
            return false;
        }

        IEnumerator i1 = c1.GetEnumerator();
        IEnumerator i2 = c2.GetEnumerator();
        while (i1.MoveNext())
        {
            i2.MoveNext();
            if (i1.Current is ICollection)
            {
                Debug.Assert(i2.Current is ICollection);
                if (!Compare((ICollection)i1.Current, (ICollection)i2.Current))
                {
                    return false;
                }
            }
            else if (!i1.Current.Equals(i2.Current))
            {
                return false;
            }
        }
        return true;
    }

    static public int run(Ice.Communicator communicator)
    {
        Write("testing serialization... ");
        Flush();

        MyClassPrx proxy = MyClassPrxHelper.uncheckedCast(communicator.stringToProxy("test"));

        MyException ex, ex2;

        ex = new MyException();
        ex.name = "";
        ex.vss = new ValStruct[0];
        ex.vsl = new List<ValStruct>();
        ex.vsll = new LinkedList<ValStruct>();
        ex.vssk = new Stack<ValStruct>();
        ex.vsq = new Queue<ValStruct>();
        ex.isd = new Dictionary<int, string>();
        ex.ivd = new Dictionary<int, ValStruct>();
        ex.ipd = null;
        ex.issd = new SortedDictionary<int, string>();
        ex.optName = new Ice.Optional<string>();
        ex.optInt = new Ice.Optional<int>();
        ex.optValStruct = new Ice.Optional<ValStruct>();
        ex.optRefStruct = new Ice.Optional<RefStruct>();
        ex.optEnum = new Ice.Optional<MyEnum>();
        ex.optProxy = new Ice.Optional<MyClassPrx>();
        ex2 = inOut(ex);
        test(ex.Equals(ex2));

        ex.name = "MyException";
        ex.b = 1;
        ex.s = 2;
        ex.i = 3;
        ex.l = 4;
        ex.vs = new ValStruct(true, 1, 2, 3, 4, MyEnum.enum2);
        ex.rs = new RefStruct("RefStruct", "prop", null, null);
        ex.vss = new ValStruct[1];
        ex.vss[0] = ex.vs;
        ex.vsl = new List<ValStruct>();
        ex.vsl.Add(ex.vs);
        ex.vsll = new LinkedList<ValStruct>();
        ex.vsll.AddLast(ex.vs);
        ex.vssk = new Stack<ValStruct>();
        ex.vssk.Push(ex.vs);
        ex.vsq = new Queue<ValStruct>();
        ex.vsq.Enqueue(ex.vs);
        ex.isd = new Dictionary<int, string>();
        ex.isd[5] = "five";
        ex.ivd = new Dictionary<int, ValStruct>();
        ex.ivd[1] = ex.vs;
        ex.ipd = new Dictionary<int, MyClassPrx>();
        ex.issd = new SortedDictionary<int, string>();
        ex.issd[3] = "three";
        ex.optName = new Ice.Optional<string>("MyException");
        ex.optInt = new Ice.Optional<int>(99);
        ex.optValStruct = new Ice.Optional<ValStruct>(ex.vs);
        ex.optRefStruct = new Ice.Optional<RefStruct>(ex.rs);
        ex.optEnum = new Ice.Optional<MyEnum>(MyEnum.enum3);
        ex.optProxy = new Ice.Optional<MyClassPrx>(proxy);

        ex2 = inOut(ex);
        test(!ex.Equals(ex2));
        ex.ipd = null; // Not serialized
        ex.optProxy = Ice.Util.None; // Not serialized
        test(ex.optName.Equals(ex2.optName));
        test(ex.optInt.Equals(ex2.optInt));
        test(ex.optValStruct.Equals(ex2.optValStruct));
        test(ex.optRefStruct.Equals(ex2.optRefStruct));
        test(ex.optEnum.Equals(ex2.optEnum));
        test(ex.optProxy.Equals(ex2.optProxy));
        test(ex.Equals(ex2));

        RefStruct rs, rs2;
        rs = new RefStruct();
        rs.s = "RefStruct";
        rs.sp = "prop";
        rs.p = MyClassPrxHelper.uncheckedCast(communicator.stringToProxy("test"));
        rs.seq = new MyClassPrx[] { rs.p };
        rs2 = inOut(rs);
        test(rs2.s.Equals(rs.s));
        test(rs2.sp.Equals(rs.sp));
        test(rs2.p == null);
        test(rs2.seq == null);

        Base b, b2;
        b = new Base(true, 1, 2, 3, 4, MyEnum.enum2);
        b2 = inOut(b);
        test(b2.bo == b.bo);
        test(b2.by == b.by);
        test(b2.sh == b.sh);
        test(b2.i == b.i);
        test(b2.l == b.l);
        test(b2.e == b.e);

        MyClass c, c2;
        c = new MyClass(true, 1, 2, 3, 4, MyEnum.enum1, null, null, new ValStruct(true, 1, 2, 3, 4, MyEnum.enum2));
        c.c = c;
        c.o = c;
        c2 = inOut(c);
        test(c2.bo == c.bo);
        test(c2.by == c.by);
        test(c2.sh == c.sh);
        test(c2.i == c.i);
        test(c2.l == c.l);
        test(c2.e == c.e);
        test(c2.c == c2);
        test(c2.o == c2);
        test(c2.s.Equals(c.s));

        WriteLine("ok");
        return 0;
    }

    private static T inOut<T>(T o)
    {
        BinaryFormatter bin = new BinaryFormatter();
        using (MemoryStream mem = new MemoryStream())
        {
            bin.Serialize(mem, o);
            mem.Seek(0, 0);
            return (T)bin.Deserialize(mem);
        }
    }
}
