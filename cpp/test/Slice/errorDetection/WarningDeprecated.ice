// Copyright (c) ZeroC, Inc.

module Test
{
    enum NormalEnum
    {
        Red,
        ["deprecated"] Blue,
    }
    ["deprecated"] enum DeprecatedEnum
    {
        Yellow,
        ["deprecated"] Green,
    }

    ["deprecated"] const string DeprecatedStringConst = "there";
    const NormalEnum NormalEnumRedConst1 = NormalEnum::Red;
    const NormalEnum NormalEnumBlueConst1 = NormalEnum::Blue;
    const DeprecatedEnum DeprecatedEnumYellowConst1 = DeprecatedEnum::Yellow;
    const DeprecatedEnum DeprecatedEnumGreenConst1 = DeprecatedEnum::Green;
    const string DeprecatedStringConst1 = DeprecatedStringConst;
    ["deprecated"] const NormalEnum NormalEnumRedConst2 = NormalEnum::Red;
    ["deprecated"] const NormalEnum NormalEnumBlueConst2 = NormalEnum::Blue;
    ["deprecated"] const DeprecatedEnum DeprecatedEnumYellowConst2 = DeprecatedEnum::Yellow;
    ["deprecated"] const DeprecatedEnum DeprecatedEnumGreenConst2 = DeprecatedEnum::Green;
    ["deprecated"] const string DeprecatedStringConst2 = DeprecatedStringConst;

    struct NormalStruct
    {
        NormalEnum normalEnumRedField1 = NormalEnum::Red;
        NormalEnum normalEnumBlueField1 = NormalEnum::Blue;
        DeprecatedEnum deprecatedEnumYellowField1 = DeprecatedEnum::Yellow;
        DeprecatedEnum deprecatedEnumGreenField1 = DeprecatedEnum::Green;
        DeprecatedEnum deprecatedEnumField1;

        ["deprecated"] NormalEnum normalEnumRedField2 = NormalEnum::Red;
        ["deprecated"] NormalEnum normalEnumBlueField2 = NormalEnum::Blue;
        ["deprecated"] DeprecatedEnum deprecatedEnumYellowField2 = DeprecatedEnum::Yellow;
        ["deprecated"] DeprecatedEnum deprecatedEnumGreenField2 = DeprecatedEnum::Green;
        ["deprecated"] DeprecatedEnum deprecatedEnumField2;

        string normalStringField = DeprecatedStringConst;
        ["deprecated"] string deprecatedStringField = DeprecatedStringConst;
    }
    ["deprecated"] struct DeprecatedStruct
    {
        NormalEnum normalEnumRedField1 = NormalEnum::Red;
        NormalEnum normalEnumBlueField1 = NormalEnum::Blue;
        DeprecatedEnum deprecatedEnumGreenField1 = DeprecatedEnum::Green;

        ["deprecated"] NormalEnum normalEnumRedField2 = NormalEnum::Red;
        ["deprecated"] NormalEnum normalEnumBlueField2 = NormalEnum::Blue;
        ["deprecated"] DeprecatedEnum deprecatedEnumGreenField2 = DeprecatedEnum::Green;
    }

    class NormalClass {}
    ["deprecated"] class DeprecatedClass {}
    class ExtendsDeprecatedClass1 : DeprecatedClass {}
    ["deprecated"] class ExtendsDeprecatedClass2 : DeprecatedClass {}

    exception NormalException {}
    ["deprecated"] exception DeprecatedException {}
    exception ExtendsDeprecatedException1 : DeprecatedException {}
    ["deprecated"] exception ExtendsDeprecatedException2 : DeprecatedException {}

    sequence<NormalStruct> NormalSequence1;
    sequence<DeprecatedStruct> DeprecatedSequence1;
    ["deprecated"] sequence<NormalStruct> NormalSequence2;
    ["deprecated"] sequence<DeprecatedStruct> DeprecatedSequence2;

    dictionary<NormalEnum, NormalClass> NormalDictionary1;
    dictionary<DeprecatedEnum, NormalClass> DeprecatedKeyDictionary1;
    dictionary<NormalEnum, DeprecatedClass> DeprecatedValueDictionary1;
    dictionary<DeprecatedEnum, DeprecatedClass> DeprecatedBothDictionary1;
    ["deprecated"] dictionary<NormalEnum, NormalClass> NormalDictionary2;
    ["deprecated"] dictionary<DeprecatedEnum, NormalClass> DeprecatedKeyDictionary2;
    ["deprecated"] dictionary<NormalEnum, DeprecatedClass> DeprecatedValueDictionary2;
    ["deprecated"] dictionary<DeprecatedEnum, DeprecatedClass> DeprecatedBothDictionary2;

    interface NormalInterface
    {
        DeprecatedEnum normal1(DeprecatedClass dc, NormalDictionary2 nd, DeprecatedSequence1 ds)
            throws NormalException, DeprecatedException;

        ["deprecated"]
        DeprecatedEnum deprecated1(DeprecatedClass dc, NormalDictionary2 nd, DeprecatedSequence1 ds)
            throws NormalException, DeprecatedException;
    }
    ["deprecated"] interface DeprecatedInterface
    {
        DeprecatedEnum normal2(DeprecatedClass dc, NormalDictionary2 nd, DeprecatedSequence1 ds)
            throws NormalException, DeprecatedException;

        ["deprecated"]
        DeprecatedEnum deprecated2(DeprecatedClass dc, NormalDictionary2 nd, DeprecatedSequence1 ds)
            throws NormalException, DeprecatedException;
    }
    interface Extends1 : NormalInterface, DeprecatedInterface {}
    ["deprecated"] interface Extends2 : NormalInterface, DeprecatedInterface {}
}
