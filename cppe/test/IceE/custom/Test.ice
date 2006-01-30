// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

module Test
{

sequence<bool> BoolSeq;
["cpp:type:std::list<bool>"] sequence<bool> BoolList;

sequence<byte> ByteSeq;
["cpp:type:std::list< ::Ice::Byte>"] sequence<byte> ByteList;

sequence<string> StringSeq;
["cpp:type:std::list<std::string>"] sequence<string> StringList;

struct Fixed
{
    short s;
};

sequence<Fixed> FixedSeq;
["cpp:type:std::list< ::Test::Fixed>"] sequence<Fixed> FixedList;

struct Variable
{
    string s;
};

sequence<Variable> VariableSeq;
["cpp:type:std::list< ::Test::Variable>"] sequence<Variable> VariableList;


["cpp:array"] sequence<byte> ByteArray;

class TestIntf
{
    ["cpp:array"] ByteSeq opByteArray(["cpp:array"] ByteSeq inSeq, out ["cpp:array"] ByteSeq outSeq);

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

    void shutdown();
};

};

#endif
