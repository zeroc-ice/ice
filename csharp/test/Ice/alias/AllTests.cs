// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Alias
{
    public static class AllTests
    {
        // Workaround for nullable reference types.
        // C# doesn't distinguish between nullable and non-nullable reference types, except as semantic sugar.
        // Under the hood, both are mapped to the non-nullable type. So to tell if a reference type is nullable, we have
        // to check for a `Nullable` or `NullableContext` attribute set on it, and that they have a value of `2`.
        private static bool HasNullableContextAttribute(ICustomAttributeProvider member, int index = 0)
        {
            var context = (Attribute?)member.GetCustomAttributes(true).FirstOrDefault(
                x => ((Attribute)x).GetType().FullName == "System.Runtime.CompilerServices.NullableContextAttribute");
            if (context != null)
            {
                FieldInfo flags = context.GetType().GetField("NullableFlags")!;
                if (((byte)flags.GetValue(context)!) == 2)
                {
                    return true;
                }
            }

            var nullable = (Attribute?)member.GetCustomAttributes(false).FirstOrDefault(
                x => ((Attribute)x).GetType().FullName == "System.Runtime.CompilerServices.NullableAttribute");
            if (nullable != null)
            {
                FieldInfo flags = nullable.GetType().GetField("NullableFlags")!;
                byte[] values = (byte[])flags.GetValue(nullable)!;
                if (values.Length >= index && values[index] == 2)
                {
                    return true;
                }
            }
            return false;
        }

        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;
            var prx = IInterface2Prx.Parse(helper.GetTestProxy("test", 0), communicator);

            Console.Out.Write("testing marshaling of parameters with aliases... ");
            Console.Out.Flush();
            {
                {
                    var (p1, p2, p3) = prx.OpBool(true, false, null);
                    TestHelper.Assert(p1 == true);
                    TestHelper.Assert(p2 == false);
                    TestHelper.Assert(p3 == null);
                }
                {
                    var (p1, p2, p3) = prx.OpByte(1, byte.MinValue, byte.MaxValue);
                    TestHelper.Assert(p1 == 1);
                    TestHelper.Assert(p2 == byte.MinValue);
                    TestHelper.Assert(p3 == byte.MaxValue);
                }
                {
                    var (p1, p2, p3) = prx.OpShort(2, short.MinValue, short.MaxValue);
                    TestHelper.Assert(p1 == 2);
                    TestHelper.Assert(p2 == short.MinValue);
                    TestHelper.Assert(p3 == short.MaxValue);
                }
                {
                    var (p1, p2, p3) = prx.OpUshort(3, ushort.MinValue, ushort.MaxValue);
                    TestHelper.Assert(p1 == 3);
                    TestHelper.Assert(p2 == ushort.MinValue);
                    TestHelper.Assert(p3 == ushort.MaxValue);
                }
                {
                    var (p1, p2, p3) = prx.OpInt(4, int.MinValue, int.MaxValue);
                    TestHelper.Assert(p1 == 4);
                    TestHelper.Assert(p2 == int.MinValue);
                    TestHelper.Assert(p3 == int.MaxValue);
                }
                {
                    var (p1, p2, p3) = prx.OpUint(5, uint.MinValue, uint.MaxValue);
                    TestHelper.Assert(p1 == 5);
                    TestHelper.Assert(p2 == uint.MinValue);
                    TestHelper.Assert(p3 == uint.MaxValue);
                }
                {
                    var (p1, p2, p3) = prx.OpVarint(6, int.MinValue, int.MaxValue);
                    TestHelper.Assert(p1 == 6);
                    TestHelper.Assert(p2 == int.MinValue);
                    TestHelper.Assert(p3 == int.MaxValue);
                }
                {
                    var (p1, p2, p3) = prx.OpVaruint(7, uint.MinValue, uint.MaxValue);
                    TestHelper.Assert(p1 == 7);
                    TestHelper.Assert(p2 == uint.MinValue);
                    TestHelper.Assert(p3 == uint.MaxValue);
                }
                {
                    var (p1, p2, p3) = prx.OpLong(8L, long.MinValue, long.MaxValue);
                    TestHelper.Assert(p1 == 8L );
                    TestHelper.Assert(p2 == long.MinValue);
                    TestHelper.Assert(p3 == long.MaxValue);
                }
                {
                    var (p1, p2, p3) = prx.OpUlong(9L, ulong.MinValue, ulong.MaxValue);
                    TestHelper.Assert(p1 == 9L);
                    TestHelper.Assert(p2 == ulong.MinValue);
                    TestHelper.Assert(p3 == ulong.MaxValue);
                }
                {
                    var (p1, p2, p3) = prx.OpVarlong(10L, -11L, null);
                    TestHelper.Assert(p1 == 10L);
                    TestHelper.Assert(p2 == -11L);
                    TestHelper.Assert(p3 == null);
                }
                {
                    var (p1, p2, p3) = prx.OpVarulong(12L, 13L, null);
                    TestHelper.Assert(p1 == 12L);
                    TestHelper.Assert(p2 == 13L);
                    TestHelper.Assert(p3 == null);
                }
                {
                    var (p1, p2, p3) = prx.OpFloat(1.0F, float.MinValue, float.MaxValue);
                    TestHelper.Assert(p1 == 1.0F);
                    TestHelper.Assert(p2 == float.MinValue);
                    TestHelper.Assert(p3 == float.MaxValue);
                }
                {
                    var (p1, p2, p3) = prx.OpDouble(2.0, double.MinValue, double.MaxValue);
                    TestHelper.Assert(p1 == 2.0);
                    TestHelper.Assert(p2 == double.MinValue);
                    TestHelper.Assert(p3 == double.MaxValue);
                }
                {
                    var (p1, p2, p3) = prx.OpString("", "hello", "there");
                    TestHelper.Assert(p1.Length == 0);
                    TestHelper.Assert(p2.Equals("hello"));
                    TestHelper.Assert(p3!.Equals("there"));
                }
                {
                    var s1 = new Struct1(-1, "foo");
                    var s2 = new Struct1(79, "bar");
                    var (p1, p2, p3) = prx.OpStruct1(s1, s2, null);
                    TestHelper.Assert(p1.I == -1);
                    TestHelper.Assert(p1.S!.Equals("foo"));
                    TestHelper.Assert(p2.I == 79);
                    TestHelper.Assert(p2.S!.Equals("bar"));
                    TestHelper.Assert(p3 == null);
                }
                {
                    var c1 = new Class1(-1, "foo");
                    var c2 = new Class1(79, "bar");
                    var (p1, p2, p3) = prx.OpClass1(c1, c2, null);
                    TestHelper.Assert(p1.I == -1);
                    TestHelper.Assert(p1.S!.Equals("foo"));
                    TestHelper.Assert(p2.I == 79);
                    TestHelper.Assert(p2.S!.Equals("bar"));
                    TestHelper.Assert(p3 == null);
                }
                {
                    var (p1, p2, p3) = prx.OpEnum1(Enum1.Thing1, Enum1.Thing2, null);
                    TestHelper.Assert(p1 == Enum1.Thing1);
                    TestHelper.Assert(p2 == Enum1.Thing2);
                    TestHelper.Assert(p3 == null);
                }
                {
                    string[] ss1 = {"abc", "de", "fghi"};
                    string[] ss2 = {"hello", "world"};
                    string[] ss3 = {"1", "22", ""};

                    var (p1, p2, p3, p4) = prx.OpStringSeq(ss1, ss2, ss3, null);
                    TestHelper.Assert(p1.Length == 3);
                    TestHelper.Assert(p1[0].Equals("abc"));
                    TestHelper.Assert(p1[1].Equals("de"));
                    TestHelper.Assert(p1[2].Equals("fghi"));
                    TestHelper.Assert(p2.Length == 2);
                    TestHelper.Assert(p2[0].Equals("hello"));
                    TestHelper.Assert(p2[1].Equals("world"));
                    TestHelper.Assert(p3.Length == 3);
                    TestHelper.Assert(p3[0].Equals("1"));
                    TestHelper.Assert(p3[1].Equals("22"));
                    TestHelper.Assert(p3[2].Length == 0);
                    TestHelper.Assert(p4 == null);
                }
                {
                    var sid1 = new Dictionary<string, int>
                    {
                        ["a"] = 1,
                        ["b"] = 2,
                        ["c"] = 3
                    };
                    var sid2 = new Dictionary<string, int>
                    {
                        ["hello"] = 5,
                        ["world"] = 50
                    };
                    var sid3 = new Dictionary<string, int>
                    {
                        [""] = 0,
                        ["foo"] = 3
                    };

                    var (p1, p2, p3, p4) = prx.OpStringIntDict(sid1, sid2, sid3, null);
                    TestHelper.Assert(p1.DictionaryEqual(sid1));
                    TestHelper.Assert(p2.DictionaryEqual(sid2));
                    TestHelper.Assert(p3.DictionaryEqual(sid3));
                    TestHelper.Assert(p4 == null);
                }
            }
            Console.Out.WriteLine("ok");

            Type C2 = typeof(Class2);

            Console.Out.Write("testing mapping of builtin type aliases... ");
            Console.Out.Flush();
            {
                TestHelper.Assert(C2.GetField("BoolAMember")!.FieldType == typeof(bool));
                TestHelper.Assert(C2.GetField("ByteAMember")!.FieldType == typeof(byte));
                TestHelper.Assert(C2.GetField("ShortAMember")!.FieldType == typeof(short));
                TestHelper.Assert(C2.GetField("UshortAMember")!.FieldType == typeof(ushort));
                TestHelper.Assert(C2.GetField("IntAMember")!.FieldType == typeof(int));
                TestHelper.Assert(C2.GetField("UintAMember")!.FieldType == typeof(uint));
                TestHelper.Assert(C2.GetField("VarintAMember")!.FieldType == typeof(int));
                TestHelper.Assert(C2.GetField("VaruintAMember")!.FieldType == typeof(uint));
                TestHelper.Assert(C2.GetField("LongAMember")!.FieldType == typeof(long));
                TestHelper.Assert(C2.GetField("UlongAMember")!.FieldType == typeof(ulong));
                TestHelper.Assert(C2.GetField("VarlongAMember")!.FieldType == typeof(long));
                TestHelper.Assert(C2.GetField("VarulongAMember")!.FieldType == typeof(ulong));
                TestHelper.Assert(C2.GetField("FloatAMember")!.FieldType == typeof(float));
                TestHelper.Assert(C2.GetField("DoubleAMember")!.FieldType == typeof(double));
                TestHelper.Assert(C2.GetField("StringAMember")!.FieldType == typeof(string));
                TestHelper.Assert(C2.GetField("Struct1AMember")!.FieldType == typeof(Struct1));
                TestHelper.Assert(C2.GetField("Class1AMember")!.FieldType == typeof(Class1));
                TestHelper.Assert(C2.GetField("Interface1AMember")!.FieldType == typeof(IInterface1Prx));
                TestHelper.Assert(C2.GetField("Enum1AMember")!.FieldType == typeof(Enum1));
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing mapping of sequence type aliases... ");
            Console.Out.Flush();
            {
                TestHelper.Assert(C2.GetField("StringASeqMember")!.FieldType == typeof(string[]));
                TestHelper.Assert(C2.GetField("OptLongASeqMember")!.FieldType == typeof(long?[]));
                TestHelper.Assert(C2.GetField("StructASeqMember")!.FieldType == typeof(Struct1[]));
                TestHelper.Assert(C2.GetField("InterfaceASeqMember")!.FieldType == typeof(IInterface1Prx[]));
                TestHelper.Assert(C2.GetField("StringSeqAMember")!.FieldType == typeof(string[]));
                TestHelper.Assert(C2.GetField("OptLongSeqAMember")!.FieldType == typeof(long?[]));
                TestHelper.Assert(C2.GetField("StructSeqAMember")!.FieldType == typeof(Struct1[]));
                TestHelper.Assert(C2.GetField("InterfaceSeqAMember")!.FieldType == typeof(IInterface1Prx[]));
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing mapping of dictionary type aliases... ");
            Console.Out.Flush();
            {
                TestHelper.Assert(C2.GetField("StringAIntADictMember")!.FieldType == typeof(Dictionary<string, int>));
                TestHelper.Assert(C2.GetField("BoolAOptLongADictMember")!.FieldType == typeof(Dictionary<bool, long?>));
                TestHelper.Assert(C2.GetField("UshortAOptInterfaceADictMember")!.FieldType == typeof(Dictionary<ushort, IInterface1Prx?>));
                TestHelper.Assert(C2.GetField("EnumAVarlongADictMember")!.FieldType == typeof(Dictionary<Enum1, long>));
                TestHelper.Assert(C2.GetField("StringIntDictAMember")!.FieldType == typeof(Dictionary<string, int>));
                TestHelper.Assert(C2.GetField("BoolOptLongDictAMember")!.FieldType == typeof(Dictionary<bool, long?>));
                TestHelper.Assert(C2.GetField("UshortOptInterfaceDictAMember")!.FieldType == typeof(Dictionary<ushort, IInterface1Prx?>));
                TestHelper.Assert(C2.GetField("EnumVarlongDictAMember")!.FieldType == typeof(Dictionary<Enum1, long>));
            }
            Console.Out.WriteLine("ok");

            // TODO uncomment after anonymous sequences.
            // Console.Out.Write("testing aliases with metadata... ");
            // Console.Out.Flush();
            // {
            //     TestHelper.Assert(C2.GetField("StringListAMember")!.FieldType == typeof(List<string>));
            //     TestHelper.Assert(C2.GetField("StringQueueAMember")!.FieldType == typeof(Queue<string>));
            //     TestHelper.Assert(C2.GetField("StringStackAMember")!.FieldType == typeof(Stack<string>));
            //     TestHelper.Assert(C2.GetField("StringListA2Member")!.FieldType == typeof(List<string>));
            //     TestHelper.Assert(C2.GetField("StringLinkedListOverrideAMember")!.FieldType == typeof(LinkedList<string>));
            //     TestHelper.Assert(C2.GetField("StringStackOverrideMember")!.FieldType == typeof(Stack<string>));
            // }
            // Console.Out.WriteLine("ok");

            Console.Out.Write("testing mapping of optional type aliases... ");
            Console.Out.Flush();
            {
                TestHelper.Assert(C2.GetField("OptBoolAMember")!.FieldType == typeof(bool?));
                TestHelper.Assert(C2.GetField("OptUshortAMember")!.FieldType == typeof(ushort?));
                // TODO uncomment after anonymous sequences.
                // TestHelper.Assert(C2.GetField("TaggedStringStackAMember")!.FieldType == typeof(Stack<string?>));

                FieldInfo OptStringAMemberField = C2.GetField("OptStringAMember")!;
                TestHelper.Assert(OptStringAMemberField.FieldType == typeof(string));
                TestHelper.Assert(HasNullableContextAttribute(OptStringAMemberField));

                FieldInfo OptClass1AMemberField = C2.GetField("OptClass1AMember")!;
                TestHelper.Assert(OptClass1AMemberField.FieldType == typeof(Class1));
                TestHelper.Assert(HasNullableContextAttribute(OptClass1AMemberField));

                FieldInfo TaggedInterface1AMemberField = C2.GetField("TaggedInterface1AMember")!;
                TestHelper.Assert(TaggedInterface1AMemberField.FieldType == typeof(IInterface1Prx));
                TestHelper.Assert(HasNullableContextAttribute(TaggedInterface1AMemberField));

                FieldInfo TaggedStructSeqAMemberField = C2.GetField("TaggedStructSeqAMember")!;
                TestHelper.Assert(TaggedStructSeqAMemberField.FieldType == typeof(Struct1[]));
                TestHelper.Assert(HasNullableContextAttribute(TaggedStructSeqAMemberField));

                FieldInfo TaggedEnumAVarlongADictMemberField = C2.GetField("TaggedEnumAVarlongADictMember")!;
                TestHelper.Assert(TaggedEnumAVarlongADictMemberField.FieldType == typeof(Dictionary<Enum1, long>));
                TestHelper.Assert(HasNullableContextAttribute(TaggedEnumAVarlongADictMemberField));
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing inheritance from aliases... ");
            Console.Out.Flush();
            {
                TestHelper.Assert(typeof(Class1).BaseType == typeof(AnyClass));
                TestHelper.Assert(typeof(Class2).BaseType == typeof(Class1));

                Type[] interface1Bases = typeof(IInterface1).GetInterfaces();
                TestHelper.Assert(interface1Bases.Length == 1);
                TestHelper.Assert(interface1Bases[0] == typeof(IObject));

                Type[] interface1AsyncBases = typeof(IAsyncInterface1).GetInterfaces();
                TestHelper.Assert(interface1AsyncBases.Length == 1);
                TestHelper.Assert(interface1AsyncBases[0] == typeof(IObject));

                Type[] interface1PrxBases = typeof(IInterface1Prx).GetInterfaces();
                TestHelper.Assert(interface1PrxBases.Length == 2);
                TestHelper.Assert(interface1PrxBases[0] == typeof(IObjectPrx));
                TestHelper.Assert(interface1PrxBases[1] == typeof(IEquatable<IObjectPrx>));

                Type[] interface2Bases = typeof(IInterface2).GetInterfaces();
                TestHelper.Assert(interface2Bases.Length == 2);
                TestHelper.Assert(interface2Bases[0] == typeof(IInterface1));
                TestHelper.Assert(interface2Bases[1] == typeof(IObject));

                Type[] interface2AsyncBases = typeof(IAsyncInterface2).GetInterfaces();
                TestHelper.Assert(interface2AsyncBases.Length == 2);
                TestHelper.Assert(interface2AsyncBases[0] == typeof(IAsyncInterface1));
                TestHelper.Assert(interface2AsyncBases[1] == typeof(IObject));

                Type[] interface2PrxBases = typeof(IInterface2Prx).GetInterfaces();
                TestHelper.Assert(interface2PrxBases.Length == 3);
                TestHelper.Assert(interface2PrxBases[0] == typeof(IInterface1Prx));
                TestHelper.Assert(interface2PrxBases[1] == typeof(IObjectPrx));
                TestHelper.Assert(interface2PrxBases[2] == typeof(IEquatable<IObjectPrx>));

                TestHelper.Assert(Enum.GetUnderlyingType(typeof(Enum1)) == typeof(int));
                TestHelper.Assert(Enum.GetUnderlyingType(typeof(Enum2)) == typeof(int));
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing constants with aliases... ");
            Console.Out.Flush();
            {
                TestHelper.Assert(Constants.const1.GetType() == typeof(int));
                TestHelper.Assert(Constants.const1 == 79);
            }
            Console.Out.WriteLine("ok");

            await prx.ShutdownAsync();
        }
    }
}
