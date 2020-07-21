//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
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
                                      vss: Array.Empty<ValStruct>(),
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

            TestHelper.Assert(ex2.Name.Length == 0);
            TestHelper.Assert(ex2.Vss!.Length == 0);
            TestHelper.Assert(ex2.Vsll!.Count == 0);
            TestHelper.Assert(ex2.Vssk!.Count == 0);
            TestHelper.Assert(ex2.Vsq!.Count == 0);
            TestHelper.Assert(ex2.Isd!.Count == 0);
            TestHelper.Assert(ex2.Ivd!.Count == 0);
            TestHelper.Assert(ex2.Ipd.Count == 0);
            TestHelper.Assert(ex2.Issd!.Count == 0);
            TestHelper.Assert(ex2.OptName == null);
            TestHelper.Assert(ex2.OptInt == null);
            TestHelper.Assert(ex2.OptValStruct == null);
            TestHelper.Assert(ex2.OptRefStruct == null);
            TestHelper.Assert(ex2.OptEnum == null);
            TestHelper.Assert(ex2.OptClass == null);
            TestHelper.Assert(ex2.OptProxy == null);

            ex.Name = "MyException";
            ex.B = 1;
            ex.S = 2;
            ex.I = 3;
            ex.L = 4;
            ex.Vs = new ValStruct(true, 1, 2, 3, 4, MyEnum.enum2);
            ex.Rs = new RefStruct("RefStruct", "prop", null, proxy, new IMyInterfacePrx?[] { proxy, null, proxy });
            ex.Vss = new ValStruct[1];
            ex.Vss[0] = ex.Vs;
            ex.Vsl = new List<ValStruct>();
            ex.Vsl.Add(ex.Vs);
            ex.Vsll = new LinkedList<ValStruct>();
            ex.Vsll.AddLast(ex.Vs);
            ex.Vssk = new Stack<ValStruct>();
            ex.Vssk.Push(ex.Vs);
            ex.Vsq = new Queue<ValStruct>();
            ex.Vsq.Enqueue(ex.Vs);
            ex.Isd = new Dictionary<int, string>();
            ex.Isd[5] = "five";
            ex.Ivd = new Dictionary<int, ValStruct>();
            ex.Ivd[1] = ex.Vs;
            ex.Ipd = new Dictionary<int, IMyInterfacePrx?>() { { 1, proxy }, { 2, null }, { 3, proxy } };
            ex.Issd = new SortedDictionary<int, string>();
            ex.Issd[3] = "three";
            ex.OptName = "MyException";
            ex.OptInt = 99;
            ex.OptValStruct = ex.Vs;
            ex.OptRefStruct = ex.Rs;
            ex.OptEnum = MyEnum.enum3;
            ex.OptClass = null;
            ex.OptProxy = proxy;
            ex2 = inOut(ex, communicator);

            TestHelper.Assert(ex2.Name.Equals(ex.Name));
            TestHelper.Assert(ex2.B == ex.B);
            TestHelper.Assert(ex2.S == ex.S);
            TestHelper.Assert(ex2.I == ex.I);
            TestHelper.Assert(ex2.L == ex.L);
            TestHelper.Assert(ex2.Vs.Equals(ex.Vs));
            TestHelper.Assert(ex2.Rs.P != null && ex2.Rs.P.Equals(ex.Rs.P));
            TestHelper.Assert(ex2.Vss[0].Equals(ex.Vs));
            TestHelper.Assert(ex2.Vsll.Count == 1 && ex2.Vsll.Last!.Value.Equals(ex.Vs));
            TestHelper.Assert(ex2.Vssk.Count == 1 && ex2.Vssk.Peek().Equals(ex.Vs));
            TestHelper.Assert(ex2.Vsq.Count == 1 && ex2.Vsq.Peek().Equals(ex.Vs));
            TestHelper.Assert(ex2.Isd.Count == 1 && ex2.Isd[5].Equals("five"));
            TestHelper.Assert(ex2.Ivd.Count == 1 && ex2.Ivd[1].Equals(ex.Vs));
            TestHelper.Assert(ex2.Ipd.Count == 3 && ex2.Ipd[2] == null);
            TestHelper.Assert(ex2.Issd.Count == 1 && ex2.Issd[3] == "three");
            TestHelper.Assert(ex2.OptName == "MyException");
            TestHelper.Assert(ex2.OptInt.HasValue && ex2.OptInt.Value == 99);
            TestHelper.Assert(ex2.OptValStruct.HasValue && ex2.OptValStruct.Value.Equals(ex.Vs));
            TestHelper.Assert(ex2.OptRefStruct != null && ex2.OptRefStruct.Value.P!.Equals(ex.Rs.P));
            TestHelper.Assert(ex2.OptEnum.HasValue && ex2.OptEnum.Value == MyEnum.enum3);
            TestHelper.Assert(ex2.OptClass == null);
            TestHelper.Assert(ex2.OptProxy!.Equals(proxy));

            RefStruct rs, rs2;
            rs = new RefStruct();
            rs.S = "RefStruct";
            rs.Sp = "prop";
            rs.C = null;
            rs.P = IMyInterfacePrx.Parse("test", communicator);
            rs.Seq = new IMyInterfacePrx[] { rs.P };
            rs2 = inOut(rs, communicator);
            TestHelper.Assert(rs2.S == "RefStruct");
            TestHelper.Assert(rs2.Sp == "prop");
            TestHelper.Assert(rs2.C == null);
            TestHelper.Assert(rs2.P != null && rs2.P.Equals(rs.P));
            TestHelper.Assert(rs2.Seq.Length == rs.Seq.Length);
            TestHelper.Assert(rs2.Seq[0]!.Equals(rs.Seq[0]));

            Base b, b2;
            b = new Base(true, 1, 2, 3, 4, MyEnum.enum2);
            b2 = inOut(b, communicator);
            TestHelper.Assert(b2.Bo == b.Bo);
            TestHelper.Assert(b2.By == b.By);
            TestHelper.Assert(b2.Sh == b.Sh);
            TestHelper.Assert(b2.I == b.I);
            TestHelper.Assert(b2.L == b.L);
            TestHelper.Assert(b2.E == b.E);

            MyClass c, c2;
            c = new MyClass(true, 1, 2, 3, 4, MyEnum.enum1, null, null, new ValStruct(true, 1, 2, 3, 4, MyEnum.enum2));
            c.C = c;
            c.O = c;
            c2 = inOut(c, communicator);
            TestHelper.Assert(c2.Bo == c.Bo);
            TestHelper.Assert(c2.By == c.By);
            TestHelper.Assert(c2.Sh == c.Sh);
            TestHelper.Assert(c2.I == c.I);
            TestHelper.Assert(c2.L == c.L);
            TestHelper.Assert(c2.E == c.E);
            TestHelper.Assert(c2.C == c2);
            TestHelper.Assert(c2.O == c2);
            TestHelper.Assert(c2.S.Equals(c.S));

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
