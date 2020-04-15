//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.IO;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using Test;

namespace Ice.serialize
{
    public class AllTests
    {
        static public int allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            TextWriter output = helper.GetWriter();
            output.Write("testing serialization... ");
            output.Flush();

            var proxy = Test.IMyInterfacePrx.Parse("test", communicator);

            Test.MyException ex, ex2;
            ex = new Test.MyException();
            ex.name = "";
            ex.vss = new Test.ValStruct[0];
            ex.vsl = new List<Test.ValStruct>();
            ex.vsll = new LinkedList<Test.ValStruct>();
            ex.vssk = new Stack<Test.ValStruct>();
            ex.vsq = new Queue<Test.ValStruct>();
            ex.isd = new Dictionary<int, string>();
            ex.ivd = new Dictionary<int, Test.ValStruct>();
            ex.ipd = new Dictionary<int, Test.IMyInterfacePrx?>(); ;
            ex.issd = new SortedDictionary<int, string>();
            ex.optName = null;
            ex.optInt = null;
            ex.optValStruct = null;
            ex.optRefStruct = null;
            ex.optEnum = null;
            ex.optClass = null;
            ex.optProxy = null;
            ex2 = inOut(ex, communicator);

            TestHelper.Assert(ex2.name == "");
            TestHelper.Assert(ex2.vss!.Length == 0);
            TestHelper.Assert(ex2.vsll!.Count == 0);
            TestHelper.Assert(ex2.vssk!.Count == 0);
            TestHelper.Assert(ex2.vsq!.Count == 0);
            TestHelper.Assert(ex2.isd!.Count == 0);
            TestHelper.Assert(ex2.ivd!.Count == 0);
            TestHelper.Assert(ex2.ipd.Count == 0);
            TestHelper.Assert(ex2.issd!.Count == 0);
            TestHelper.Assert(ex2.optName == null);
            TestHelper.Assert(ex2.optInt == null);
            TestHelper.Assert(ex2.optValStruct == null);
            TestHelper.Assert(ex2.optRefStruct == null);
            TestHelper.Assert(ex2.optEnum == null);
            TestHelper.Assert(ex2.optClass == null);
            TestHelper.Assert(ex2.optProxy == null);

            ex.name = "MyException";
            ex.b = 1;
            ex.s = 2;
            ex.i = 3;
            ex.l = 4;
            ex.vs = new Test.ValStruct(true, 1, 2, 3, 4, Test.MyEnum.enum2);
            ex.rs = new Test.RefStruct("RefStruct", "prop", null, proxy, new Test.IMyInterfacePrx?[] { proxy, null, proxy });
            ex.vss = new Test.ValStruct[1];
            ex.vss[0] = ex.vs;
            ex.vsl = new List<Test.ValStruct>();
            ex.vsl.Add(ex.vs);
            ex.vsll = new LinkedList<Test.ValStruct>();
            ex.vsll.AddLast(ex.vs);
            ex.vssk = new Stack<Test.ValStruct>();
            ex.vssk.Push(ex.vs);
            ex.vsq = new Queue<Test.ValStruct>();
            ex.vsq.Enqueue(ex.vs);
            ex.isd = new Dictionary<int, string>();
            ex.isd[5] = "five";
            ex.ivd = new Dictionary<int, Test.ValStruct>();
            ex.ivd[1] = ex.vs;
            ex.ipd = new Dictionary<int, Test.IMyInterfacePrx?>() { { 1, proxy }, { 2, null }, { 3, proxy } };
            ex.issd = new SortedDictionary<int, string>();
            ex.issd[3] = "three";
            ex.optName = "MyException";
            ex.optInt = 99;
            ex.optValStruct = ex.vs;
            ex.optRefStruct = ex.rs;
            ex.optEnum = Test.MyEnum.enum3;
            ex.optClass = null;
            ex.optProxy = proxy;
            ex2 = inOut(ex, communicator);

            TestHelper.Assert(ex2.name.Equals(ex.name));
            TestHelper.Assert(ex2.b == ex.b);
            TestHelper.Assert(ex2.s == ex.s);
            TestHelper.Assert(ex2.i == ex.i);
            TestHelper.Assert(ex2.l == ex.l);
            TestHelper.Assert(ex2.vs.Equals(ex.vs));
            TestHelper.Assert(ex2.rs.p != null && ex2.rs.p.Equals(ex.rs.p));
            TestHelper.Assert(ex2.vss[0].Equals(ex.vs));
            TestHelper.Assert(ex2.vsll.Count == 1 && ex2.vsll.Last!.Value.Equals(ex.vs));
            TestHelper.Assert(ex2.vssk.Count == 1 && ex2.vssk.Peek().Equals(ex.vs));
            TestHelper.Assert(ex2.vsq.Count == 1 && ex2.vsq.Peek().Equals(ex.vs));
            TestHelper.Assert(ex2.isd.Count == 1 && ex2.isd[5].Equals("five"));
            TestHelper.Assert(ex2.ivd.Count == 1 && ex2.ivd[1].Equals(ex.vs));
            TestHelper.Assert(ex2.ipd.Count == 3 && ex2.ipd[2] == null);
            TestHelper.Assert(ex2.issd.Count == 1 && ex2.issd[3] == "three");
            TestHelper.Assert(ex2.optName == "MyException");
            TestHelper.Assert(ex2.optInt.HasValue && ex2.optInt.Value == 99);
            TestHelper.Assert(ex2.optValStruct.HasValue && ex2.optValStruct.Value.Equals(ex.vs));
            TestHelper.Assert(ex2.optRefStruct != null && ex2.optRefStruct.Value.p!.Equals(ex.rs.p));
            TestHelper.Assert(ex2.optEnum.HasValue && ex2.optEnum.Value == Test.MyEnum.enum3);
            TestHelper.Assert(ex2.optClass == null);
            TestHelper.Assert(ex2.optProxy!.Equals(proxy));

            Test.RefStruct rs, rs2;
            rs = new Test.RefStruct();
            rs.s = "RefStruct";
            rs.sp = "prop";
            rs.c = null;
            rs.p = Test.IMyInterfacePrx.Parse("test", communicator);
            rs.seq = new Test.IMyInterfacePrx[] { rs.p };
            rs2 = inOut(rs, communicator);
            TestHelper.Assert(rs2.s == "RefStruct");
            TestHelper.Assert(rs2.sp == "prop");
            TestHelper.Assert(rs2.c == null);
            TestHelper.Assert(rs2.p != null && rs2.p.Equals(rs.p));
            TestHelper.Assert(rs2.seq.Length == rs.seq.Length);
            TestHelper.Assert(rs2.seq[0]!.Equals(rs.seq[0]));

            Test.Base b, b2;
            b = new Test.Base(true, 1, 2, 3, 4, Test.MyEnum.enum2);
            b2 = inOut(b, communicator);
            TestHelper.Assert(b2.bo == b.bo);
            TestHelper.Assert(b2.by == b.by);
            TestHelper.Assert(b2.sh == b.sh);
            TestHelper.Assert(b2.i == b.i);
            TestHelper.Assert(b2.l == b.l);
            TestHelper.Assert(b2.e == b.e);

            Test.MyClass c, c2;
            c = new Test.MyClass(true, 1, 2, 3, 4, Test.MyEnum.enum1, null, null, new Test.ValStruct(true, 1, 2, 3, 4, Test.MyEnum.enum2));
            c.c = c;
            c.o = c;
            c2 = inOut(c, communicator);
            TestHelper.Assert(c2.bo == c.bo);
            TestHelper.Assert(c2.by == c.by);
            TestHelper.Assert(c2.sh == c.sh);
            TestHelper.Assert(c2.i == c.i);
            TestHelper.Assert(c2.l == c.l);
            TestHelper.Assert(c2.e == c.e);
            TestHelper.Assert(c2.c == c2);
            TestHelper.Assert(c2.o == c2);
            TestHelper.Assert(c2.s.Equals(c.s));

            output.WriteLine("ok");
            return 0;
        }

        private static T inOut<T>(T o, Communicator communicator)
        {
            var bin = new BinaryFormatter(null, new StreamingContext(StreamingContextStates.All, communicator));
            using MemoryStream mem = new MemoryStream();
            bin.Serialize(mem, o);
            mem.Seek(0, 0);
            return (T)bin.Deserialize(mem);
        }
    }
}
