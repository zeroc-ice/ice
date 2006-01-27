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
["cpp:type:std::list"] sequence<bool> BoolList;

sequence<byte> ByteSeq;
["cpp:type:std::list"]sequence<byte> ByteList;

sequence<string> StringSeq;
["cpp:type:std::list"]sequence<string> StringList;

struct Fixed
{
    short s;
};

sequence<Fixed> FixedSeq;
["cpp:type:std::list"]sequence<Fixed> FixedList;

struct Variable
{
    string s;
};

sequence<Variable> VariableSeq;
["cpp:type:std::list"]sequence<Variable> VariableList;

class TestIntf
{
    ["cpp:type:std::deque"] BoolSeq 
    opBoolSeq(["cpp:type:std::deque"] BoolSeq inSeq, out ["cpp:type:std::deque"]BoolSeq outSeq);

    BoolList opBoolList(BoolList inSeq, out BoolList outSeq);

    ["cpp:type:std::deque"] ByteSeq 
    opByteSeq(["cpp:type:std::deque"] ByteSeq inSeq, out ["cpp:type:std::deque"] ByteSeq outSeq);

    ByteList opByteList(ByteList inSeq, out ByteList outSeq);

    ["cpp:type:std::deque"] StringSeq 
    opStringSeq(["cpp:type:std::deque"] StringSeq inSeq, out ["cpp:type:std::deque"] StringSeq outSeq);

    StringList opStringList(StringList inSeq, out StringList outSeq);

    ["cpp:type:std::deque"] FixedSeq 
    opFixedSeq(["cpp:type:std::deque"] FixedSeq inSeq, out ["cpp:type:std::deque"] FixedSeq outSeq);

    FixedList opFixedList(FixedList inSeq, out FixedList outSeq);

    ["cpp:type:std::deque"] VariableSeq 
    opVariableSeq(["cpp:type:std::deque"] VariableSeq inSeq, out ["cpp:type:std::deque"] VariableSeq outSeq);

    VariableList opVariableList(VariableList inSeq, out VariableList outSeq);

    void shutdown();
};

};

#endif
