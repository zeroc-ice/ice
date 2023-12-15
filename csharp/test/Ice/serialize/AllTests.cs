//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;
using System.Runtime.Serialization;

namespace Ice
{
    namespace serialize
    {
        public class AllTests : global::Test.AllTests
        {
            //
            // There does not appear to be any way to compare collections
            // in either C# or with the .NET framework. Something like
            // C++ STL EqualRange would be nice...
            //
            private static bool Compare(ICollection c1, ICollection c2)
            {
                if(c1 == null)
                {
                    return c2 == null;
                }
                if(c2 == null)
                {
                    return false;
                }
                if(!c1.GetType().Equals(c2.GetType()))
                {
                    return false;
                }

                if(c1.Count != c2.Count)
                {
                    return false;
                }

                IEnumerator i1 = c1.GetEnumerator();
                IEnumerator i2 = c2.GetEnumerator();
                while(i1.MoveNext())
                {
                    i2.MoveNext();
                    if(i1.Current is ICollection)
                    {
                        Debug.Assert(i2.Current is ICollection);
                        if(!Compare((ICollection)i1.Current,(ICollection)i2.Current))
                        {
                            return false;
                        }
                    }
                    else if(!i1.Current.Equals(i2.Current))
                    {
                        return false;
                    }
                }
                return true;
            }

            static public int allTests(global::Test.TestHelper helper)
            {
                var communicator = helper.communicator();
                var output = helper.getWriter();
                output.Write("testing serialization... ");
                output.Flush();

                var proxy = Test.MyInterfacePrxHelper.uncheckedCast(communicator.stringToProxy("test"));

                Test.RefStruct rs, rs2;
                rs = new Test.RefStruct();
                rs.s = "RefStruct";
                rs.sp = "prop";
                rs.c = null;
                rs.p = Test.MyInterfacePrxHelper.uncheckedCast(communicator.stringToProxy("test"));
                rs.seq = new Test.MyInterfacePrx[] { rs.p };
                rs2 = inOut(rs, communicator);
                test(rs.Equals(rs2));

                Test.Base b, b2;
                b = new Test.Base(true, 1, 2, 3, 4, Test.MyEnum.enum2);
                b2 = inOut(b, communicator);
                test(b2.bo == b.bo);
                test(b2.by == b.by);
                test(b2.sh == b.sh);
                test(b2.i == b.i);
                test(b2.l == b.l);
                test(b2.e == b.e);

                Test.MyClass c, c2;
                c = new Test.MyClass(true, 1, 2, 3, 4, Test.MyEnum.enum1, null, null, new Test.ValStruct(true, 1, 2, 3, 4, Test.MyEnum.enum2));
                c.c = c;
                c.o = c;
                c2 = inOut(c, communicator);
                test(c2.bo == c.bo);
                test(c2.by == c.by);
                test(c2.sh == c.sh);
                test(c2.i == c.i);
                test(c2.l == c.l);
                test(c2.e == c.e);
                test(c2.c == c2);
                test(c2.o == c2);
                test(c2.s.Equals(c.s));

                output.WriteLine("ok");
                return 0;
            }

            private static T inOut<T>(T o, Ice.Communicator communicator)
            {
#if NET8_0_OR_GREATER
                // .NET 8.0 and greater removed support for BinaryFormatter
                // See also #1549.
                return o;
#else
                BinaryFormatter bin = new BinaryFormatter(null,
                    new StreamingContext(StreamingContextStates.All, communicator));
                using(MemoryStream mem = new MemoryStream())
                {
#pragma warning disable SYSLIB0011 // Type or member is obsolete
                    bin.Serialize(mem, o);
                    mem.Seek(0, 0);
                    return(T)bin.Deserialize(mem);
#pragma warning restore SYSLIB0011 // Type or member is obsolete
                }
#endif
            }
        }
    }
}
