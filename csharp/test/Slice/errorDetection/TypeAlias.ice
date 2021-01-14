//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{
    sequence<float> floatSeq;

    struct Custom
    {
        int i;
    }

    class C0 {}
    interface I0 {}
    exception E0 {}

    module Nested1
    {
        class C1;
    }

    module Nested2
    {
        class C2;
    }

    // Basic type aliases
    using intAlias = int;               //ok
    using stringAlias = string;         //ok
    using floatSeqAlias = floatSeq;     //ok
    using customAlias = Custom;         //ok
    using unknownAlias = foo;           //can't resolve type
    using optintAlias = int?;           //can't typealias optional types
    using missingType;                  //missing underlying type
    using aliasName = int               //missing semicolon

    using C0Alias = C0;                      //ok
    using C1Alias = C1;                      //can't resolve type
    using scopedC0Alias = Test::C0;          //ok
    using scopedC1Alias = Test::Nested1::C1; //ok

    // Type resolution of type-aliases
    module Nested2
    {
        using C0Alias = C0;                      //ok
        using C1Alias = C1;                      //can't resolve type
        using C2Alias = C2;                      //ok
        using scopedC0Alias = Test::C0;          //ok
        using scopedC1Alias = Test::Nested1::C1; //ok
        using scopedC2Alias = Test::Nested2::C2; //ok
    }

    // Inheritance from type-aliases
    // Note, all metadata is allowed, but non-type metadata will error out when the alias is used.
    using C0Alias = C0;                            //redefinition error
    using C0AliasMetadata = [protected] C0;        //illegal metadata
    class DerivedClass1 : C0Alias {}               //ok
    class DerivedClass2 : C0AliasMetadata {}       //illegal metadata

    using I0Alias = I0;                                //ok
    using I0AliasMetadata = [amd] I0;                  //illegal metadata
    interface DerivedInterface1 : I0Alias {}           //ok
    interface DerivedInterface2 : I0AliasMetadata {}   //illegal metadata

    using E0Alias = E0;         //exceptions cannot be used as types

    struct TestStruct
    {
        intAlias i1;            //ok
        stringAlias? s1;        //ok
        C0AliasMetadata c1;     //illegal metadata (TODO when we introduce better validation)
    }

    interface TestInterface
    {
        intAlias op1(stringAlias s1, intAlias? i1);    //ok
        (floatSeqAlias fs1, C0AliasMetadata c1) op2(); //illegal metadata (TODO when we introduce better validation)
    }
}
