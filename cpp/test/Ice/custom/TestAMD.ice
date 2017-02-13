// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:include:deque", "cpp:include:list", "cpp:include:MyByteSeq.h", "cpp:include:CustomMap.h",
  "cpp:include:CustomBuffer.h", "cpp:include:StringView.h"]]

module Test
{

sequence<bool> BoolSeq;
["cpp:type:std::list<bool>"] sequence<bool> BoolList;

["cpp:type:std::list< ::Test::BoolList>"] sequence<BoolList> BoolListList;
sequence<BoolList> BoolListSeq;
["cpp:type:std::list< ::Test::BoolSeq>"] sequence<BoolSeq> BoolSeqList;

sequence<byte> ByteSeq;
["cpp:type:std::list< ::Ice::Byte>"] sequence<byte> ByteList;

["cpp:type:std::list< ::Test::ByteList>"] sequence<ByteList> ByteListList;
sequence<ByteList> ByteListSeq;
["cpp:type:std::list< ::Test::ByteSeq>"] sequence<ByteSeq> ByteSeqList;

sequence<string> StringSeq;
["cpp:type:std::list<std::string>"] sequence<string> StringList;

["cpp:type:std::list< ::Test::StringList>"] sequence<StringList> StringListList;
sequence<StringList> StringListSeq;
["cpp:type:std::list< ::Test::StringSeq>"] sequence<StringSeq> StringSeqList;

struct Fixed
{
    short s;
};

sequence<Fixed> FixedSeq;
["cpp:type:std::list< ::Test::Fixed>"] sequence<Fixed> FixedList;

["cpp:type:std::list< ::Test::FixedList>"] sequence<FixedList> FixedListList;
sequence<FixedList> FixedListSeq;
["cpp:type:std::list< ::Test::FixedSeq>"] sequence<FixedSeq> FixedSeqList;

struct Variable
{
    string s;
    BoolList bl;
    ["cpp:type:std::list<std::string>"] StringSeq ss;
};

sequence<Variable> VariableSeq;
["cpp:type:std::list< ::Test::Variable>"] sequence<Variable> VariableList;

["cpp:type:std::list< ::Test::VariableList>"] sequence<VariableList> VariableListList;
sequence<VariableList> VariableListSeq;
["cpp:type:std::list< ::Test::VariableSeq>"] sequence<VariableSeq> VariableSeqList;

dictionary<string, string> StringStringDict;
sequence<StringStringDict> StringStringDictSeq;
["cpp:type:std::list< ::Test::StringStringDict>"] sequence<StringStringDict> StringStringDictList;

["cpp:type:std::list< ::Test::StringStringDictList>"] sequence<StringStringDictList> StringStringDictListList;
sequence<StringStringDictList> StringStringDictListSeq;
["cpp:type:std::list< ::Test::StringStringDictSeq>"] sequence<StringStringDictSeq> StringStringDictSeqList;

enum E { E1, E2, E3 };
sequence<E> ESeq;
["cpp:type:std::list< ::Test::E>"] sequence<E> EList;

["cpp:type:std::list< ::Test::EList>"] sequence<EList> EListList;
sequence<EList> EListSeq;
["cpp:type:std::list< ::Test::ESeq>"] sequence<ESeq> ESeqList;

class C {};
sequence<C> CSeq;
["cpp:type:std::list< ::Test::CPtr>", "cpp11:type:std::list<std::shared_ptr<::Test::C>>"] sequence<C> CList;

["cpp:type:std::list< ::Test::CList>"] sequence<CList> CListList;
sequence<CList> CListSeq;
["cpp:type:std::list< ::Test::CSeq>"] sequence<CSeq> CSeqList;

sequence<C*> CPrxSeq;
["cpp:type:std::list< ::Test::CPrx>", "cpp11:type:std::list<std::shared_ptr<Ice::ObjectPrx>>"] sequence<C*> CPrxList;

["cpp:type:std::list< ::Test::CPrxList>"] sequence<CPrxList> CPrxListList;
sequence<CPrxList> CPrxListSeq;
["cpp:type:std::list< ::Test::CPrxSeq>"] sequence<CPrxSeq> CPrxSeqList;

sequence<double> DoubleSeq;

["cpp:class"] struct ClassOtherStruct
{
    int x;
};
sequence<ClassOtherStruct> ClassOtherStructSeq;

["cpp:class"] struct ClassStruct
{
    ClassOtherStructSeq otherSeq;
    ClassOtherStruct other;
    int y;
};
sequence<ClassStruct> ClassStructSeq;

["cpp:type:Test::CustomMap<Ice::Int, std::string>"] dictionary<int, string> IntStringDict;
dictionary<long, long> LongLongDict;
dictionary<string, int> StringIntDict;

class DictClass
{
    IntStringDict isdict;
};

["cpp:type:Test::CustomBuffer<bool>"] sequence<bool> BoolBuffer;
["cpp:type:Test::CustomBuffer<Ice::Short>"] sequence<short> ShortBuffer;
["cpp:type:Test::CustomBuffer<Ice::Int>"] sequence<int> IntBuffer;
["cpp:type:Test::CustomBuffer<Ice::Long>"] sequence<long> LongBuffer;
["cpp:type:Test::CustomBuffer<Ice::Float>"] sequence<float> FloatBuffer;
["cpp:type:Test::CustomBuffer<Ice::Double>"] sequence<double> DoubleBuffer;
["cpp:type:Test::CustomBuffer<Ice::Byte>"] sequence<byte> ByteBuffer;
struct BufferStruct
{
    ByteBuffer byteBuf;
    BoolBuffer boolBuf;
    ShortBuffer shortBuf;
    IntBuffer intBuf;
    LongBuffer longBuf;
    FloatBuffer floatBuf;
    DoubleBuffer doubleBuf;
};

["amd"] interface TestIntf
{
    DoubleSeq opDoubleArray(["cpp:array"] DoubleSeq inSeq, out DoubleSeq outSeq);

    BoolSeq opBoolArray(["cpp:array"] BoolSeq inSeq, out BoolSeq outSeq);

    ["cpp:array"] ByteList opByteArray(["cpp:array"] ByteList inSeq, out ["cpp:array"] ByteList outSeq);

    VariableList opVariableArray(["cpp:array"] VariableList inSeq, out VariableList outSeq);

    BoolSeq opBoolRange(["cpp:range"] BoolSeq inSeq, out BoolSeq outSeq);

    ["cpp:range"] ByteList opByteRange(["cpp:range"] ByteList inSeq, out ["cpp:range"] ByteList outSeq);

    VariableList opVariableRange(["cpp:range"] VariableList inSeq, out VariableList outSeq);

    ByteList opByteRangeType(["cpp:range:::Test::ByteList"] ByteList inSeq, out ByteList outSeq);

    VariableList
    opVariableRangeType(["cpp:range:std::deque< ::Test::Variable>"] VariableList inSeq, out VariableList outSeq);

    ["cpp:type:std::deque<bool>"] BoolSeq
    opBoolSeq(["cpp:type:std::deque<bool>"] BoolSeq inSeq, out ["cpp:type:std::deque<bool>"]BoolSeq outSeq);

    BoolList opBoolList(BoolList inSeq, out BoolList outSeq);

    ["cpp:type:std::deque< ::Ice::Byte>"] ByteSeq
    opByteSeq(["cpp:type:std::deque< ::Ice::Byte>"] ByteSeq inSeq,
              out ["cpp:type:std::deque< ::Ice::Byte>"] ByteSeq outSeq);

    ByteList opByteList(ByteList inSeq, out ByteList outSeq);

    ["cpp:type:MyByteSeq"] ByteSeq
    opMyByteSeq(["cpp:type:MyByteSeq"] ByteSeq inSeq, out ["cpp:type:MyByteSeq"] ByteSeq outSeq);


    ["cpp:view-type:Util::string_view"] string
    opString(["cpp:view-type:Util::string_view"] string inString,
             out ["cpp:view-type:Util::string_view"] string outString);

    ["cpp:type:std::deque<std::string>"] StringSeq
    opStringSeq(["cpp:type:std::deque<std::string>"] StringSeq inSeq,
                out ["cpp:type:std::deque<std::string>"] StringSeq outSeq);

    StringList opStringList(StringList inSeq, out StringList outSeq);

    ["cpp:type:std::deque< ::Test::Fixed>"] FixedSeq
    opFixedSeq(["cpp:type:std::deque< ::Test::Fixed>"] FixedSeq inSeq,
               out ["cpp:type:std::deque< ::Test::Fixed>"] FixedSeq outSeq);

    FixedList opFixedList(FixedList inSeq, out FixedList outSeq);

    ["cpp:type:std::deque< ::Test::Variable>"] VariableSeq
    opVariableSeq(["cpp:type:std::deque< ::Test::Variable>"] VariableSeq inSeq,
                  out ["cpp:type:std::deque< ::Test::Variable>"] VariableSeq outSeq);

    VariableList opVariableList(VariableList inSeq, out VariableList outSeq);

    ["cpp:type:std::deque< ::Test::StringStringDict>"] StringStringDictSeq
    opStringStringDictSeq(["cpp:type:std::deque< ::Test::StringStringDict>"] StringStringDictSeq inSeq,
                          out ["cpp:type:std::deque< ::Test::StringStringDict>"] StringStringDictSeq outSeq);

    StringStringDictList opStringStringDictList(StringStringDictList inSeq, out StringStringDictList outSeq);

    ["cpp:type:std::deque< ::Test::E>"] ESeq
    opESeq(["cpp:type:std::deque< ::Test::E>"] ESeq inSeq, out ["cpp:type:std::deque< ::Test::E>"] ESeq outSeq);

    EList opEList(EList inSeq, out EList outSeq);

    ["cpp:type:std::deque< ::Test::CPrx>", "cpp11:type:std::deque<std::shared_ptr<Ice::ObjectPrx>>"] CPrxSeq
    opCPrxSeq(["cpp:type:std::deque< ::Test::CPrx>", "cpp11:type:std::deque<std::shared_ptr<Ice::ObjectPrx>>"] CPrxSeq inSeq,
              out ["cpp:type:std::deque< ::Test::CPrx>", "cpp11:type:std::deque<std::shared_ptr<Ice::ObjectPrx>>"] CPrxSeq outSeq);

    CPrxList opCPrxList(CPrxList inSeq, out CPrxList outSeq);

    ["cpp:type:std::deque< ::Test::CPtr>", "cpp11:type:std::deque<std::shared_ptr<Test::C>>"] CSeq
    opCSeq(["cpp:type:std::deque< ::Test::CPtr>", "cpp11:type:std::deque<std::shared_ptr<Test::C>>"] CSeq inSeq,
           out ["cpp:type:std::deque< ::Test::CPtr>", "cpp11:type:std::deque<std::shared_ptr<Test::C>>"] CSeq outSeq);

    CList opCList(CList inSeq, out CList outSeq);

    ClassStruct opClassStruct(ClassStruct inS, ClassStructSeq inSeq, out ClassStruct outS, out ClassStructSeq outSeq);

    void opOutArrayByteSeq(ByteSeq org, out ["cpp:array"] ByteSeq copy);

    void opOutRangeByteSeq(ByteSeq org, out ["cpp:range"] ByteSeq copy);

    IntStringDict opIntStringDict(IntStringDict idict, out IntStringDict odict);

    ["cpp:type:::Test::CustomMap< ::Ice::Long, ::Ice::Long>"] LongLongDict
    opVarDict(["cpp:type:::Test::CustomMap<std::string, ::Ice::Int>"] StringIntDict idict,
              out ["cpp:type:::Test::CustomMap<std::string, ::Ice::Int>"] StringIntDict odict);

    ["cpp:view-type:::std::map< ::Ice::Int, ::Util::string_view>", "cpp:type:::Test::CustomMap< ::Ice::Int, std::string>"] IntStringDict
    opCustomIntStringDict(
        ["cpp:view-type:::std::map< ::Ice::Int, ::Util::string_view>", "cpp:type:::Test::CustomMap< ::Ice::Int, std::string>"] IntStringDict idict,
        out ["cpp:view-type:::std::map< ::Ice::Int, ::Util::string_view>", "cpp:type:::Test::CustomMap< ::Ice::Int, std::string>"] IntStringDict odict);


    ShortBuffer opShortBuffer(ShortBuffer inS, out ShortBuffer outS);

    ["cpp:type:::Test::CustomBuffer<bool>"] BoolSeq opBoolBuffer(
        ["cpp:type:::Test::CustomBuffer<bool>"] BoolSeq inS,
        out ["cpp:type:::Test::CustomBuffer<bool>"] BoolSeq outS);

    BufferStruct opBufferStruct(BufferStruct s);

    void shutdown();
};

};
