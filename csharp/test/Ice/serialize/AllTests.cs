//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.IO;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using Test;

namespace ZeroC.Ice.Test.Serialize
{
    public class AllTests
    {
        public static int allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            TextWriter output = helper.GetWriter();
            output.Write("testing serialization... ");
            output.Flush();

            var proxy = IMyInterfacePrx.Parse("test", communicator);

            MyException ex, ex2;
            ex = new MyException(name: "",
                                      b: 0,
                                      s: 0,
                                      i: 0,
                                      l: 0,
                                      vs: default,
                                      rs: default,
                                      c: null,
                                      p: null,
                                      vss: new ValStruct[0],
                                      vsl: new List<ValStruct>(),
                                      vsll: new LinkedList<ValStruct>(),
                                      vssk: new Stack<ValStruct>(),
                                      vsq: new Queue<ValStruct>(),
                                      isd: new Dictionary<int, string>(),
                                      ivd: new Dictionary<int, ValStruct>(),
                                      ipd: new Dictionary<int, IMyInterfacePrx?>(),
                                      issd: new SortedDictionary<int, string>(),
                                      optName: null,
                                      optInt: null,
                                      optValStruct: null,
                                      optRefStruct: null,
                                      optEnum: null,
                                      optClass: null,
                                      optProxy: null);

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
            ex.vs = new ValStruct(true, 1, 2, 3, 4, MyEnum.enum2);
            ex.rs = new RefStruct("RefStruct", "prop", null, proxy, new IMyInterfacePrx?[] { proxy, null, proxy });
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
            ex.ipd = new Dictionary<int, IMyInterfacePrx?>() { { 1, proxy }, { 2, null }, { 3, proxy } };
            ex.issd = new SortedDictionary<int, string>();
            ex.issd[3] = "three";
            ex.optName = "MyException";
            ex.optInt = 99;
            ex.optValStruct = ex.vs;
            ex.optRefStruct = ex.rs;
            ex.optEnum = MyEnum.enum3;
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
            TestHelper.Assert(ex2.optEnum.HasValue && ex2.optEnum.Value == MyEnum.enum3);
            TestHelper.Assert(ex2.optClass == null);
            TestHelper.Assert(ex2.optProxy!.Equals(proxy));

            RefStruct rs, rs2;
            rs = new RefStruct();
            rs.s = "RefStruct";
            rs.sp = "prop";
            rs.c = null;
            rs.p = IMyInterfacePrx.Parse("test", communicator);
            rs.seq = new IMyInterfacePrx[] { rs.p };
            rs2 = inOut(rs, communicator);
            TestHelper.Assert(rs2.s == "RefStruct");
            TestHelper.Assert(rs2.sp == "prop");
            TestHelper.Assert(rs2.c == null);
            TestHelper.Assert(rs2.p != null && rs2.p.Equals(rs.p));
            TestHelper.Assert(rs2.seq.Length == rs.seq.Length);
            TestHelper.Assert(rs2.seq[0]!.Equals(rs.seq[0]));

            Base b, b2;
            b = new Base(true, 1, 2, 3, 4, MyEnum.enum2);
            b2 = inOut(b, communicator);
            TestHelper.Assert(b2.bo == b.bo);
            TestHelper.Assert(b2.by == b.by);
            TestHelper.Assert(b2.sh == b.sh);
            TestHelper.Assert(b2.i == b.i);
            TestHelper.Assert(b2.l == b.l);
            TestHelper.Assert(b2.e == b.e);

            MyClass c, c2;
            c = new MyClass(true, 1, 2, 3, 4, MyEnum.enum1, null, null, new ValStruct(true, 1, 2, 3, 4, MyEnum.enum2));
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
