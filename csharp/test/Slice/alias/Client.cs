// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Slice.Test.Alias
{
    public class Client : TestHelper
    {
        // Workaround for nullable reference types.
        // C# doesn't distinguish between nullable and non-nullable reference types, except as semantic sugar.
        // Under the hood, both are mapped to the non-nullable type. So to tell if a reference type is nullable, we have
        // to check if a `NullableContext` attribute is set on it, and that it has a value of `2`.
        private static bool hasNullableContextAttribute(Type type)
        {
            var attribute = type.CustomAttributes.FirstOrDefault(
                x => x.AttributeType.FullName == "System.Runtime.CompilerServices.NullableContextAttribute");

            if (attribute != null)
            {
                var args = attribute.ConstructorArguments;
                return args.Count == 1 && args[0].ArgumentType == typeof(byte) && (byte)(args[0].Value!) == 2;
            }
            return false;
        }

        public override Task RunAsync(string[] args)
        {
            Type C2 = typeof(Class2);

            Console.Out.Write("testing aliases of builtin types... ");
            Console.Out.Flush();
            {
                Assert(C2.GetField("BoolAMember")!.FieldType == typeof(bool));
                Assert(C2.GetField("ByteAMember")!.FieldType == typeof(byte));
                Assert(C2.GetField("ShortAMember")!.FieldType == typeof(short));
                Assert(C2.GetField("UshortAMember")!.FieldType == typeof(ushort));
                Assert(C2.GetField("IntAMember")!.FieldType == typeof(int));
                Assert(C2.GetField("UintAMember")!.FieldType == typeof(uint));
                Assert(C2.GetField("VarintAMember")!.FieldType == typeof(int));
                Assert(C2.GetField("VaruintAMember")!.FieldType == typeof(uint));
                Assert(C2.GetField("LongAMember")!.FieldType == typeof(long));
                Assert(C2.GetField("UlongAMember")!.FieldType == typeof(ulong));
                Assert(C2.GetField("VarlongAMember")!.FieldType == typeof(long));
                Assert(C2.GetField("VarulongAMember")!.FieldType == typeof(ulong));
                Assert(C2.GetField("FloatAMember")!.FieldType == typeof(float));
                Assert(C2.GetField("DoubleAMember")!.FieldType == typeof(double));
                Assert(C2.GetField("StringAMember")!.FieldType == typeof(string));
                Assert(C2.GetField("Struct1AMember")!.FieldType == typeof(Struct1));
                Assert(C2.GetField("Class1AMember")!.FieldType == typeof(Class1));
                Assert(C2.GetField("Interface1AMember")!.FieldType == typeof(IInterface1Prx));
                Assert(C2.GetField("Enum1AMember")!.FieldType == typeof(Enum1));
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing sequences with aliases... ");
            Console.Out.Flush();
            {
                Assert(C2.GetField("StringASeqMember")!.FieldType == typeof(string[]));
                Assert(C2.GetField("OptLongASeqMember")!.FieldType == typeof(long?[]));
                Assert(C2.GetField("StructASeqMember")!.FieldType == typeof(Struct1[]));
                Assert(C2.GetField("InterfaceASeqMember")!.FieldType == typeof(IInterface1Prx[]));
                Assert(C2.GetField("StringSeqAMember")!.FieldType == typeof(string[]));
                Assert(C2.GetField("OptLongSeqAMember")!.FieldType == typeof(long?[]));
                Assert(C2.GetField("StructSeqAMember")!.FieldType == typeof(Struct1[]));
                Assert(C2.GetField("InterfaceSeqAMember")!.FieldType == typeof(IInterface1Prx[]));
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing dictionaries with aliases... ");
            Console.Out.Flush();
            {
                Assert(C2.GetField("StringAIntADictMember")!.FieldType == typeof(Dictionary<string, int>));
                Assert(C2.GetField("BoolAOptLongADictMember")!.FieldType == typeof(Dictionary<bool, long?>));
                Assert(C2.GetField("UshortAOptInterfaceADictMember")!.FieldType == typeof(Dictionary<ushort, IInterface1Prx?>));
                Assert(C2.GetField("EnumAVarlongADictMember")!.FieldType == typeof(Dictionary<Enum1, long>));
                Assert(C2.GetField("StringIntDictAMember")!.FieldType == typeof(Dictionary<string, int>));
                Assert(C2.GetField("BoolOptLongDictAMember")!.FieldType == typeof(Dictionary<bool, long?>));
                Assert(C2.GetField("UshortOptInterfaceDictAMember")!.FieldType == typeof(Dictionary<ushort, IInterface1Prx?>));
                Assert(C2.GetField("EnumVarlongDictAMember")!.FieldType == typeof(Dictionary<Enum1, long>));
            }
            Console.Out.WriteLine("ok");

            // TODO uncomment after anonymous sequences.
            // Console.Out.Write("testing aliases with metadata... ");
            // Console.Out.Flush();
            // {
            //     Assert(C2.GetField("StringListAMember")!.FieldType == typeof(List<string>));
            //     Assert(C2.GetField("StringQueueAMember")!.FieldType == typeof(Queue<string>));
            //     Assert(C2.GetField("StringStackAMember")!.FieldType == typeof(Stack<string>));
            //     Assert(C2.GetField("StringListA2Member")!.FieldType == typeof(List<string>));
            //     Assert(C2.GetField("StringLinkedListOverrideAMember")!.FieldType == typeof(LinkedList<string>));
            //     Assert(C2.GetField("StringStackOverrideMember")!.FieldType == typeof(Stack<string>));
            // }
            // Console.Out.WriteLine("ok");

            Console.Out.Write("testing optionals of aliases... ");
            Console.Out.Flush();
            {
                Assert(C2.GetField("OptBoolAMember")!.FieldType == typeof(bool?));
                Assert(C2.GetField("OptUshortAMember")!.FieldType == typeof(ushort?));
                Assert(C2.GetField("TaggedStructSeqAMember")!.FieldType == typeof(Struct1?[]));
                Assert(C2.GetField("TaggedEnumAVarlongADictMember")!.FieldType == typeof(Dictionary<Enum1, long>));
                // TODO uncomment after anonymous sequences.
                // Assert(C2.GetField("TaggedStringStackAMember")!.FieldType == typeof(Stack<string?>));

                Type OptStringAMemberType = C2.GetField("OptStringAMember")!.FieldType;
                Assert(OptStringAMemberType == typeof(string));
                Assert(hasNullableContextAttribute(OptStringAMemberType));
                Type OptClass1AMemberType = C2.GetField("OptClass1AMember")!.FieldType;
                Assert(OptClass1AMemberType == typeof(Class1));
                Assert(hasNullableContextAttribute(OptClass1AMemberType));
                Type TaggedInterface1AMemberType = C2.GetField("TaggedInterface1AMember")!.FieldType;
                Assert(TaggedInterface1AMemberType == typeof(IInterface1Prx));
                Assert(hasNullableContextAttribute(TaggedInterface1AMemberType));
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing inheritance from aliases... ");
            Console.Out.Flush();
            {
                Assert(typeof(Class1).BaseType == typeof(ZeroC.Ice.AnyClass));
                Assert(typeof(Class2).BaseType == typeof(Class1));

                Type[] interface1Bases = typeof(IInterface1).GetInterfaces();
                Assert(interface1Bases[0] == typeof(ZeroC.Ice.IObject));
                Assert(interface1Bases.Length == 1);

                Type[] interface1AsyncBases = typeof(IAsyncInterface1).GetInterfaces();
                Assert(interface1AsyncBases[0] == typeof(ZeroC.Ice.IObject));
                Assert(interface1AsyncBases.Length == 1);

                Type[] interface1PrxBases = typeof(IInterface1Prx).GetInterfaces();
                Assert(interface1PrxBases[0] == typeof(ZeroC.Ice.IObjectPrx));
                Assert(interface1PrxBases.Length == 1);

                Type[] interface2Bases = typeof(IInterface2).GetInterfaces();
                Assert(interface2Bases[0] == typeof(IInterface1));
                Assert(interface2Bases[1] == typeof(ZeroC.Ice.IObject));
                Assert(interface2Bases.Length == 2);

                Type[] interface2AsyncBases = typeof(IAsyncInterface2).GetInterfaces();
                Assert(interface2AsyncBases[0] == typeof(IAsyncInterface1));
                Assert(interface2AsyncBases[1] == typeof(ZeroC.Ice.IObject));
                Assert(interface2AsyncBases.Length == 2);

                Type[] interface2PrxBases = typeof(IInterface2Prx).GetInterfaces();
                Assert(interface1PrxBases[0] == typeof(IInterface1Prx));
                Assert(interface1PrxBases[1] == typeof(ZeroC.Ice.IObjectPrx));
                Assert(interface2PrxBases.Length == 2);

                Assert(Enum.GetUnderlyingType(typeof(Enum1)) == typeof(int));
                Assert(Enum.GetUnderlyingType(typeof(Enum2)) == typeof(int));
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing constants of aliases... ");
            Console.Out.Flush();
            {
                Assert(Constants.const1.GetType() == typeof(int));
                Assert(Constants.const1 == 79);
            }
            Console.Out.WriteLine("ok");

            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
