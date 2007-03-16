// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

[["cpp:include:deque", "cpp:include:list", "cpp:include:MyByteSeq.h"]]

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
sequence<C*> CPrxSeq;
["cpp:type:std::list< ::Test::CPrx>"] sequence<C*> CPrxList;

["cpp:type:std::list< ::Test::CPrxList>"] sequence<CPrxList> CPrxListList;
sequence<CPrxList> CPrxListSeq;
["cpp:type:std::list< ::Test::CPrxSeq>"] sequence<CPrxSeq> CPrxSeqList;

class TestIntf
{
    BoolSeq opBoolArray(["cpp:array"] BoolSeq inSeq, out BoolSeq outSeq);

    ByteList opByteArray(["cpp:array"] ByteList inSeq, out ByteList outSeq);

    VariableList opVariableArray(["cpp:array"] VariableList inSeq, out VariableList outSeq);

    BoolSeq opBoolRange(["cpp:range"] BoolSeq inSeq, out BoolSeq outSeq);

    ByteList opByteRange(["cpp:range"] ByteList inSeq, out ByteList outSeq);

    VariableList opVariableRange(["cpp:range"] VariableList inSeq, out VariableList outSeq);

    BoolSeq opBoolRangeType(["cpp:range:array"] BoolSeq inSeq, out BoolSeq outSeq);
    
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

    ["cpp:type:std::deque< ::Test::CPrx>"] CPrxSeq 
    opCPrxSeq(["cpp:type:std::deque< ::Test::CPrx>"] CPrxSeq inSeq, 
    	      out ["cpp:type:std::deque< ::Test::CPrx>"] CPrxSeq outSeq);

    CPrxList opCPrxList(CPrxList inSeq, out CPrxList outSeq);

    void shutdown();
};

};

#endif
