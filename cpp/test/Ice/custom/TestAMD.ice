// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:include:deque", "cpp:include:list", "cpp:include:MyByteSeq.h", "cpp:include:CustomMap.h",
"cpp:include:CustomBuffer.h"]]

module Test
{
    sequence<bool> BoolSeq;
    ["cpp:type:std::list<bool>"] sequence<bool> BoolList;

    ["cpp:type:std::list<::Test::BoolList>"] sequence<BoolList> BoolListList;
    sequence<BoolList> BoolListSeq;
    ["cpp:type:std::list<::Test::BoolSeq>"] sequence<BoolSeq> BoolSeqList;

    sequence<byte> ByteSeq;
    ["cpp:type:std::list<std::byte>"] sequence<byte> ByteList;

    ["cpp:type:std::list<::Test::ByteList>"] sequence<ByteList> ByteListList;
    sequence<ByteList> ByteListSeq;
    ["cpp:type:std::list<::Test::ByteSeq>"] sequence<ByteSeq> ByteSeqList;

    sequence<string> StringSeq;
    ["cpp:type:std::list<std::string>"] sequence<string> StringList;

    ["cpp:type:std::list<::Test::StringList>"] sequence<StringList> StringListList;
    sequence<StringList> StringListSeq;
    ["cpp:type:std::list<::Test::StringSeq>"] sequence<StringSeq> StringSeqList;

    struct Fixed
    {
        short s;
    }

    sequence<Fixed> FixedSeq;
    ["cpp:type:std::list<::Test::Fixed>"] sequence<Fixed> FixedList;

    ["cpp:type:std::list<::Test::FixedList>"] sequence<FixedList> FixedListList;
    sequence<FixedList> FixedListSeq;
    ["cpp:type:std::list<::Test::FixedSeq>"] sequence<FixedSeq> FixedSeqList;

    struct Variable
    {
        string s;
        BoolList bl;
        ["cpp:type:std::list<std::string>"] StringSeq ss;
    }

    sequence<Variable> VariableSeq;
    ["cpp:type:std::list<::Test::Variable>"] sequence<Variable> VariableList;

    ["cpp:type:std::list<::Test::VariableList>"] sequence<VariableList> VariableListList;
    sequence<VariableList> VariableListSeq;
    ["cpp:type:std::list<::Test::VariableSeq>"] sequence<VariableSeq> VariableSeqList;

    dictionary<string, string> StringStringDict;
    sequence<StringStringDict> StringStringDictSeq;
    ["cpp:type:std::list<::Test::StringStringDict>"] sequence<StringStringDict> StringStringDictList;

    ["cpp:type:std::list<::Test::StringStringDictList>"] sequence<StringStringDictList> StringStringDictListList;
    sequence<StringStringDictList> StringStringDictListSeq;
    ["cpp:type:std::list<::Test::StringStringDictSeq>"] sequence<StringStringDictSeq> StringStringDictSeqList;

    enum E { E1, E2, E3 }
    sequence<E> ESeq;
    ["cpp:type:std::list<::Test::E>"] sequence<E> EList;

    ["cpp:type:std::list<::Test::EList>"] sequence<EList> EListList;
    sequence<EList> EListSeq;
    ["cpp:type:std::list<::Test::ESeq>"] sequence<ESeq> ESeqList;

    class C {}
    sequence<C> CSeq;
    ["cpp:type:std::list<std::shared_ptr<::Test::C>>"] sequence<C> CList;

    ["cpp:type:std::list<::Test::CList>"] sequence<CList> CListList;
    sequence<CList> CListSeq;
    ["cpp:type:std::list<::Test::CSeq>"] sequence<CSeq> CSeqList;

    interface D{}
    sequence<D*> DPrxSeq;
    ["cpp:type:std::list<std::optional<::Test::DPrx>>"] sequence<D*> DPrxList;

    ["cpp:type:std::list<::Test::DPrxList>"] sequence<DPrxList> DPrxListList;
    sequence<DPrxList> DPrxListSeq;
    ["cpp:type:std::list<::Test::DPrxSeq>"] sequence<DPrxSeq> DPrxSeqList;

    sequence<double> DoubleSeq;
    sequence<short> ShortSeq;

    ["cpp:type:Test::CustomMap<std::int32_t, std::string>"] dictionary<int, string> IntStringDict;
    dictionary<long, long> LongLongDict;
    dictionary<string, int> StringIntDict;

    class DictClass
    {
        IntStringDict isdict;
    }

    ["cpp:type:Test::CustomBuffer<bool>"] sequence<bool> BoolBuffer;
    ["cpp:type:Test::CustomBuffer<std::int16_t>"] sequence<short> ShortBuffer;
    ["cpp:type:Test::CustomBuffer<std::int32_t>"] sequence<int> IntBuffer;
    ["cpp:type:Test::CustomBuffer<int64_t>"] sequence<long> LongBuffer;
    ["cpp:type:Test::CustomBuffer<float>"] sequence<float> FloatBuffer;
    ["cpp:type:Test::CustomBuffer<double>"] sequence<double> DoubleBuffer;
    ["cpp:type:Test::CustomBuffer<std::byte>"] sequence<byte> ByteBuffer;
    struct BufferStruct
    {
        ByteBuffer byteBuf;
        BoolBuffer boolBuf;
        ShortBuffer shortBuf;
        IntBuffer intBuf;
        LongBuffer longBuf;
        FloatBuffer floatBuf;
        DoubleBuffer doubleBuf;
    }

    ["amd"] interface TestIntf
    {
        ["cpp:array"] ShortSeq opShortArray(["cpp:array"] ShortSeq inSeq, ["cpp:array"] out ShortSeq outSeq);

        DoubleSeq opDoubleArray(bool padding, ["cpp:array"] DoubleSeq inSeq, out DoubleSeq outSeq);

        BoolSeq opBoolArray(["cpp:array"] BoolSeq inSeq, out BoolSeq outSeq);

        ["cpp:array"] ByteList opByteArray(["cpp:array"] ByteList inSeq, ["cpp:array"] out ByteList outSeq);

        VariableList opVariableArray(["cpp:array"] VariableList inSeq, out VariableList outSeq);

        ["cpp:type:std::deque<bool>"] BoolSeq
        opBoolSeq(["cpp:type:std::deque<bool>"] BoolSeq inSeq, ["cpp:type:std::deque<bool>"] out BoolSeq outSeq);

        BoolList opBoolList(BoolList inSeq, out BoolList outSeq);

        ["cpp:type:std::deque<std::byte>"] ByteSeq
        opByteSeq(["cpp:type:std::deque<std::byte>"] ByteSeq inSeq,
            ["cpp:type:std::deque<std::byte>"] out ByteSeq outSeq);

        ByteList opByteList(ByteList inSeq, out ByteList outSeq);

        ["cpp:type:MyByteSeq"] ByteSeq
        opMyByteSeq(["cpp:type:MyByteSeq"] ByteSeq inSeq, ["cpp:type:MyByteSeq"] out ByteSeq outSeq);

        ["cpp:type:std::deque<std::string>"] StringSeq
        opStringSeq(["cpp:type:std::deque<std::string>"] StringSeq inSeq,
            ["cpp:type:std::deque<std::string>"] out StringSeq outSeq);

        StringList opStringList(StringList inSeq, out StringList outSeq);

        ["cpp:type:std::deque<::Test::Fixed>"] FixedSeq
        opFixedSeq(["cpp:type:std::deque<::Test::Fixed>"] FixedSeq inSeq,
            ["cpp:type:std::deque<::Test::Fixed>"] out FixedSeq outSeq);

        FixedList opFixedList(FixedList inSeq, out FixedList outSeq);

        ["cpp:type:std::deque<::Test::Variable>"] VariableSeq
        opVariableSeq(["cpp:type:std::deque<::Test::Variable>"] VariableSeq inSeq,
            ["cpp:type:std::deque<::Test::Variable>"] out VariableSeq outSeq);

        VariableList opVariableList(VariableList inSeq, out VariableList outSeq);

        ["cpp:type:std::deque<::Test::StringStringDict>"] StringStringDictSeq
        opStringStringDictSeq(["cpp:type:std::deque<::Test::StringStringDict>"] StringStringDictSeq inSeq,
            ["cpp:type:std::deque<::Test::StringStringDict>"] out StringStringDictSeq outSeq);

        StringStringDictList opStringStringDictList(StringStringDictList inSeq, out StringStringDictList outSeq);

        ["cpp:type:std::deque<::Test::E>"] ESeq
        opESeq(["cpp:type:std::deque<::Test::E>"] ESeq inSeq, ["cpp:type:std::deque<::Test::E>"] out ESeq outSeq);

        EList opEList(EList inSeq, out EList outSeq);

        ["cpp:type:std::deque<std::optional<::Test::DPrx>>"] DPrxSeq
        opDPrxSeq(["cpp:type:std::deque<std::optional<::Test::DPrx>>"] DPrxSeq inSeq,
            ["cpp:type:std::deque<std::optional<::Test::DPrx>>"] out DPrxSeq outSeq);

        DPrxList opDPrxList(DPrxList inSeq, out DPrxList outSeq);

        ["cpp:type:std::deque<std::shared_ptr<Test::C>>"] CSeq
        opCSeq(["cpp:type:std::deque<std::shared_ptr<Test::C>>"] CSeq inSeq,
            ["cpp:type:std::deque<std::shared_ptr<Test::C>>"] out CSeq outSeq);

        CList opCList(CList inSeq, out CList outSeq);

        void opOutArrayByteSeq(ByteSeq org, ["cpp:array"] out ByteSeq copy);

        IntStringDict opIntStringDict(IntStringDict idict, out IntStringDict odict);

        ["cpp:type:::Test::CustomMap< int64_t, int64_t>"] LongLongDict
        opVarDict(["cpp:type:::Test::CustomMap<std::string, std::int32_t>"] StringIntDict idict,
            ["cpp:type:::Test::CustomMap<std::string, std::int32_t>"] out StringIntDict odict);

        ShortBuffer opShortBuffer(ShortBuffer inS, out ShortBuffer outS);

        ["cpp:type:::Test::CustomBuffer<bool>"] BoolSeq opBoolBuffer(
            ["cpp:type:::Test::CustomBuffer<bool>"] BoolSeq inS,
            ["cpp:type:::Test::CustomBuffer<bool>"] out BoolSeq outS);

        BufferStruct opBufferStruct(BufferStruct s);

        void shutdown();
    }
}
