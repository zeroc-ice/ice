// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

#include <IceXML/StreamI.h>

using namespace std;

static string header =
"<ice:data xmlns=\"http://www.noorg.org/schemas\""
" xmlns:ice=\"http://www.zeroc.com/schemas\""
" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
" xsi:schemaLocation=\"http://www.noorg.org/schemas Test.xsd\">";

static string footer =
"</ice:data>";

static const string element = "s";
static const string strings[] =
{
    "hello world",
    "hello & world",
    "\"hello world\"",
    "'hello world'",
    "hello <world",
    "hello >world",
    "hello >>world",
    "hello <<>>world",
    "hello &&''\"\"<<>>world",
    ""
};

void
TestString(const Ice::CommunicatorPtr& communicator)
{
    for(int i = 0; !strings[i].empty(); ++i)
    {
	ostringstream os;
	os << "<data>";
	Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
	ostream->writeString(element, strings[i]);
	os << "</data>";;
	
	istringstream is(os.str());
	Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
	string result = istream->readString(element);
	test(result == strings[i]);
    }
}

void
TestStruct1(const Ice::CommunicatorPtr& communicator)
{
    const string element = "Test.Struct1";

    Test::Struct1 sin;
    sin.l = 10;

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    sin.ice_marshal(element, ostream);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    Test::Struct1 sout;
    sout.ice_unmarshal(element, istream);
    test(sin == sout);
}

void
TestStruct2(const Ice::CommunicatorPtr& communicator)
{
    const string element = "Test.Struct2";

    Test::Struct2 sin;
    sin.s1.l = 10;

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    sin.ice_marshal(element, ostream);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    Test::Struct2 sout;
    sout.ice_unmarshal(element, istream);
    test(sin == sout);
}

void
TestStruct3(const Ice::CommunicatorPtr& communicator)
{
    const string element = "Test.Struct3";

    Test::Struct3 sin;
    sin.l = 20;
    sin.s2.s1.l = 10;

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    sin.ice_marshal(element, ostream);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    Test::Struct3 sout;
    sout.ice_unmarshal(element, istream);
    test(sin == sout);
}

void
TestStruct4(const Ice::CommunicatorPtr& communicator)
{
    const string element = "Test.Struct4";

    Test::Struct4 sin;
    sin.l = 30;
    sin.s3.l = 20;
    sin.s3.s2.s1.l = 10;

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    sin.ice_marshal(element, ostream);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    Test::Struct4 sout;
    sout.ice_unmarshal(element, istream);
    test(sin == sout);
}

void
TestStruct4Seq(const Ice::CommunicatorPtr& communicator)
{
    const string element = "Test.Struct4Seq";

    Test::Struct4Seq seqin;

    Test::Struct4 sin;
    sin.l = 30;
    sin.s3.l = 20;
    sin.s3.s2.s1.l = 10;
    seqin.push_back(sin);

    sin.l = 40;
    sin.s3.l = 30;
    sin.s3.s2.s1.l = 20;
    seqin.push_back(sin);

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    Test::Struct4SeqHelper::ice_marshal(element, ostream, seqin);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    Test::Struct4Seq seqout;
    Test::Struct4SeqHelper::ice_unmarshal(element, istream, seqout);
    test(seqout.size() == seqin.size());
    while(!seqin.empty())
    {
	Test::Struct4 sin = seqin.back();
	Test::Struct4 sout = seqout.back();
	test(sin == sout);
	seqout.pop_back();
	seqin.pop_back();
    }
}

void
TestStringStruct4Dict(const Ice::CommunicatorPtr& communicator)
{
    const string element = "Test.StringStruct4Dict";

    Test::StringStruct4Dict dictin;

    Test::Struct4 sin;
    sin.l = 30;
    sin.s3.l = 20;
    sin.s3.s2.s1.l = 10;
    dictin["1"] = sin;

    sin.l = 40;
    sin.s3.l = 30;
    sin.s3.s2.s1.l = 20;
    dictin["2"] = sin;

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    Test::StringStruct4DictHelper::ice_marshal(element, ostream, dictin);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    Test::StringStruct4Dict dictout;
    Test::StringStruct4DictHelper::ice_unmarshal(element, istream, dictout);
    test(dictout.size() == dictin.size());
    for(Test::StringStruct4Dict::const_iterator p = dictin.begin(); p != dictin.end(); ++p)
    {
	Test::StringStruct4Dict::const_iterator q = dictout.find(p->first);
	test(q != dictout.end());
	test(q->second == p->second);
    }
}

void
TestColor(const Ice::CommunicatorPtr& communicator)
{
    const string element = "Test.Color";

    Test::Color ein = Test::Red;

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    ::Test::ice_marshal(element, ostream, ein);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    Test::Color eout;
    ::Test::ice_unmarshal(element, istream, eout);
    test(ein == eout);
}

void
TestColorSeq(const Ice::CommunicatorPtr& communicator)
{
    const string element = "Test.ColorSeq";

    Test::ColorSeq seqin;

    seqin.push_back(Test::Red);
    seqin.push_back(Test::Green);
    seqin.push_back(Test::Blue);

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    Test::ColorSeqHelper::ice_marshal(element, ostream, seqin);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    Test::ColorSeq seqout;
    Test::ColorSeqHelper::ice_unmarshal(element, istream, seqout);
    test(seqout.size() == seqin.size());
    while(!seqin.empty())
    {
	Test::Color sin = seqin.back();
	Test::Color sout = seqout.back();
	test(sin == sout);
	seqout.pop_back();
	seqin.pop_back();
    }
}

void
TestClass1(const Ice::CommunicatorPtr& communicator)
{
    const string element = "Test.Class1";

    Test::Class1Ptr in = new Test::Class1();
    in->c = Test::Red;
    in->name = "Red";

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    in->ice_marshal(element, ostream);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    Test::Class1Ptr out;
    Test::Class1::ice_unmarshal(element, istream, out);
    test(in->c == out->c && in->name == out->name);
}

void
TestClass2(const Ice::CommunicatorPtr& communicator)
{
    const string element = "Test.Class2";

    Test::Class2Ptr in = new Test::Class2();
    in->c = Test::Blue;
    in->name = "Blue";

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    in->ice_marshal(element, ostream);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    Test::Class2Ptr out;
    Test::Class2::ice_unmarshal(element, istream, out);
    test(in->c == out->c && in->name == out->name);
}

void
TestClass2Rec(const Ice::CommunicatorPtr& communicator)
{
    const string element = "Test.Class2";

    Test::Class2Ptr in = new Test::Class2();
    in->c = Test::Blue;
    in->name = "Blue";
    Test::Class2Ptr in2 = new Test::Class2();
    in2->c = Test::Green;
    in2->name = "Green";
    in2->r = in;
    in->r = in2;

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    in->ice_marshal(element, ostream);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    Test::Class2Ptr out;
    Test::Class2::ice_unmarshal(element, istream, out);
    test(out->r);
    test(out->r->r);
    test(in->c == out->c && in->name == out->name);
    test(in->r->c == out->r->c && in->r->name == out->r->name);
    test(out == out->r->r);

    //
    // Clear recursive fields to avoid memory leak complains from Purify
    //
    in->r = 0;
    out->r = 0;
}

class Class3I : public Test::Class3
{
public:

    virtual void if1(const ::Ice::Current&)
    {
    }
};

class Class3Factory : public ::Ice::ObjectFactory
{
public:

    virtual ::Ice::ObjectPtr
    create(const ::std::string& type)
    {
	test(type == ::Test::Class3::ice_staticId());
	return new Class3I;
    }

    virtual void
    destroy()
    {
    }

};

void
TestClass3(const Ice::CommunicatorPtr& communicator)
{
    const string element = "Test.Class3";

    Test::Class3Ptr in = new Class3I();
    in->c = Test::Blue;
    in->name = "Blue";

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    in->ice_marshal(element, ostream);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    ::Ice::ObjectPtr o;
    ::Ice::Object::ice_unmarshal(element, istream, o);

    Test::Class3Ptr out = ::Test::Class3Ptr::dynamicCast(o);
    test(in->c == out->c && in->name == out->name);
}

void
TestClass3Rec(const Ice::CommunicatorPtr& communicator)
{
    const string element = "Test.Class3";

    Test::Class3Ptr in = new Class3I();
    in->c = Test::Blue;
    in->name = "Blue";
    Test::Class3Ptr in2 = new Class3I();
    in2->c = Test::Green;
    in2->name = "Green";
    in2->r = in;
    in->r = in2;

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    in->ice_marshal(element, ostream);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    ::Ice::ObjectPtr o;
    ::Ice::Object::ice_unmarshal(element, istream, o);
    Test::Class3Ptr out = ::Test::Class3Ptr::dynamicCast(o);
    test(out->r);
    test(out->r->r);
    test(in->c == out->c && in->name == out->name);
    test(in->r->c == out->r->c && in->r->name == out->r->name);
    test(out == out->r->r);

    //
    // Clear recursive fields to avoid memory leak complains from Purify
    //
    in->r = 0;
    out->r = 0;
}

void
TestFacets(const Ice::CommunicatorPtr& communicator)
{
    communicator->addObjectFactory(Test::Class1::ice_factory(), Test::Class1::ice_staticId());
    communicator->addObjectFactory(Test::Class2::ice_factory(), Test::Class2::ice_staticId());

    const string element = "Test.Class2";

    Test::Class2Ptr in = new Test::Class2();
    in->c = Test::Blue;
    in->name = "Blue";
    Test::Class2Ptr in2 = new Test::Class2();
    in2->c = Test::Green;
    in2->name = "Green";
    in2->r = in;
    in->r = in2;

    Test::Class1Ptr facet = new Test::Class1();
    facet->c = Test::Red;
    facet->name = "Red";
    in->ice_addFacet(facet, "red");

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    in->ice_marshal(element, ostream);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    Test::Class2Ptr out;
    Test::Class2::ice_unmarshal(element, istream, out);
    test(out->r);
    test(out->r->r);
    test(in->c == out->c && in->name == out->name);
    test(in->r->c == out->r->c && in->r->name == out->r->name);
    test(out == out->r->r);
    Ice::ObjectPtr obj = out->ice_findFacet("red");
    test(obj);
    Test::Class1Ptr outFacet = Test::Class1Ptr::dynamicCast(obj);
    test(outFacet);
    test(facet->c == outFacet->c && facet->name == outFacet->name);

    communicator->removeObjectFactory(Test::Class1::ice_staticId());
    communicator->removeObjectFactory(Test::Class2::ice_staticId());

    //
    // Clear recursive fields to avoid memory leak complains from Purify
    //
    in->r = 0;
    out->r = 0;
}

void
TestException1(const Ice::CommunicatorPtr& communicator)
{
    const string element = "Test.Exception1";

    Test::Exception1 in;

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    in.ice_marshal(element, ostream);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    Test::Exception1 out;
    out.ice_unmarshal(element, istream);
}

void
TestException2(const Ice::CommunicatorPtr& communicator)
{
    const string element = "Test.Exception2";

    Test::Exception2 in;
    in.msg = "hello world";

    ostringstream os;
    os << header;
    Ice::StreamPtr ostream = new IceXML::StreamI(communicator, os);
    in.ice_marshal(element, ostream);
    os << footer;

    istringstream is(os.str());
    Ice::StreamPtr istream = new IceXML::StreamI(communicator, is);
    Test::Exception2 out;
    out.ice_unmarshal(element, istream);
    test(in.msg == out.msg);
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    //
    // If the test is run out of the CWD then adjust the location of
    // Test.xsd appropriately.
    //
    if(argc > 1)
    {
	string dir(argv[1]);
	dir += '/';
	size_t pos = header.find("Test.xsd");
	assert(pos != string::npos);
	header.insert(pos, dir);
    }

    cout << "testing string... ";
    TestString(communicator);
    cout << "ok" << endl;

    cout << "testing struct... ";
    TestStruct1(communicator);
    TestStruct2(communicator);
    TestStruct3(communicator);
    TestStruct4(communicator);
    cout << "ok" << endl;

    cout << "testing sequence... ";
    TestStruct4Seq(communicator);
    cout << "ok" << endl;

    cout << "testing dictionaries... ";
    TestStringStruct4Dict(communicator);
    cout << "ok" << endl;

    cout << "testing enumerations... ";
    TestColor(communicator);
    TestColorSeq(communicator);
    cout << "ok" << endl;

    cout << "testing class... ";
    TestClass1(communicator);
    TestClass2(communicator);
    TestClass2Rec(communicator);
    try
    {
	TestClass3(communicator);
	test(false);
    }
    catch(const ::Ice::NoObjectFactoryException&)
    {
    }

    communicator->addObjectFactory(new Class3Factory, ::Test::Class3::ice_staticId());

    TestClass3(communicator);
    TestClass3Rec(communicator);
    TestFacets(communicator);
    cout << "ok" << endl;

    cout << "testing exception... ";
    TestException1(communicator);
    TestException2(communicator);
    cout << "ok" << endl;

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	communicator = Ice::initialize(argc, argv);
	status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if(communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const Ice::Exception& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
