// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <IceE/Locator.h>
#include <TestCommon.h>
#include <Test.h>
#include <Wstring.h>

using namespace std;

Test::TestIntfPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    tprintf("testing stringToProxy... ");
    string ref = communicator->getProperties()->getPropertyWithDefault(
	"Custom.Proxy", "test:default -p 12010 -t 10000");
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    Test::TestIntfPrx t = Test::TestIntfPrx::checkedCast(base);
    test(t);
    test(t == base);
    tprintf("ok\n");

    tprintf("testing alternate sequences... ");

    {
        Test::BoolSeq in(5);
	in[0] = false;
	in[1] = true;
	in[2] = true;
	in[3] = false;
	in[4] = true;
	bool inArray[5];
	for(int i = 0; i < 5; ++i)
	{
	    inArray[i] = in[i];
	}
	pair<const bool*, const bool*> inPair(inArray, inArray + 5);

	Test::BoolSeq out;
	Test::BoolSeq ret = t->opBoolArray(inPair, out);
	test(out == in);
	test(ret == in);
    }

    {
        Test::ByteList in;
	Ice::Byte inArray[5];
	inArray[0] = '1';
	in.push_back(inArray[0]);
	inArray[1] = '2';
	in.push_back(inArray[1]);
	inArray[2] = '3';
	in.push_back(inArray[2]);
	inArray[3] = '4';
	in.push_back(inArray[3]);
	inArray[4] = '5';
	in.push_back(inArray[4]);
	pair<const Ice::Byte*, const Ice::Byte*> inPair(inArray, inArray + 5);

	Test::ByteList out;
	Test::ByteList ret = t->opByteArray(inPair, out);
	test(out == in);
	test(ret == in);
    }

    {
        Test::VariableList in;
	Test::Variable inArray[5];
	inArray[0].s = "These";
	in.push_back(inArray[0]);
	inArray[1].s = "are";
	in.push_back(inArray[1]);
	inArray[2].s = "five";
	in.push_back(inArray[2]);
	inArray[3].s = "short";
	in.push_back(inArray[3]);
	inArray[4].s = "strings.";
	in.push_back(inArray[4]);
	pair<const Test::Variable*, const Test::Variable*> inPair(inArray, inArray + 5);

	Test::VariableList out;
	Test::VariableList ret = t->opVariableArray(inPair, out);
	test(out == in);
	test(ret == in);
    }

    {
        Test::BoolSeq in(5);
	in[0] = false;
	in[1] = true;
	in[2] = true;
	in[3] = false;
	in[4] = true;
	pair<Test::BoolSeq::const_iterator, Test::BoolSeq::const_iterator> inPair(in.begin(), in.end());

	Test::BoolSeq out;
	Test::BoolSeq ret = t->opBoolRange(inPair, out);
	test(out == in);
	test(ret == in);
    }

    {
        Test::ByteList in;
	in.push_back('1');
	in.push_back('2');
	in.push_back('3');
	in.push_back('4');
	in.push_back('5');
	pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator> inPair(in.begin(), in.end());

	Test::ByteList out;
	Test::ByteList ret = t->opByteRange(inPair, out);
	test(out == in);
	test(ret == in);
    }

    {
        Test::VariableList in;
	Test::Variable v;
	v.s = "These";
	in.push_back(v);
	v.s = "are";
	in.push_back(v);
	v.s = "five";
	in.push_back(v);
	v.s = "short";
	in.push_back(v);
	v.s = "strings.";
	in.push_back(v);
	pair<Test::VariableList::const_iterator, Test::VariableList::const_iterator> inPair(in.begin(), in.end());

	Test::VariableList out;
	Test::VariableList ret = t->opVariableRange(inPair, out);
	test(out == in);
	test(ret == in);
    }

    {
        Test::BoolSeq in(5);
	in[0] = false;
	in[1] = true;
	in[2] = true;
	in[3] = false;
	in[4] = true;
	bool inArray[5];
	for(int i = 0; i < 5; ++i)
	{
	    inArray[i] = in[i];
	}
	pair<const bool*, const bool*> inPair(inArray, inArray + 5);

	Test::BoolSeq out;
	Test::BoolSeq ret = t->opBoolRangeType(inPair, out);
	test(out == in);
	test(ret == in);
    }

    {
        Test::ByteList in;
	in.push_back('1');
	in.push_back('2');
	in.push_back('3');
	in.push_back('4');
	in.push_back('5');
	pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator> inPair(in.begin(), in.end());

	Test::ByteList out;
	Test::ByteList ret = t->opByteRangeType(inPair, out);
	test(out == in);
	test(ret == in);
    }

    {
        Test::VariableList in;
	deque<Test::Variable> inSeq;
	Test::Variable v;
	v.s = "These";
	in.push_back(v);
	inSeq.push_back(v);
	v.s = "are";
	in.push_back(v);
	inSeq.push_back(v);
	v.s = "five";
	in.push_back(v);
	inSeq.push_back(v);
	v.s = "short";
	in.push_back(v);
	inSeq.push_back(v);
	v.s = "strings.";
	in.push_back(v);
	inSeq.push_back(v);
	pair<deque<Test::Variable>::const_iterator, deque<Test::Variable>::const_iterator>
	    inPair(inSeq.begin(), inSeq.end());

	Test::VariableList out;
	Test::VariableList ret = t->opVariableRangeType(inPair, out);
	test(out == in);
	test(ret == in);
    }

    {
        deque<bool> in(5);
	in[0] = false;
	in[1] = true;
	in[2] = true;
	in[3] = false;
	in[4] = true;

	deque<bool> out;
	deque<bool> ret = t->opBoolSeq(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        list<bool> in;
	in.push_back(false);
	in.push_back(true);
	in.push_back(true);
	in.push_back(false);
	in.push_back(true);

	list<bool> out;
	list<bool> ret = t->opBoolList(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        deque< ::Ice::Byte> in(5);
	in[0] = '1';
	in[1] = '2';
	in[2] = '3';
	in[3] = '4';
	in[4] = '5';

	deque< ::Ice::Byte> out;
	deque< ::Ice::Byte> ret = t->opByteSeq(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        list< ::Ice::Byte> in;
	in.push_back('1');
	in.push_back('2');
	in.push_back('3');
	in.push_back('4');
	in.push_back('5');

	list< ::Ice::Byte> out;
	list< ::Ice::Byte> ret = t->opByteList(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        MyByteSeq in(5);
	int i = 0;
	for(MyByteSeq::iterator p = in.begin(); p != in.end(); ++p)
	{
	    *p = '1' + i++;
	}

	MyByteSeq out;
	MyByteSeq ret = t->opMyByteSeq(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        deque<string> in(5);
	in[0] = "These";
	in[1] = "are";
	in[2] = "five";
	in[3] = "short";
	in[4] = "strings.";

	deque<string> out;
	deque<string> ret = t->opStringSeq(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        list<string> in;
	in.push_back("These");
	in.push_back("are");
	in.push_back("five");
	in.push_back("short");
	in.push_back("strings.");

	list<string> out;
	list<string> ret = t->opStringList(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        deque<Test::Fixed> in(5);
	in[0].s = 1;
	in[1].s = 2;
	in[2].s = 3;
	in[3].s = 4;
	in[4].s = 5;

	deque<Test::Fixed> out;
	deque<Test::Fixed> ret = t->opFixedSeq(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        list<Test::Fixed> in(5);
	short num = 1;
	for(list<Test::Fixed>::iterator p = in.begin(); p != in.end(); ++p)
	{
	    (*p).s = num++;
	}

	list<Test::Fixed> out;
	list<Test::Fixed> ret = t->opFixedList(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        deque<Test::Variable> in(5);
	in[0].s = "These";
	in[1].s = "are";
	in[2].s = "five";
	in[3].s = "short";
	in[4].s = "strings.";

	deque<Test::Variable> out;
	deque<Test::Variable> ret = t->opVariableSeq(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        list<Test::Variable> in;
	Test::Variable v;
	v.s = "These";
	in.push_back(v);
	v.s = "are";
	in.push_back(v);
	v.s = "five";
	in.push_back(v);
	v.s = "short";
	in.push_back(v);
	v.s = "strings.";
	in.push_back(v);

	list<Test::Variable> out;
	list<Test::Variable> ret = t->opVariableList(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        deque<Test::StringStringDict> in(5);
	in[0]["A"] = "a";
	in[1]["B"] = "b";
	in[2]["C"] = "c";
	in[3]["D"] = "d";
	in[4]["E"] = "e";

	deque<Test::StringStringDict> out;
	deque<Test::StringStringDict> ret = t->opStringStringDictSeq(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        list<Test::StringStringDict> in;
	Test::StringStringDict ssd;
	ssd["A"] = "a";
	in.push_back(ssd);
	ssd["B"] = "b";
	in.push_back(ssd);
	ssd["C"] = "c";
	in.push_back(ssd);
	ssd["D"] = "d";
	in.push_back(ssd);
	ssd["E"] = "e";
	in.push_back(ssd);

	list<Test::StringStringDict> out;
	list<Test::StringStringDict> ret = t->opStringStringDictList(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        deque<Test::E> in(5);
	in[0] = Test::E1;
	in[1] = Test::E2;
	in[2] = Test::E3;
	in[3] = Test::E1;
	in[4] = Test::E3;

	deque<Test::E> out;
	deque<Test::E> ret = t->opESeq(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        list<Test::E> in;
	in.push_back(Test::E1);
	in.push_back(Test::E2);
	in.push_back(Test::E3);
	in.push_back(Test::E1);
	in.push_back(Test::E3);

	list<Test::E> out;
	list<Test::E> ret = t->opEList(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        deque<Test::CPrx> in(5);
	in[0] = Test::CPrx::uncheckedCast(communicator->stringToProxy("C1:default -p 12010 -t 10000"));
	in[1] = Test::CPrx::uncheckedCast(communicator->stringToProxy("C2:default -p 12010 -t 10001"));
	in[2] = Test::CPrx::uncheckedCast(communicator->stringToProxy("C3:default -p 12010 -t 10002"));
	in[3] = Test::CPrx::uncheckedCast(communicator->stringToProxy("C4:default -p 12010 -t 10003"));
	in[4] = Test::CPrx::uncheckedCast(communicator->stringToProxy("C5:default -p 12010 -t 10004"));

	deque<Test::CPrx> out;
	deque<Test::CPrx> ret = t->opCPrxSeq(in, out);
	test(out == in);
	test(ret == in);
    }

    {
        list<Test::CPrx> in;
	in.push_back(Test::CPrx::uncheckedCast(communicator->stringToProxy("C1:default -p 12010 -t 10000")));
	in.push_back(Test::CPrx::uncheckedCast(communicator->stringToProxy("C2:default -p 12010 -t 10001")));
	in.push_back(Test::CPrx::uncheckedCast(communicator->stringToProxy("C3:default -p 12010 -t 10002")));
	in.push_back(Test::CPrx::uncheckedCast(communicator->stringToProxy("C4:default -p 12010 -t 10003")));
	in.push_back(Test::CPrx::uncheckedCast(communicator->stringToProxy("C5:default -p 12010 -t 10004")));

	list<Test::CPrx> out;
	list<Test::CPrx> ret = t->opCPrxList(in, out);
	test(out == in);
	test(ret == in);
    }

    tprintf("ok\n");

#ifdef ICEE_HAS_WSTRING
    tprintf("testing wstring... ");

    Test1::WstringSeq wseq1;
    wseq1.push_back(L"Wide String");

    Test2::WstringSeq wseq2;
    wseq2 = wseq1;

    Test1::WstringWStringDict wdict1;
    wdict1[L"Key"] = L"Value";

    Test2::WstringWStringDict wdict2;
    wdict2 = wdict1;

    ref = communicator->getProperties()->getPropertyWithDefault(
        "Custom.WstringProxy1", "wstring1:default -p 12010 -t 10000");
    base = communicator->stringToProxy(ref);
    test(base);
    Test1::WstringClassPrx wsc1 = Test1::WstringClassPrx::checkedCast(base);
    test(t);

    ref = communicator->getProperties()->getPropertyWithDefault(
        "Custom.WstringProxy2", "wstring2:default -p 12010 -t 10000");
    base = communicator->stringToProxy(ref);
    test(base);
    Test2::WstringClassPrx wsc2 = Test2::WstringClassPrx::checkedCast(base);
    test(t);

    wstring wstr = L"A Wide String";
    wstring out;
    wstring ret = wsc1->opString(wstr, out);
    test(out == wstr);
    test(ret == wstr);

    ret = wsc2->opString(wstr, out);
    test(out == wstr);
    test(ret == wstr);

    Test1::WstringStruct wss1;
    wss1.s = wstr;
    Test1::WstringStruct wss1out;
    Test1::WstringStruct wss1ret = wsc1->opStruct(wss1, wss1out);
    test(wss1out == wss1);
    test(wss1ret == wss1);

    Test2::WstringStruct wss2;
    wss2.s = wstr;
    Test2::WstringStruct wss2out;
    Test2::WstringStruct wss2ret = wsc2->opStruct(wss2, wss2out);
    test(wss2out == wss2);
    test(wss2ret == wss2);

    try
    {
        wsc1->throwExcept(wstr);
    }
    catch(const Test1::WstringException& ex)
    {
        test(ex.reason == wstr);
    }

    try
    {
        wsc2->throwExcept(wstr);
    }
    catch(const Test2::WstringException& ex)
    {
        test(ex.reason == wstr);
    }

    tprintf("ok\n");
#endif

    return t;
}
