// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <deque>
#include <list>
#include <IceE/IceE.h>
#include <IceE/Locator.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

Test::TestIntfPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    tprintf("testing stringToProxy... ");
    string ref = communicator->getProperties()->getPropertyWithDefault(
	"Custom.Proxy", "test:default -p 12345 -t 10000");
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    Test::TestIntfPrx t = Test::TestIntfPrx::checkedCast(base);
    test(t);
    test(t == base);
    tprintf("ok\n");

    tprintf("testing custom bool sequences... ");

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

    tprintf("ok\n");

    tprintf("testing custom byte sequences... ");

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

    tprintf("ok\n");

    tprintf("testing custom string sequences... ");

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

    tprintf("ok\n");

    tprintf("testing custom fixed struct sequences... ");

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

    tprintf("ok\n");

    tprintf("testing custom variable struct sequences... ");

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

    tprintf("ok\n");

    return t;
}
