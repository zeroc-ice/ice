// Copyright (c) ZeroC, Inc.

#include <complex>
#ifdef _MSC_VER
#    pragma warning(disable : 4244) // '=': conversion from 'int' to 'int16_t', possible loss of data
#endif

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;
using namespace Test::Sub;
using namespace Test2::Sub2;

class TestObjectWriter : public Ice::Value
{
public:
    TestObjectWriter(const MyClassPtr& p)
    {
        obj = p;
        called = false;
    }

    void _iceWrite(Ice::OutputStream* out) const override
    {
        obj->_iceWrite(out);
        const_cast<TestObjectWriter*>(this)->called = true;
    }

    void _iceRead(Ice::InputStream*) override { assert(false); }

    MyClassPtr obj;
    bool called;
};

class TestObjectReader : public Ice::Value
{
public:
    TestObjectReader() { called = false; }

    void _iceWrite(Ice::OutputStream*) const override { assert(false); }

    void _iceRead(Ice::InputStream* in) override
    {
        obj = std::make_shared<MyClass>();
        obj->_iceRead(in);
        called = true;
    }

    MyClassPtr obj;
    bool called;
};

void
patchObject(void* addr, const Ice::ValuePtr& v)
{
    auto* p = static_cast<Ice::ValuePtr*>(addr);
    assert(p);
    *p = v;
}

class CustomSliceLoader final : public Ice::SliceLoader
{
public:
    [[nodiscard]] Ice::ValuePtr newClassInstance(string_view typeId) const final
    {
        if (typeId == MyClass::ice_staticId() && _useReader)
        {
            return make_shared<TestObjectReader>();
        }
        return nullptr;
    }

    void useReader(bool b) { _useReader = b; }

private:
    bool _useReader{false};
};

using CustomSliceLoaderPtr = shared_ptr<CustomSliceLoader>;

void
allTests(Test::TestHelper* helper, const CustomSliceLoaderPtr& customSliceLoader)
{
    Ice::CommunicatorPtr communicator = helper->communicator();

    vector<byte> data;

    //
    // Test the stream API.
    //
    cout << "testing primitive types... " << flush;

    {
        vector<byte> byte;
        Ice::InputStream in(communicator, byte);
    }

    {
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(true);
        out.endEncapsulation();
        out.finished(data);
        pair<const byte*, const byte*> d = out.finished();
        test(d.second - d.first == static_cast<int>(data.size()));
        test(vector<byte>(d.first, d.second) == data);

        Ice::InputStream in(communicator, data);
        in.startEncapsulation();
        bool v;
        in.read(v);
        test(v);
        in.endEncapsulation();
    }

    {
        vector<byte> byte;
        Ice::InputStream in(communicator, byte);
        try
        {
            bool v;
            in.read(v);
            test(false);
        }
        catch (const Ice::MarshalException&)
        {
        }
    }

    {
        Ice::OutputStream out(communicator);
        out.write(true);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        bool v;
        in.read(v);
        test(v);
    }

    {
        Ice::OutputStream out(communicator);
        out.write((uint8_t)1);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        uint8_t v;
        in.read(v);
        test(v == 1);
    }

    {
        Ice::OutputStream out(communicator);
        out.write((int16_t)2);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        int16_t v;
        in.read(v);
        test(v == 2);
    }

    {
        Ice::OutputStream out(communicator);
        out.write((int32_t)3);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        int32_t v;
        in.read(v);
        test(v == 3);
    }

    {
        Ice::OutputStream out(communicator);
        out.write((int64_t)4);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        int64_t v;
        in.read(v);
        test(v == 4);
    }

    {
        Ice::OutputStream out(communicator);
        out.write((float)5.0);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        float v;
        in.read(v);
        test(v == 5.0);
    }

    {
        Ice::OutputStream out(communicator);
        out.write((double)6.0);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        double v;
        in.read(v);
        test(v == 6.0);
    }

    {
        Ice::OutputStream out(communicator);
        out.write("hello world");
        out.finished(data);
        Ice::InputStream in(communicator, data);
        string v;
        in.read(v);
        test(v == "hello world");
    }

    cout << "ok" << endl;

    cout << "testing constructed types... " << flush;

    {
        Ice::OutputStream out(communicator);
        out.write(MyEnum::enum3);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        MyEnum e;
        in.read(e);
        test(e == MyEnum::enum3);
    }

    {
        Ice::OutputStream out(communicator);
        LargeStruct s;
        s.bo = true;
        s.by = 1;
        s.sh = 2;
        s.i = 3;
        s.l = 4;
        s.f = 5.0;
        s.d = 6.0;
        s.str = "7";
        s.e = MyEnum::enum2;
        s.p = MyInterfacePrx(communicator, "test:default");
        out.write(s);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        LargeStruct s2;
        in.read(s2);
        test(s2.p == s.p);
        s2.p = s.p; // otherwise the s2 == s below will fail

        test(s2 == s);
    }

    {
        Ice::OutputStream out(communicator);
        OptionalClassPtr o = std::make_shared<OptionalClass>();
        o->bo = false;
        o->by = 5;
        o->sh = static_cast<int16_t>(4);
        o->i = 3;
        out.write(o);
        out.writePendingValues();
        out.finished(data);
        Ice::InputStream in(communicator, data);
        OptionalClassPtr o2;
        in.read(o2);
        in.readPendingValues();
        test(o2->bo == o->bo);
        test(o2->by == o->by);
        if (in.getEncoding() == Ice::Encoding_1_0)
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
        Ice::OutputStream out(communicator, Ice::Encoding_1_0);
        OptionalClassPtr o = std::make_shared<OptionalClass>();
        o->bo = false;
        o->by = 5;
        o->sh = static_cast<int16_t>(4);
        o->i = 3;
        out.write(o);
        out.writePendingValues();
        out.finished(data);
        Ice::InputStream in(communicator, Ice::Encoding_1_0, data);
        OptionalClassPtr o2;
        in.read(o2);
        in.readPendingValues();
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

        Ice::OutputStream out(communicator);
        out.write(arr);
        out.finished(data);

        Ice::InputStream in(communicator, data);
        Ice::BoolSeq arr2;
        in.read(arr2);
        test(arr2 == arr);

        BoolSS arrS;
        arrS.push_back(arr);
        arrS.emplace_back();
        arrS.push_back(arr);

        Ice::OutputStream out2(communicator);
        out2.write(arrS);
        out2.finished(data);

        Ice::InputStream in2(communicator, data);
        BoolSS arr2S;
        in2.read(arr2S);
        test(arr2S == arrS);
    }

    {
        Ice::ByteSeq arr;
        arr.push_back(byte{0x01});
        arr.push_back(byte{0x11});
        arr.push_back(byte{0x12});
        arr.push_back(byte{0x22});

        Ice::OutputStream out(communicator);
        out.write(arr);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        Ice::ByteSeq arr2;
        in.read(arr2);
        test(arr2 == arr);

        ByteSS arrS;
        arrS.push_back(arr);
        arrS.emplace_back();
        arrS.push_back(arr);

        Ice::OutputStream out2(communicator);
        out2.write(arrS);
        out2.finished(data);

        Ice::InputStream in2(communicator, data);
        ByteSS arr2S;
        in2.read(arr2S);
        test(arr2S == arrS);
    }

    {
        Ice::ShortSeq arr;
        arr.push_back(0x01);
        arr.push_back(0x11);
        arr.push_back(0x12);
        arr.push_back(0x22);
        Ice::OutputStream out(communicator);
        out.write(arr);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        Ice::ShortSeq arr2;
        in.read(arr2);
        test(arr2 == arr);

        ShortSS arrS;
        arrS.push_back(arr);
        arrS.emplace_back();
        arrS.push_back(arr);

        Ice::OutputStream out2(communicator);
        out2.write(arrS);
        out2.finished(data);

        Ice::InputStream in2(communicator, data);
        ShortSS arr2S;
        in2.read(arr2S);
        test(arr2S == arrS);
    }

    {
        Ice::IntSeq arr;
        arr.push_back(0x01);
        arr.push_back(0x11);
        arr.push_back(0x12);
        arr.push_back(0x22);
        Ice::OutputStream out(communicator);
        out.write(arr);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        Ice::IntSeq arr2;
        in.read(arr2);
        test(arr2 == arr);

        IntSS arrS;
        arrS.push_back(arr);
        arrS.emplace_back();
        arrS.push_back(arr);

        Ice::OutputStream out2(communicator);
        out2.write(arrS);
        out2.finished(data);

        Ice::InputStream in2(communicator, data);
        IntSS arr2S;
        in2.read(arr2S);
        test(arr2S == arrS);
    }

    {
        Ice::LongSeq arr;
        arr.push_back(0x01);
        arr.push_back(0x11);
        arr.push_back(0x12);
        arr.push_back(0x22);
        Ice::OutputStream out(communicator);
        out.write(arr);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        Ice::LongSeq arr2;
        in.read(arr2);
        test(arr2 == arr);

        LongSS arrS;
        arrS.push_back(arr);
        arrS.emplace_back();
        arrS.push_back(arr);

        Ice::OutputStream out2(communicator);
        out2.write(arrS);
        out2.finished(data);

        Ice::InputStream in2(communicator, data);
        LongSS arr2S;
        in2.read(arr2S);
        test(arr2S == arrS);
    }

    {
        Ice::FloatSeq arr;
        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(3);
        arr.push_back(4);
        Ice::OutputStream out(communicator);
        out.write(arr);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        Ice::FloatSeq arr2;
        in.read(arr2);
        test(arr2 == arr);

        FloatSS arrS;
        arrS.push_back(arr);
        arrS.emplace_back();
        arrS.push_back(arr);

        Ice::OutputStream out2(communicator);
        out2.write(arrS);
        out2.finished(data);

        Ice::InputStream in2(communicator, data);
        FloatSS arr2S;
        in2.read(arr2S);
        test(arr2S == arrS);
    }

    {
        Ice::DoubleSeq arr;
        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(3);
        arr.push_back(4);
        Ice::OutputStream out(communicator);
        out.write(arr);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        Ice::DoubleSeq arr2;
        in.read(arr2);
        test(arr2 == arr);

        DoubleSS arrS;
        arrS.push_back(arr);
        arrS.emplace_back();
        arrS.push_back(arr);

        Ice::OutputStream out2(communicator);
        out2.write(arrS);
        out2.finished(data);

        Ice::InputStream in2(communicator, data);
        DoubleSS arr2S;
        in2.read(arr2S);
        test(arr2S == arrS);
    }

    {
        Ice::StringSeq arr;
        arr.emplace_back("string1");
        arr.emplace_back("string2");
        arr.emplace_back("string3");
        arr.emplace_back("string4");
        Ice::OutputStream out(communicator);
        out.write(arr);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        Ice::StringSeq arr2;
        in.read(arr2);
        test(arr2 == arr);

        StringSS arrS;
        arrS.push_back(arr);
        arrS.emplace_back();
        arrS.push_back(arr);

        Ice::OutputStream out2(communicator);
        out2.write(arrS);
        out2.finished(data);

        Ice::InputStream in2(communicator, data);
        StringSS arr2S;
        in2.read(arr2S);
        test(arr2S == arrS);
    }

    {
        MyEnumS arr;
        arr.push_back(MyEnum::enum3);
        arr.push_back(MyEnum::enum2);
        arr.push_back(MyEnum::enum1);
        arr.push_back(MyEnum::enum2);

        Ice::OutputStream out(communicator);
        out.write(arr);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        MyEnumS arr2;
        in.read(arr2);
        test(arr2 == arr);

        MyEnumSS arrS;
        arrS.push_back(arr);
        arrS.emplace_back();
        arrS.push_back(arr);

        Ice::OutputStream out2(communicator);
        out2.write(arrS);
        out2.finished(data);

        Ice::InputStream in2(communicator, data);
        MyEnumSS arr2S;
        in2.read(arr2S);
        test(arr2S == arrS);
    }

    {
        LargeStructS arr;
        for (int i = 0; i < 4; ++i)
        {
            LargeStruct s;
            s.bo = true;
            s.by = 1;
            s.sh = 2;
            s.i = 3;
            s.l = 4;
            s.f = 5.0;
            s.d = 6.0;
            s.str = "7";
            s.e = MyEnum::enum2;
            s.p = MyInterfacePrx(communicator, "test:default");
            arr.push_back(s);
        }
        Ice::OutputStream out(communicator);
        out.write(arr);
        out.writePendingValues();
        out.finished(data);
        Ice::InputStream in(communicator, data);
        LargeStructS arr2;
        in.read(arr2);
        in.readPendingValues();
        test(arr2.size() == arr.size());

        for (LargeStructS::size_type j = 0; j < arr2.size(); ++j)
        {
            test(arr[j].p == arr2[j].p);
            arr2[j].p = arr[j].p;
            test(arr[j] == arr2[j]);
        }

        LargeStructSS arrS;
        arrS.push_back(arr);
        arrS.emplace_back();
        arrS.push_back(arr);

        Ice::OutputStream out2(communicator);
        out2.write(arrS);
        out2.finished(data);

        Ice::InputStream in2(communicator, data);
        LargeStructSS arr2S;
        in2.read(arr2S);
    }

    {
        MyClassS arr;
        for (int i = 0; i < 4; ++i)
        {
            MyClassPtr c = std::make_shared<MyClass>();
            c->c = c;
            c->o = c;
            c->s.e = MyEnum::enum2;

            c->seq1.push_back(true);
            c->seq1.push_back(false);
            c->seq1.push_back(true);
            c->seq1.push_back(false);

            c->seq2.push_back(byte{1});
            c->seq2.push_back(byte{2});
            c->seq2.push_back(byte{3});
            c->seq2.push_back(byte{4});

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

            c->seq8.emplace_back("string1");
            c->seq8.emplace_back("string2");
            c->seq8.emplace_back("string3");
            c->seq8.emplace_back("string4");

            c->seq9.push_back(MyEnum::enum3);
            c->seq9.push_back(MyEnum::enum2);
            c->seq9.push_back(MyEnum::enum1);

            c->d["hi"] = c;
            arr.push_back(c);
        }
        Ice::OutputStream out(communicator);
        out.write(arr);
        out.writePendingValues();
        out.finished(data);

        Ice::InputStream in(communicator, data);
        MyClassS arr2;
        in.read(arr2);
        in.readPendingValues();
        test(arr2.size() == arr.size());
        for (MyClassS::size_type j = 0; j < arr2.size(); ++j)
        {
            test(arr2[j]);
            test(arr2[j]->c == arr2[j]);
            test(arr2[j]->o == arr2[j]);
            test(arr2[j]->s.e == MyEnum::enum2);
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

        MyClassSS arrS;
        arrS.push_back(arr);
        arrS.emplace_back();
        arrS.push_back(arr);

        Ice::OutputStream out2(communicator);
        out2.write(arrS);
        out2.writePendingValues();
        out2.finished(data);

        Ice::InputStream in2(communicator, data);
        MyClassSS arr2S;
        in2.read(arr2S);
        in2.readPendingValues();
        test(arr2S.size() == arrS.size());
        test(arr2S[0].size() == arrS[0].size());
        test(arr2S[1].size() == arrS[1].size());
        test(arr2S[2].size() == arrS[2].size());
        for (const auto& j : arr2S)
        {
            for (size_t k = 0; k < j.size(); ++k)
            {
                test(j[k]->c == j[k]);
                test(j[k]->o == j[k]);
                test(j[k]->s.e == MyEnum::enum2);
                test(j[k]->seq1 == arr[k]->seq1);
                test(j[k]->seq2 == arr[k]->seq2);
                test(j[k]->seq3 == arr[k]->seq3);
                test(j[k]->seq4 == arr[k]->seq4);
                test(j[k]->seq5 == arr[k]->seq5);
                test(j[k]->seq6 == arr[k]->seq6);
                test(j[k]->seq7 == arr[k]->seq7);
                test(j[k]->seq8 == arr[k]->seq8);
                test(j[k]->seq9 == arr[k]->seq9);
                test(j[k]->d["hi"] == j[k]);
            }
        }

        auto clearS = [](MyClassS& arr3)
        {
            for (const auto& p : arr3)
            {
                if (p)
                {
                    p->c = nullptr;
                    p->o = nullptr;
                    p->d["hi"] = nullptr;
                }
            }
        };
        auto clearSS = [clearS](MyClassSS& arr3)
        {
            for (auto& p : arr3)
            {
                clearS(p);
            }
        };
        clearS(arr);
        clearS(arr2);
        clearSS(arrS);
        clearSS(arr2S);
    }

    {
        Ice::OutputStream out(communicator);
        MyClassPtr obj = std::make_shared<MyClass>();
        obj->s.e = MyEnum::enum2;
        shared_ptr<TestObjectWriter> writer = make_shared<TestObjectWriter>(obj);
        out.write(writer);
        out.writePendingValues();
        out.finished(data);
        test(writer->called);
    }

    {
        Ice::OutputStream out(communicator);
        MyClassPtr obj = std::make_shared<MyClass>();
        obj->s.e = MyEnum::enum2;
        shared_ptr<TestObjectWriter> writer = make_shared<TestObjectWriter>(obj);
        out.write(writer);
        out.writePendingValues();
        out.finished(data);
        test(writer->called);
        customSliceLoader->useReader(true);
        Ice::InputStream in(communicator, data);
        Ice::ValuePtr p;
        in.read(&patchObject, &p);
        in.readPendingValues();
        test(p);
        shared_ptr<TestObjectReader> reader = dynamic_pointer_cast<TestObjectReader>(p);
        test(reader);
        test(reader->called);
        test(reader->obj);
        test(reader->obj->s.e == MyEnum::enum2);
        customSliceLoader->useReader(false);
    }

    {
        Ice::OutputStream out(communicator);
        MyException ex;
        MyClassPtr c = std::make_shared<MyClass>();
        c->c = c;
        c->o = c;
        c->s.e = MyEnum::enum2;

        c->seq1.push_back(true);
        c->seq1.push_back(false);
        c->seq1.push_back(true);
        c->seq1.push_back(false);

        c->seq2.push_back(byte{1});
        c->seq2.push_back(byte{2});
        c->seq2.push_back(byte{3});
        c->seq2.push_back(byte{4});

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

        c->seq8.emplace_back("string1");
        c->seq8.emplace_back("string2");
        c->seq8.emplace_back("string3");
        c->seq8.emplace_back("string4");

        c->seq9.push_back(MyEnum::enum3);
        c->seq9.push_back(MyEnum::enum2);
        c->seq9.push_back(MyEnum::enum1);

        ex.c = c;

        out.write(ex);
        out.finished(data);

        Ice::InputStream in(communicator, data);
        try
        {
            in.throwException();
            test(false);
        }
        catch (const MyException& ex1)
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

            ex1.c->c = nullptr;
            ex1.c->o = nullptr;
        }

        c->c = nullptr;
        c->o = nullptr;
    }

    {
        ByteBoolD dict;
        dict[0x04] = true;
        dict[0x01] = false;
        Ice::OutputStream out(communicator);
        out.write(dict);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        ByteBoolD dict2;
        in.read(dict2);
        test(dict2 == dict);
    }

    {
        ShortIntD dict;
        dict[1] = 9;
        dict[4] = 8;
        Ice::OutputStream out(communicator);
        out.write(dict);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        ShortIntD dict2;
        in.read(dict2);
        test(dict2 == dict);
    }

    {
        LongFloatD dict;
        dict[123809828] = 0.51f;
        dict[123809829] = 0.56f;
        Ice::OutputStream out(communicator);
        out.write(dict);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        LongFloatD dict2;
        in.read(dict2);
        test(dict2 == dict);
    }

    {
        StringStringD dict;
        dict["key1"] = "value1";
        dict["key2"] = "value2";
        Ice::OutputStream out(communicator);
        out.write(dict);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        StringStringD dict2;
        in.read(dict2);
        test(dict2 == dict);
    }

    {
        StringMyClassD dict;
        dict["key1"] = std::make_shared<MyClass>();
        dict["key1"]->s.e = MyEnum::enum2;
        dict["key2"] = std::make_shared<MyClass>();
        dict["key2"]->s.e = MyEnum::enum3;
        Ice::OutputStream out(communicator);
        out.write(dict);
        out.writePendingValues();
        out.finished(data);
        Ice::InputStream in(communicator, data);
        StringMyClassD dict2;
        in.read(dict2);
        in.readPendingValues();
        test(dict2.size() == dict.size());
        test(dict2["key1"] && (dict2["key1"]->s.e == MyEnum::enum2));
        test(dict2["key2"] && (dict2["key2"]->s.e == MyEnum::enum3));
    }

    {
        Ice::OutputStream out(communicator);
        out.write(Sub::NestedEnum::nestedEnum3);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        NestedEnum e;
        in.read(e);
        test(e == Sub::NestedEnum::nestedEnum3);
    }

    {
        Ice::OutputStream out(communicator);
        NestedStruct s;
        s.bo = true;
        s.by = 1;
        s.sh = 2;
        s.i = 3;
        s.l = 4;
        s.f = 5.0;
        s.d = 6.0;
        s.str = "7";
        s.e = Sub::NestedEnum::nestedEnum2;
        out.write(s);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        NestedStruct s2;
        in.read(s2);
        test(s2 == s);
    }

    {
        Ice::OutputStream out(communicator);
        NestedException ex;
        ex.str = "str";

        out.write(ex);
        out.finished(data);

        Ice::InputStream in(communicator, data);
        try
        {
            in.throwException();
            test(false);
        }
        catch (const NestedException& ex1)
        {
            test(ex1.str == ex.str);
        }
    }

    {
        Ice::OutputStream out(communicator);
        out.write(NestedEnum2::nestedEnum4);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        NestedEnum2 e;
        in.read(e);
        test(e == NestedEnum2::nestedEnum4);
    }

    {
        Ice::OutputStream out(communicator);
        NestedStruct2 s;
        s.bo = true;
        s.by = 1;
        s.sh = 2;
        s.i = 3;
        s.l = 4;
        s.f = 5.0;
        s.d = 6.0;
        s.str = "7";
        s.e = NestedEnum2::nestedEnum5;
        out.write(s);
        out.finished(data);
        Ice::InputStream in(communicator, data);
        NestedStruct2 s2;
        in.read(s2);
        test(s2 == s);
    }

    {
        Ice::OutputStream out(communicator);
        NestedException2 ex;
        ex.str = "str";

        out.write(ex);
        out.finished(data);

        Ice::InputStream in(communicator, data);
        try
        {
            in.throwException();
            test(false);
        }
        catch (const NestedException2& ex1)
        {
            test(ex1.str == ex.str);
        }
    }

    //
    // Test marshaling to user-supplied buffer.
    //
    {
        byte buf[128];
        pair<byte*, byte*> p(&buf[0], &buf[0] + sizeof(buf));
        Ice::OutputStream out(p);
        vector<byte> v;
        v.resize(127);
        out.write(v);
        test(out.pos() == 128);     // 127 bytes + leading size (1 byte)
        test(out.b.begin() == buf); // Verify the stream hasn't reallocated.
    }
    {
        byte buf[128];
        pair<byte*, byte*> p(&buf[0], &buf[0] + sizeof(buf));
        Ice::OutputStream out(p);
        vector<byte> v;
        v.resize(127);
        ::memset(&v[0], 0xFF, v.size());
        out.write(v);
        out.write(uint8_t{0xFF});   // This extra byte should make the stream reallocate.
        test(out.pos() == 129);     // 127 bytes + leading size (1 byte) + 1 byte
        test(out.b.begin() != buf); // Verify the stream was reallocated.
        out.finished(data);

        Ice::InputStream in(communicator, data);
        vector<byte> v2;
        in.read(v2);
        test(v2.size() == 127);
        test(v == v2); // Make sure the original buffer was preserved.
    }

    cout << "ok" << endl;
}

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    initData.properties->setProperty("Ice.AcceptClassCycles", "1");
    auto customSliceLoader = make_shared<CustomSliceLoader>();
    initData.sliceLoader = customSliceLoader;
    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);
    allTests(this, customSliceLoader);
}

DEFINE_TEST(Client)
