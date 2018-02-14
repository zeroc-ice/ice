// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

DEFINE_TEST("client")

using namespace std;

class TestObjectWriter : public Ice::ObjectWriter
{
public:

    TestObjectWriter(const Test::MyClassPtr& p)
    {
        obj = p;
        called = false;
    }

    virtual void
    write(const Ice::OutputStreamPtr& out) const
    {
        obj->__write(out);
        const_cast<TestObjectWriter*>(this)->called = true;
    }

    Test::MyClassPtr obj;
    bool called;
};
typedef IceUtil::Handle<TestObjectWriter> TestObjectWriterPtr;

class TestObjectReader : public Ice::ObjectReader
{
public:

    TestObjectReader()
    {
        called = false;
    }

    virtual void
    read(const Ice::InputStreamPtr& in)
    {
        obj = new Test::MyClass;
        obj->__read(in);
        called = true;
    }

    Test::MyClassPtr obj;
    bool called;
};
typedef IceUtil::Handle<TestObjectReader> TestObjectReaderPtr;

class TestObjectFactory : public Ice::ObjectFactory
{
public:

    virtual Ice::ObjectPtr
#ifndef NDEBUG
    create(const string& type)
#else
    create(const string&)
#endif
    {
        assert(type == Test::MyClass::ice_staticId());
        return new TestObjectReader;
    }

    virtual void
    destroy()
    {
    }
};

class TestReadObjectCallback : public Ice::ReadObjectCallback
{
public:

    virtual void
    invoke(const Ice::ObjectPtr& p)
    {
        obj = p;
    }

    Ice::ObjectPtr obj;
};
typedef IceUtil::Handle<TestReadObjectCallback> TestReadObjectCallbackPtr;

class MyClassFactoryWrapper : public Ice::ObjectFactory
{
public:

    MyClassFactoryWrapper() : _factory(Test::MyClass::ice_factory())
    {
    }

    virtual Ice::ObjectPtr
    create(const string& type)
    {
        return _factory->create(type);
    }

    virtual void
    destroy()
    {
    }

    void
    setFactory(const Ice::ObjectFactoryPtr& factory)
    {
        if(!factory)
        {
            _factory = Test::MyClass::ice_factory();
        }
        else
        {
            _factory = factory;
        }
    }

private:

    Ice::ObjectFactoryPtr _factory;
};
typedef IceUtil::Handle<MyClassFactoryWrapper> MyClassFactoryWrapperPtr;

class MyInterfaceFactory : public Ice::ObjectFactory
{
public:

    virtual Ice::ObjectPtr
    create(const string&)
    {
        return new Test::MyInterface;
    }

    virtual void
    destroy()
    {
    }
};

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    MyClassFactoryWrapperPtr factoryWrapper = new MyClassFactoryWrapper;
    communicator->addObjectFactory(factoryWrapper, Test::MyClass::ice_staticId());
    communicator->addObjectFactory(new MyInterfaceFactory, Test::MyInterface::ice_staticId());

    Ice::InputStreamPtr in;
    Ice::OutputStreamPtr out;
    vector<Ice::Byte> data;

    //
    // Test the stream api.
    //
    cout << "testing primitive types... " << flush;

    {
        vector<Ice::Byte> byte;
        in = Ice::createInputStream(communicator, byte);
    }

    {
        out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(true);
        out->endEncapsulation();
        out->finished(data);
        pair<const Ice::Byte*, const Ice::Byte*> d = out->finished();
        test(d.second - d.first == static_cast<int>(data.size()));
        test(vector<Ice::Byte>(d.first, d.second) == data);
        out = 0;

        in = Ice::createInputStream(communicator, data);
        in->startEncapsulation();
        bool v;
        in->read(v);
        test(v);
        in->endEncapsulation();

        in = Ice::wrapInputStream(communicator, data);
        in->startEncapsulation();
        in->read(v);
        test(v);
        in->endEncapsulation();
    }

    {
        vector<Ice::Byte> byte;
        in = Ice::createInputStream(communicator, byte);
        try
        {
            bool v;
            in->read(v);
            test(false);
        }
        catch(const Ice::UnmarshalOutOfBoundsException&)
        {
        }
    }

    {
        out = Ice::createOutputStream(communicator);
        out->write(true);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        bool v;
        in->read(v);
        test(v);
    }

    {
        out = Ice::createOutputStream(communicator);
        out->write((Ice::Byte)1);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Ice::Byte v;
        in->read(v);
        test(v == 1);
    }

    {
        out = Ice::createOutputStream(communicator);
        out->write((Ice::Short)2);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Ice::Short v;
        in->read(v);
        test(v == 2);
    }

    {
        out = Ice::createOutputStream(communicator);
        out->write((Ice::Int)3);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Ice::Int v;
        in->read(v);
        test(v == 3);
    }

    {
        out = Ice::createOutputStream(communicator);
        out->write((Ice::Long)4);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Ice::Long v;
        in->read(v);
        test(v == 4);
    }

    {
        out = Ice::createOutputStream(communicator);
        out->write((Ice::Float)5.0);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Ice::Float v;
        in->read(v);
        test(v == 5.0);
    }

    {
        out = Ice::createOutputStream(communicator);
        out->write((Ice::Double)6.0);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Ice::Double v;
        in->read(v);
        test(v == 6.0);
    }

    {
        out = Ice::createOutputStream(communicator);
        out->write("hello world");
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        string v;
        in->read(v);
        test(v == "hello world");
    }

    cout << "ok" << endl;

    cout << "testing constructed types... " << flush;

    {
        out = Ice::createOutputStream(communicator);
        out->write(Test::enum3);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test::MyEnum e;
        in->read(e);
        test(e == Test::enum3);
    }

    {
        out = Ice::createOutputStream(communicator);
        Test::SmallStruct s;
        s.bo = true;
        s.by = 1;
        s.sh = 2;
        s.i = 3;
        s.l = 4;
        s.f = 5.0;
        s.d = 6.0;
        s.str = "7";
        s.e = Test::enum2;
        s.p = Test::MyClassPrx::uncheckedCast(communicator->stringToProxy("test:default"));
        out->write(s);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test::SmallStruct s2;
        in->read(s2);
        test(s2 == s);
    }

    {
        out = Ice::createOutputStream(communicator);
        Test::ClassStructPtr s = new Test::ClassStruct();
        s->i = 10;
        out->write(s);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test::ClassStructPtr s2 = new Test::ClassStruct();
        in->read(s2);
        test(s2->i == s->i);
    }

    {
        out = Ice::createOutputStream(communicator);
        Test::OptionalClassPtr o = new Test::OptionalClass();
        o->bo = false;
        o->by = 5;
        o->sh = 4;
        o->i = 3;
        out->write(o);
        out->writePendingObjects();
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test::OptionalClassPtr o2;
        in->read(o2);
        in->readPendingObjects();
        test(o2->bo == o->bo);
        test(o2->by == o->by);
        if(in->getEncoding() == Ice::Encoding_1_0)
        {
            test(!o2->sh);
            test(!o2->i);
        }
        else
        {
            test(o2->sh == o->sh);
            test(o2->i == o->i);
        }
    }

    {
        out = Ice::createOutputStream(communicator, Ice::Encoding_1_0);
        Test::OptionalClassPtr o = new Test::OptionalClass();
        o->bo = false;
        o->by = 5;
        o->sh = 4;
        o->i = 3;
        out->write(o);
        out->writePendingObjects();
        out->finished(data);
        in = Ice::createInputStream(communicator, data, Ice::Encoding_1_0);
        Test::OptionalClassPtr o2;
        in->read(o2);
        in->readPendingObjects();
        test(o2->bo == o->bo);
        test(o2->by == o->by);
        test(!o2->sh);
        test(!o2->i);
    }

    {
        Ice::BoolSeq arr;
        arr.push_back(true);
        arr.push_back(false);
        arr.push_back(true);
        arr.push_back(false);

        out = Ice::createOutputStream(communicator);
        out->write(arr);
        out->finished(data);

        in = Ice::createInputStream(communicator, data);
        Ice::BoolSeq arr2;
        in->read(arr2);
        test(arr2 == arr);

        Test::BoolSS arrS;
        arrS.push_back(arr);
        arrS.push_back(Ice::BoolSeq());
        arrS.push_back(arr);

        out = Ice::createOutputStream(communicator);
        out->write(arrS);
        out->finished(data);

        in = Ice::createInputStream(communicator, data);
        Test::BoolSS arr2S;
        in->read(arr2S);
        test(arr2S == arrS);
    }

    {
        Ice::ByteSeq arr;
        arr.push_back(0x01);
        arr.push_back(0x11);
        arr.push_back(0x12);
        arr.push_back(0x22);

        out = Ice::createOutputStream(communicator);
        out->write(arr);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Ice::ByteSeq arr2;
        in->read(arr2);
        test(arr2 == arr);

        Test::ByteSS arrS;
        arrS.push_back(arr);
        arrS.push_back(Ice::ByteSeq());
        arrS.push_back(arr);

        out = Ice::createOutputStream(communicator);
        out->write(arrS);
        out->finished(data);

        in = Ice::createInputStream(communicator, data);
        Test::ByteSS arr2S;
        in->read(arr2S);
        test(arr2S == arrS);
    }

    {
        Ice::ShortSeq arr;
        arr.push_back(0x01);
        arr.push_back(0x11);
        arr.push_back(0x12);
        arr.push_back(0x22);
        out = Ice::createOutputStream(communicator);
        out->write(arr);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Ice::ShortSeq arr2;
        in->read(arr2);
        test(arr2 == arr);

        Test::ShortSS arrS;
        arrS.push_back(arr);
        arrS.push_back(Ice::ShortSeq());
        arrS.push_back(arr);

        out = Ice::createOutputStream(communicator);
        out->write(arrS);
        out->finished(data);

        in = Ice::createInputStream(communicator, data);
        Test::ShortSS arr2S;
        in->read(arr2S);
        test(arr2S == arrS);
    }

    {
        Ice::IntSeq arr;
        arr.push_back(0x01);
        arr.push_back(0x11);
        arr.push_back(0x12);
        arr.push_back(0x22);
        out = Ice::createOutputStream(communicator);
        out->write(arr);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Ice::IntSeq arr2;
        in->read(arr2);
        test(arr2 == arr);

        Test::IntSS arrS;
        arrS.push_back(arr);
        arrS.push_back(Ice::IntSeq());
        arrS.push_back(arr);

        out = Ice::createOutputStream(communicator);
        out->write(arrS);
        out->finished(data);

        in = Ice::createInputStream(communicator, data);
        Test::IntSS arr2S;
        in->read(arr2S);
        test(arr2S == arrS);
    }

    {
        Ice::LongSeq arr;
        arr.push_back(0x01);
        arr.push_back(0x11);
        arr.push_back(0x12);
        arr.push_back(0x22);
        out = Ice::createOutputStream(communicator);
        out->write(arr);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Ice::LongSeq arr2;
        in->read(arr2);
        test(arr2 == arr);

        Test::LongSS arrS;
        arrS.push_back(arr);
        arrS.push_back(Ice::LongSeq());
        arrS.push_back(arr);

        out = Ice::createOutputStream(communicator);
        out->write(arrS);
        out->finished(data);

        in = Ice::createInputStream(communicator, data);
        Test::LongSS arr2S;
        in->read(arr2S);
        test(arr2S == arrS);
    }

    {
        Ice::FloatSeq arr;
        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(3);
        arr.push_back(4);
        out = Ice::createOutputStream(communicator);
        out->write(arr);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Ice::FloatSeq arr2;
        in->read(arr2);
        test(arr2 == arr);

        Test::FloatSS arrS;
        arrS.push_back(arr);
        arrS.push_back(Ice::FloatSeq());
        arrS.push_back(arr);

        out = Ice::createOutputStream(communicator);
        out->write(arrS);
        out->finished(data);

        in = Ice::createInputStream(communicator, data);
        Test::FloatSS arr2S;
        in->read(arr2S);
        test(arr2S == arrS);
    }

    {
        Ice::DoubleSeq arr;
        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(3);
        arr.push_back(4);
        out = Ice::createOutputStream(communicator);
        out->write(arr);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Ice::DoubleSeq arr2;
        in->read(arr2);
        test(arr2 == arr);

        Test::DoubleSS arrS;
        arrS.push_back(arr);
        arrS.push_back(Ice::DoubleSeq());
        arrS.push_back(arr);

        out = Ice::createOutputStream(communicator);
        out->write(arrS);
        out->finished(data);

        in = Ice::createInputStream(communicator, data);
        Test::DoubleSS arr2S;
        in->read(arr2S);
        test(arr2S == arrS);
    }

    {
        Ice::StringSeq arr;
        arr.push_back("string1");
        arr.push_back("string2");
        arr.push_back("string3");
        arr.push_back("string4");
        out = Ice::createOutputStream(communicator);
        out->write(arr);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Ice::StringSeq arr2;
        in->read(arr2);
        test(arr2 == arr);

        Test::StringSS arrS;
        arrS.push_back(arr);
        arrS.push_back(Ice::StringSeq());
        arrS.push_back(arr);

        out = Ice::createOutputStream(communicator);
        out->write(arrS);
        out->finished(data);

        in = Ice::createInputStream(communicator, data);
        Test::StringSS arr2S;
        in->read(arr2S);
        test(arr2S == arrS);
    }

    {
        Test::MyEnumS arr;
        arr.push_back(Test::enum3);
        arr.push_back(Test::enum2);
        arr.push_back(Test::enum1);
        arr.push_back(Test::enum2);

        out = Ice::createOutputStream(communicator);
        out->write(arr);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test::MyEnumS arr2;
        in->read(arr2);
        test(arr2 == arr);

        Test::MyEnumSS arrS;
        arrS.push_back(arr);
        arrS.push_back(Test::MyEnumS());
        arrS.push_back(arr);

        out = Ice::createOutputStream(communicator);
        out->write(arrS);
        out->finished(data);

        in = Ice::createInputStream(communicator, data);
        Test::MyEnumSS arr2S;
        in->read(arr2S);
        test(arr2S == arrS);
    }

    {
        Test::SmallStructS arr;
        for(int i = 0; i < 4; ++i)
        {
            Test::SmallStruct s;
            s.bo = true;
            s.by = 1;
            s.sh = 2;
            s.i = 3;
            s.l = 4;
            s.f = 5.0;
            s.d = 6.0;
            s.str = "7";
            s.e = Test::enum2;
            s.p = Test::MyClassPrx::uncheckedCast(communicator->stringToProxy("test:default"));
            arr.push_back(s);
        }
        out = Ice::createOutputStream(communicator);
        out->write(arr);
        out->writePendingObjects();
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test::SmallStructS arr2;
        in->read(arr2);
        in->readPendingObjects();
        test(arr2.size() == arr.size());
        for(Test::SmallStructS::size_type j = 0; j < arr2.size(); ++j)
        {
            test(arr[j] == arr2[j]);
        }

        Test::SmallStructSS arrS;
        arrS.push_back(arr);
        arrS.push_back(Test::SmallStructS());
        arrS.push_back(arr);

        out = Ice::createOutputStream(communicator);
        out->write(arrS);
        out->finished(data);

        in = Ice::createInputStream(communicator, data);
        Test::SmallStructSS arr2S;
        in->read(arr2S);
        test(arr2S == arrS);
    }

    {
        Test::MyClassS arr;
        for(int i = 0; i < 4; ++i)
        {
            Test::MyClassPtr c = new Test::MyClass;
            c->c = c;
            c->o = c;
            c->s.e = Test::enum2;

            c->seq1.push_back(true);
            c->seq1.push_back(false);
            c->seq1.push_back(true);
            c->seq1.push_back(false);

            c->seq2.push_back(1);
            c->seq2.push_back(2);
            c->seq2.push_back(3);
            c->seq2.push_back(4);

            c->seq3.push_back(1);
            c->seq3.push_back(2);
            c->seq3.push_back(3);
            c->seq3.push_back(4);

            c->seq4.push_back(1);
            c->seq4.push_back(2);
            c->seq4.push_back(3);
            c->seq4.push_back(4);

            c->seq5.push_back(1);
            c->seq5.push_back(2);
            c->seq5.push_back(3);
            c->seq5.push_back(4);

            c->seq6.push_back(1);
            c->seq6.push_back(2);
            c->seq6.push_back(3);
            c->seq6.push_back(4);

            c->seq7.push_back(1);
            c->seq7.push_back(2);
            c->seq7.push_back(3);
            c->seq7.push_back(4);

            c->seq8.push_back("string1");
            c->seq8.push_back("string2");
            c->seq8.push_back("string3");
            c->seq8.push_back("string4");

            c->seq9.push_back(Test::enum3);
            c->seq9.push_back(Test::enum2);
            c->seq9.push_back(Test::enum1);

            c->d["hi"] = c;
            c->ice_collectable(true);
            arr.push_back(c);
        }
        out = Ice::createOutputStream(communicator);
        out->write(arr);
        out->writePendingObjects();
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test::MyClassS arr2;
        in->read(arr2);
        in->readPendingObjects();
        test(arr2.size() == arr.size());
        for(Test::MyClassS::size_type j = 0; j < arr2.size(); ++j)
        {
            test(arr2[j]);
            test(arr2[j]->c == arr2[j]);
            test(arr2[j]->o == arr2[j]);
            test(arr2[j]->s.e == Test::enum2);
            test(arr2[j]->seq1 == arr[j]->seq1);
            test(arr2[j]->seq2 == arr[j]->seq2);
            test(arr2[j]->seq3 == arr[j]->seq3);
            test(arr2[j]->seq4 == arr[j]->seq4);
            test(arr2[j]->seq5 == arr[j]->seq5);
            test(arr2[j]->seq6 == arr[j]->seq6);
            test(arr2[j]->seq7 == arr[j]->seq7);
            test(arr2[j]->seq8 == arr[j]->seq8);
            test(arr2[j]->seq9 == arr[j]->seq9);
            test(arr2[j]->d["hi"] == arr2[j]);
        }

        Test::MyClassSS arrS;
        arrS.push_back(arr);
        arrS.push_back(Test::MyClassS());
        arrS.push_back(arr);

        out = Ice::createOutputStream(communicator);
        out->write(arrS);
        out->finished(data);

        in = Ice::createInputStream(communicator, data);
        Test::MyClassSS arr2S;
        in->read(arr2S);
        test(arr2S.size() == arrS.size());
        test(arr2S[0].size() == arrS[0].size());
        test(arr2S[1].size() == arrS[1].size());
        test(arr2S[2].size() == arrS[2].size());
    }

    {
        Test::MyInterfacePtr i = new Test::MyInterface();
        out = Ice::createOutputStream(communicator);
        out->write(i);
        out->writePendingObjects();
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        i = 0;
        in->read(i);
        in->readPendingObjects();
        test(i);
    }

    {
        out = Ice::createOutputStream(communicator);
        Test::MyClassPtr obj = new Test::MyClass;
        obj->s.e = Test::enum2;
        TestObjectWriterPtr writer = new TestObjectWriter(obj);
        out->writeObject(writer);
        out->writePendingObjects();
        out->finished(data);
        test(writer->called);
    }

    {
        out = Ice::createOutputStream(communicator);
        Test::MyClassPtr obj = new Test::MyClass;
        obj->s.e = Test::enum2;
        TestObjectWriterPtr writer = new TestObjectWriter(obj);
        out->writeObject(writer);
        out->writePendingObjects();
        out->finished(data);
        test(writer->called);
        factoryWrapper->setFactory(new TestObjectFactory);
        in = Ice::createInputStream(communicator, data);
        TestReadObjectCallbackPtr cb = new TestReadObjectCallback;
        in->readObject(cb);
        in->readPendingObjects();
        test(cb->obj);
        TestObjectReaderPtr reader = TestObjectReaderPtr::dynamicCast(cb->obj);
        test(reader);
        test(reader->called);
        test(reader->obj);
        test(reader->obj->s.e == Test::enum2);
        factoryWrapper->setFactory(0);
    }

    {
        out = Ice::createOutputStream(communicator);
        Test::MyException ex;
        Test::MyClassPtr c = new Test::MyClass;
        c->c = c;
        c->o = c;
        c->s.e = Test::enum2;

        c->seq1.push_back(true);
        c->seq1.push_back(false);
        c->seq1.push_back(true);
        c->seq1.push_back(false);

        c->seq2.push_back(1);
        c->seq2.push_back(2);
        c->seq2.push_back(3);
        c->seq2.push_back(4);

        c->seq3.push_back(1);
        c->seq3.push_back(2);
        c->seq3.push_back(3);
        c->seq3.push_back(4);

        c->seq4.push_back(1);
        c->seq4.push_back(2);
        c->seq4.push_back(3);
        c->seq4.push_back(4);

        c->seq5.push_back(1);
        c->seq5.push_back(2);
        c->seq5.push_back(3);
        c->seq5.push_back(4);

        c->seq6.push_back(1);
        c->seq6.push_back(2);
        c->seq6.push_back(3);
        c->seq6.push_back(4);

        c->seq7.push_back(1);
        c->seq7.push_back(2);
        c->seq7.push_back(3);
        c->seq7.push_back(4);

        c->seq8.push_back("string1");
        c->seq8.push_back("string2");
        c->seq8.push_back("string3");
        c->seq8.push_back("string4");

        c->seq9.push_back(Test::enum3);
        c->seq9.push_back(Test::enum2);
        c->seq9.push_back(Test::enum1);

        ex.c = c;
        ex.c->ice_collectable(true);

        out->write(ex);
        out->finished(data);

        in = Ice::createInputStream(communicator, data);
        try
        {
            in->throwException();
            test(false);
        }
        catch(const Test::MyException& ex1)
        {
            test(ex1.c->s.e == c->s.e);
            test(ex1.c->seq1 == c->seq1);
            test(ex1.c->seq2 == c->seq2);
            test(ex1.c->seq3 == c->seq3);
            test(ex1.c->seq4 == c->seq4);
            test(ex1.c->seq5 == c->seq5);
            test(ex1.c->seq6 == c->seq6);
            test(ex1.c->seq7 == c->seq7);
            test(ex1.c->seq8 == c->seq8);
            test(ex1.c->seq9 == c->seq9);
        }
    }

    {
        Test::ByteBoolD dict;
        dict[0x04] = true;
        dict[0x01] = false;
        out = Ice::createOutputStream(communicator);
        out->write(dict);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test::ByteBoolD dict2;
        in->read(dict2);
        test(dict2 == dict);
    }

    {
        Test::ShortIntD dict;
        dict[1] = 9;
        dict[4] = 8;
        out = Ice::createOutputStream(communicator);
        out->write(dict);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test::ShortIntD dict2;
        in->read(dict2);
        test(dict2 == dict);
    }

    {
        Test::LongFloatD dict;
        dict[123809828] = 0.51f;
        dict[123809829] = 0.56f;
        out = Ice::createOutputStream(communicator);
        out->write(dict);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test::LongFloatD dict2;
        in->read(dict2);
        test(dict2 == dict);
    }

    {
        Test::StringStringD dict;
        dict["key1"] = "value1";
        dict["key2"] = "value2";
        out = Ice::createOutputStream(communicator);
        out->write(dict);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test::StringStringD dict2;
        in->read(dict2);
        test(dict2 == dict);
    }

    {
        Test::StringMyClassD dict;
        dict["key1"] = new Test::MyClass;
        dict["key1"]->s.e = Test::enum2;
        dict["key2"] = new Test::MyClass;
        dict["key2"]->s.e = Test::enum3;
        out = Ice::createOutputStream(communicator);
        out->write(dict);
        out->writePendingObjects();
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test::StringMyClassD dict2;
        in->read(dict2);
        in->readPendingObjects();
        test(dict2.size() == dict.size());
        test(dict2["key1"] && (dict2["key1"]->s.e == Test::enum2));
        test(dict2["key2"] && (dict2["key2"]->s.e == Test::enum3));
    }

    {
        out = Ice::createOutputStream(communicator);
        out->write(Test::Sub::nestedEnum3);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test::Sub::NestedEnum e;
        in->read(e);
        test(e == Test::Sub::nestedEnum3);
    }

    {
        out = Ice::createOutputStream(communicator);
        Test::Sub::NestedStruct s;
        s.bo = true;
        s.by = 1;
        s.sh = 2;
        s.i = 3;
        s.l = 4;
        s.f = 5.0;
        s.d = 6.0;
        s.str = "7";
        s.e = Test::Sub::nestedEnum2;
        out->write(s);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test::Sub::NestedStruct s2;
        in->read(s2);
        test(s2 == s);
    }

    {
        out = Ice::createOutputStream(communicator);
        Test::Sub::NestedClassStructPtr s = new Test::Sub::NestedClassStruct();
        s->i = 10;
        out->write(s);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test::Sub::NestedClassStructPtr s2 = new Test::Sub::NestedClassStruct();
        in->read(s2);
        test(s2->i == s->i);
    }

    {
        out = Ice::createOutputStream(communicator);
        Test::Sub::NestedException ex;
        ex.str = "str";

        out->write(ex);
        out->finished(data);

        in = Ice::createInputStream(communicator, data);
        try
        {
            in->throwException();
            test(false);
        }
        catch(const Test::Sub::NestedException& ex1)
        {
            test(ex1.str == ex.str);
        }
    }

    {
        out = Ice::createOutputStream(communicator);
        out->write(Test2::Sub2::nestedEnum4);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test2::Sub2::NestedEnum2 e;
        in->read(e);
        test(e == Test2::Sub2::nestedEnum4);
    }

    {
        out = Ice::createOutputStream(communicator);
        Test2::Sub2::NestedStruct2 s;
        s.bo = true;
        s.by = 1;
        s.sh = 2;
        s.i = 3;
        s.l = 4;
        s.f = 5.0;
        s.d = 6.0;
        s.str = "7";
        s.e = Test2::Sub2::nestedEnum5;
        out->write(s);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test2::Sub2::NestedStruct2 s2;
        in->read(s2);
        test(s2 == s);
    }

    {
        out = Ice::createOutputStream(communicator);
        Test2::Sub2::NestedClassStruct2Ptr s = new Test2::Sub2::NestedClassStruct2();
        s->i = 10;
        out->write(s);
        out->finished(data);
        in = Ice::createInputStream(communicator, data);
        Test2::Sub2::NestedClassStruct2Ptr s2 = new Test2::Sub2::NestedClassStruct2();
        in->read(s2);
        test(s2->i == s->i);
    }

    {
        out = Ice::createOutputStream(communicator);
        Test2::Sub2::NestedException2 ex;
        ex.str = "str";

        out->write(ex);
        out->finished(data);

        in = Ice::createInputStream(communicator, data);
        try
        {
            in->throwException();
            test(false);
        }
        catch(const Test2::Sub2::NestedException2& ex1)
        {
            test(ex1.str == ex.str);
        }
    }

    cout << "ok" << endl;
    return 0;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif

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
