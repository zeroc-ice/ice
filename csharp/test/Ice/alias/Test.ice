//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::Alias
{
    struct Struct1
    {
        int i;
        string? s;
    }

    class Class1
    {
        int i;
        tag(2) string? s;
    }

    interface Interface1
    {
        void shutdown();
    }

    enum Enum1
    {
        Thing1,
        Thing2 = 5
    }

    // basic types
    using boolA = bool;
    using byteA = byte;
    using shortA = short;
    using ushortA = ushort;
    using intA = int;
    using uintA = uint;
    using varintA = varint;
    using varuintA = varuint;
    using longA = long;
    using ulongA = ulong;
    using varlongA = varlong;
    using varulongA = varulong;
    using floatA = float;
    using doubleA = double;
    using stringA = string;
    using Struct1A = Struct1;
    using Class1A = Class1;
    using Interface1A = Interface1;
    using Enum1A = Enum1;

    // aliases
    sequence<string> stringSeq;
    sequence<long?> optLongSeq;
    sequence<Struct1> structSeq;
    sequence<Interface1?> interfaceSeq;

    sequence<stringA> stringASeq;
    sequence<longA?> optLongASeq;
    sequence<Struct1A> structASeq;
    sequence<Interface1A?> interfaceASeq;

    using stringSeqA = stringSeq;
    using optLongSeqA = optLongSeq;
    using structSeqA = structSeq;
    using interfaceSeqA = interfaceSeq;

    // dictionaries
    dictionary<string, int> stringIntDict;
    dictionary<bool, long?> boolOptLongDict;
    dictionary<ushort, Interface1?> ushortOptInterfaceDict;
    dictionary<Enum1, varlong> enumVarlongDict;

    dictionary<stringA, intA> stringAIntADict;
    dictionary<boolA, longA?> boolAOptLongADict;
    dictionary<ushortA, Interface1A?> ushortAOptInterfaceADict;
    dictionary<Enum1A, varlongA> enumAVarlongADict;

    using stringIntDictA = stringIntDict;
    using boolOptLongDictA = boolOptLongDict;
    using ushortOptInterfaceDictA = ushortOptInterfaceDict;
    using enumVarlongDictA = enumVarlongDict;

    // metadata and nesting
    using stringListA = [cs:generic(List)] stringSeq;
    using stringQueueA = [cs:generic(Queue)] stringListA;
    using stringStackA = [cs:generic(Stack)] stringQueueA;
    using stringListA2 = [cs:generic(List)] stringStackA;

    // constants
    const intA const1 = 79;

    // inheritance from aliases, and members with aliases
    class Class2 : Class1A
    {
        boolA boolAMember;
        byteA byteAMember;
        shortA shortAMember;
        ushortA ushortAMember;
        intA intAMember;
        uintA uintAMember;
        varintA varintAMember;
        varuintA varuintAMember;
        longA longAMember;
        ulongA ulongAMember;
        varlongA varlongAMember;
        varulongA varulongAMember;
        floatA floatAMember;
        doubleA doubleAMember;
        stringA stringAMember;
        Struct1A Struct1AMember;
        Class1A Class1AMember;
        Interface1A Interface1AMember;
        Enum1A Enum1AMember;

        stringASeq stringASeqMember;
        optLongASeq optLongASeqMember;
        structASeq structASeqMember;
        interfaceASeq interfaceASeqMember;
        stringSeqA stringSeqAMember;
        optLongSeqA optLongSeqAMember;
        structSeqA structSeqAMember;
        interfaceSeqA interfaceSeqAMember;

        stringAIntADict stringAIntADictMember;
        boolAOptLongADict boolAOptLongADictMember;
        ushortAOptInterfaceADict ushortAOptInterfaceADictMember;
        enumAVarlongADict enumAVarlongADictMember;
        stringIntDictA stringIntDictAMember;
        boolOptLongDictA boolOptLongDictAMember;
        ushortOptInterfaceDictA ushortOptInterfaceDictAMember;
        enumVarlongDictA enumVarlongDictAMember;

        // TODO uncomment after anonymous sequences.
        // stringListA stringListAMember;
        // stringQueueA stringQueueAMember;
        // stringStackA stringStackAMember;
        // stringListA2 stringListA2Member;
        // [cs:generic(LinkedList)] stringListA stringLinkedListOverrideAMember;
        // [cs:generic(Stack)] stringQueueA stringStackOverrideMember;

        boolA? optBoolAMember;
        ushortA? optUshortAMember;
        stringA? optStringAMember;
        Class1A? optClass1AMember;
        tag(1) Interface1A? taggedInterface1AMember;
        tag(2) structSeqA? taggedStructSeqAMember;
        tag(3) enumAVarlongADict? taggedEnumAVarlongADictMember;
        // TODO uncomment after anonymous sequences.
        // tag(4) stringStackA? taggedStringStackAMember;
    }

    enum Enum2 : intA
    {
        Test1 = const1
    }

    interface Interface2 : Interface1A
    {
        (boolA r1, bool r2, tag(1) boolA? r3) opBool(bool p1, boolA p2, tag(1) boolA? p3);

        (byteA r1, byte r2, tag(1) byteA? r3) opByte(byte p1, byteA p2, tag(1) byteA? p3);

        (shortA r1, short r2, tag(1) shortA? r3) opShort(short p1, shortA p2, tag(1) shortA? p3);

        (ushortA r1, ushort r2, tag(1) ushortA? r3) opUshort(ushort p1, ushortA p2, tag(1) ushortA? p3);

        (intA r1, int r2, tag(1) intA? r3) opInt(int p1, intA p2, tag(1) intA? p3);

        (uintA r1, uint r2, tag(1) uintA? r3) opUint(uint p1, uintA p2, tag(1) uintA? p3);

        (varintA r1, varint r2, tag(1) varintA? r3) opVarint(varint p1, varintA p2, tag(1) varintA? p3);

        (varuintA r1, varuint r2, tag(1) varuintA? r3) opVaruint(varuint p1, varuintA p2, tag(1) varuintA? p3);

        (longA r1, long r2, tag(1) longA? r3) opLong(long p1, longA p2, tag(1) longA? p3);

        (ulongA r1, ulong r2, tag(1) ulongA? r3) opUlong(ulong p1, ulongA p2, tag(1) ulongA? p3);

        (varlongA r1, varlong r2, tag(1) varlongA? r3) opVarlong(varlong p1, varlongA p2, tag(1) varlongA? p3);

        (varulongA r1, varulong r2, tag(1) varulongA? r3) opVarulong(varulong p1, varulongA p2, tag(1) varulongA? p3);

        (floatA r1, float r2, tag(1) floatA? r3) opFloat(float p1, floatA p2, tag(1) floatA? p3);

        (doubleA r1, double r2, tag(1) doubleA? r3) opDouble(double p1, doubleA p2, tag(1) doubleA? p3);

        (stringA r1, string r2, tag(1) stringA? r3) opString(string p1, stringA p2, tag(1) stringA? p3);

        (Struct1A r1, Struct1 r2, tag(1) Struct1A? r3) opStruct1(Struct1 p1, Struct1A p2, tag(1) Struct1A? p3);

        (Class1A r1, Class1 r2, Class1A? r3) opClass1(Class1 p1, Class1A p2, Class1A? p3);

        (Interface1A r1, Interface1 r2, tag(1) Interface1A? r3) opInterface1(Interface1 p1, Interface1A p2, tag(1) Interface1A? p3);

        (Enum1A r1, Enum1 r2, tag(1) Enum1A? r3) opEnum1(Enum1 p1, Enum1A p2, tag(1) Enum1A? p3);

        (stringASeq r1, stringSeqA r2, stringSeq r3, tag(1) stringSeqA? r4) opStringSeq(stringSeq p1, stringASeq p2, stringSeqA p3, tag(1) stringSeqA? p4);

        (stringAIntADict r1, stringIntDictA r2, stringIntDict r3, tag(1) stringIntDictA? r4) opStringIntDict(stringIntDict p1, stringAIntADict p2, stringIntDictA p3, tag(1) stringIntDictA? p4);
    }
}
