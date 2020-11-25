//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module ZeroC::Slice::Test::Alias
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
        void opVoid(string s);
        (int i, int? j) opIntInt();
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

    interface Interface2 : Interface1A
    {
    }

    enum Enum2 : intA
    {
        Test1 = const1
    }
}
