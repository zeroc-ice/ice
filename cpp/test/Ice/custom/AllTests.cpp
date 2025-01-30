// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"
#include "Wstring.h"
#include <iterator>

#ifdef _MSC_VER
#    pragma warning(4 : 4503) // C4503: ... : decorated name length exceeded, name was truncated
#endif

using namespace std;
using namespace Ice;
using namespace Test;

namespace
{
    template<typename T> bool arrayRangeEquals(pair<const T*, const T*> lhs, pair<const T*, const T*> rhs)
    {
        if (lhs.second - lhs.first != rhs.second - rhs.first)
        {
            return false;
        }

        T* l = const_cast<T*>(lhs.first);
        T* r = const_cast<T*>(rhs.first);
        while (l != lhs.second)
        {
            if (*l++ != *r++)
            {
                return false;
            }
        }
        return true;
    }
}

TestIntfPrx
allTests(TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();
    const string endp = helper->getTestEndpoint();
    TestIntfPrx t(communicator, communicator->getProperties()->getPropertyWithDefault("Custom.Proxy", "test:" + endp));

    cout << "testing ice_id and ice_ids with string converter... " << flush;
    test(t->ice_id() == TestIntfPrx::ice_staticId());
    test(t->ice_ids()[0] == ObjectPrx::ice_staticId());
    test(t->ice_ids()[1] == TestIntfPrx::ice_staticId());
    cout << "ok" << endl;

    cout << "testing alternate sequences... " << flush;

    {
        ShortSeq in(50);
        for (size_t i = 0; i < in.size(); ++i)
        {
            in[i] = static_cast<int16_t>(i + 1);
        }
        pair<const int16_t*, const int16_t*> inPair(in.data(), in.data() + in.size());

        ShortSeq out;
        ShortSeq ret = t->opShortArray(inPair, out);
        test(out == in);
        test(ret == in);
    }

    {
        DoubleSeq in(5);
        in[0] = 3.14;
        in[1] = 1 / 3;
        in[2] = 0.375;
        in[3] = 4 / 3;
        in[4] = -5.725;
        double inArray[5];
        for (size_t i = 0; i < 5; ++i)
        {
            inArray[i] = in[i];
        }
        pair<const double*, const double*> inPair(inArray, inArray + 5);

        DoubleSeq out;
        DoubleSeq ret = t->opDoubleArray(false, inPair, out);
        test(out == in);
        test(ret == in);
    }

    {
        BoolSeq in(5);
        in[0] = false;
        in[1] = true;
        in[2] = true;
        in[3] = false;
        in[4] = true;
        bool inArray[5];
        for (size_t i = 0; i < 5; ++i)
        {
            inArray[i] = in[i];
        }
        pair<const bool*, const bool*> inPair(inArray, inArray + 5);

        BoolSeq out;
        BoolSeq ret = t->opBoolArray(inPair, out);
        test(out == in);
        test(ret == in);
    }

    {
        ByteList in;
        byte inArray[5];
        inArray[0] = byte{'1'};
        in.push_back(inArray[0]);
        inArray[1] = byte{'2'};
        in.push_back(inArray[1]);
        inArray[2] = byte{'3'};
        in.push_back(inArray[2]);
        inArray[3] = byte{'4'};
        in.push_back(inArray[3]);
        inArray[4] = byte{'5'};
        in.push_back(inArray[4]);
        pair<const byte*, const byte*> inPair(inArray, inArray + 5);

        ByteList out;
        ByteList ret = t->opByteArray(inPair, out);
        test(out == in);
        test(ret == in);
    }

    {
        VariableList in;
        Variable inArray[5];
        inArray[0].s = "THESE";
        in.push_back(inArray[0]);
        inArray[1].s = "ARE";
        in.push_back(inArray[1]);
        inArray[2].s = "FIVE";
        in.push_back(inArray[2]);
        inArray[3].s = "SHORT";
        in.push_back(inArray[3]);
        inArray[4].s = "STRINGS.";
        in.push_back(inArray[4]);
        pair<const Variable*, const Variable*> inPair(inArray, inArray + 5);

        VariableList out;
        VariableList ret = t->opVariableArray(inPair, out);

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
        deque<byte> in(5);
        in[0] = byte{'1'};
        in[1] = byte{'2'};
        in[2] = byte{'3'};
        in[3] = byte{'4'};
        in[4] = byte{'5'};

        deque<byte> out;
        deque<byte> ret = t->opByteSeq(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        list<byte> in;
        in.push_back(byte{'1'});
        in.push_back(byte{'2'});
        in.push_back(byte{'3'});
        in.push_back(byte{'4'});
        in.push_back(byte{'5'});

        list<byte> out;
        list<byte> ret = t->opByteList(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        MyByteSeq in(5);
        int i = 0;
        for (auto& p : in)
        {
            p = static_cast<byte>('1' + i++);
        }

        MyByteSeq out;
        MyByteSeq ret = t->opMyByteSeq(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        deque<string> in(5);
        in[0] = "THESE";
        in[1] = "ARE";
        in[2] = "FIVE";
        in[3] = "SHORT";
        in[4] = "STRINGS.";

        deque<string> out;
        deque<string> ret = t->opStringSeq(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        list<string> in;
        in.emplace_back("THESE");
        in.emplace_back("ARE");
        in.emplace_back("FIVE");
        in.emplace_back("SHORT");
        in.emplace_back("STRINGS.");

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
        for (auto& p : in)
        {
            p.s = num++;
        }

        list<Test::Fixed> out;
        list<Test::Fixed> ret = t->opFixedList(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        deque<Variable> in(5);
        in[0].s = "THESE";
        in[1].s = "ARE";
        in[2].s = "FIVE";
        in[3].s = "SHORT";
        in[4].s = "STRINGS.";

        deque<Variable> out;
        deque<Variable> ret = t->opVariableSeq(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        list<Variable> in;
        Variable v;
        v.s = "THESE";
        in.push_back(v);
        v.s = "ARE";
        in.push_back(v);
        v.s = "FIVE";
        in.push_back(v);
        v.s = "SHORT";
        in.push_back(v);
        v.s = "STRINGS.";
        in.push_back(v);

        list<Variable> out;
        list<Variable> ret = t->opVariableList(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        deque<StringStringDict> in(5);
        in[0]["A"] = "A";
        in[1]["B"] = "B";
        in[2]["C"] = "C";
        in[3]["D"] = "D";
        in[4]["E"] = "E";

        deque<StringStringDict> out;
        deque<StringStringDict> ret = t->opStringStringDictSeq(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        list<StringStringDict> in;
        StringStringDict ssd;
        ssd["A"] = "A";
        in.push_back(ssd);
        ssd["B"] = "B";
        in.push_back(ssd);
        ssd["C"] = "C";
        in.push_back(ssd);
        ssd["D"] = "D";
        in.push_back(ssd);
        ssd["E"] = "E";
        in.push_back(ssd);

        list<StringStringDict> out;
        list<StringStringDict> ret = t->opStringStringDictList(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        deque<E> in(5);
        in[0] = E::E1;
        in[1] = E::E2;
        in[2] = E::E3;
        in[3] = E::E1;
        in[4] = E::E3;

        deque<E> out;
        deque<E> ret = t->opESeq(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        list<E> in;
        in.push_back(E::E1);
        in.push_back(E::E2);
        in.push_back(E::E3);
        in.push_back(E::E1);
        in.push_back(E::E3);

        list<E> out;
        list<E> ret = t->opEList(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        deque<optional<DPrx>> in(5);
        in[0] = DPrx(communicator, "C1:" + endp + " -t 10000");
        in[1] = DPrx(communicator, "C2:" + endp + " -t 10001");
        in[2] = DPrx(communicator, "C3:" + endp + " -t 10002");
        in[3] = DPrx(communicator, "C4:" + endp + " -t 10003");
        in[4] = DPrx(communicator, "C5:" + endp + " -t 10004");

        deque<optional<DPrx>> out;
        deque<optional<DPrx>> ret = t->opDPrxSeq(in, out);

        auto op = out.begin();
        auto rp = ret.begin();

        for (const auto& i : in)
        {
            test(*op++ == i);
            test(*rp++ == i);
        }
    }

    {
        list<optional<DPrx>> in;
        in.emplace_back(DPrx(communicator, "C1:" + endp + " -t 10000"));
        in.emplace_back(DPrx(communicator, "C2:" + endp + " -t 10001"));
        in.emplace_back(DPrx(communicator, "C3:" + endp + " -t 10002"));
        in.emplace_back(DPrx(communicator, "C4:" + endp + " -t 10003"));
        in.emplace_back(DPrx(communicator, "C5:" + endp + " -t 10004"));

        list<optional<DPrx>> out;
        list<optional<DPrx>> ret = t->opDPrxList(in, out);
        auto op = out.begin();
        auto rp = ret.begin();

        for (const auto& i : in)
        {
            test(*op++ == i);
            test(*rp++ == i);
        }
    }

    {
        deque<CPtr> in(5);
        in[0] = make_shared<C>();
        in[1] = in[0];
        in[2] = in[0];
        in[3] = in[0];
        in[4] = in[0];

        deque<CPtr> out;
        deque<CPtr> ret = t->opCSeq(in, out);
        test(out.size() == in.size());
        test(ret.size() == in.size());
        for (unsigned int i = 1; i < in.size(); ++i)
        {
            test(out[i] == out[0]);
            test(ret[i] == out[i]);
        }
    }

    {
        list<CPtr> in;
        in.push_back(make_shared<C>());
        in.push_back(make_shared<C>());
        in.push_back(make_shared<C>());
        in.push_back(make_shared<C>());
        in.push_back(make_shared<C>());

        list<CPtr> out;
        list<CPtr> ret = t->opCList(in, out);
        test(out.size() == in.size());
        test(ret.size() == in.size());
        list<CPtr>::const_iterator p1;
        list<CPtr>::const_iterator p2;
        for (p1 = out.begin(), p2 = ret.begin(); p1 != out.end(); ++p1, ++p2)
        {
            test(*p1 == *p2);
        }
    }

    cout << "ok" << endl;

    cout << "testing alternate dictionaries... " << flush;

    {
        IntStringDict idict;

        idict[1] = "ONE";
        idict[2] = "TWO";
        idict[3] = "THREE";
        idict[-1] = "MINUS ONE";

        IntStringDict out;
        out[5] = "FIVE";

        IntStringDict ret = t->opIntStringDict(idict, out);

        test(out == idict);
        test(ret == idict);
    }

    {
        CustomMap<std::string, int32_t> idict;

        idict["ONE"] = 1;
        idict["TWO"] = 2;
        idict["THREE"] = 3;
        idict["MINUS ONE"] = -1;

        CustomMap<std::string, int32_t> out;
        out["FIVE"] = 5;

        CustomMap<int64_t, int64_t> ret = t->opVarDict(idict, out);

        test(out == idict);

        test(ret.size() == 1000);
        for (auto i = ret.begin(); i != ret.end(); ++i)
        {
            test(i->second == i->first * i->first);
        }
    }

    cout << "ok" << endl;

    cout << "testing alternate custom sequences... " << flush;
    {
        ShortBuffer inS;
        inS.setAndInit(new int16_t[3], 3);
        ShortBuffer outS;
        ShortBuffer ret = t->opShortBuffer(inS, outS);

        test(outS == inS);
        test(ret == inS);

        CustomBuffer<bool> inBS;
        inBS.setAndInit(new bool[2], 2);

        CustomBuffer<bool> outBS;
        CustomBuffer<bool> retBS = t->opBoolBuffer(inBS, outBS);

        test(outBS == inBS);
        test(retBS == inBS);

        BufferStruct bs;
        bs.byteBuf.setAndInit(new byte[10], 10);
        bs.boolBuf.setAndInit(new bool[10], 10);
        bs.shortBuf.setAndInit(new int16_t[10], 10);
        bs.intBuf.setAndInit(new int32_t[10], 10);
        bs.longBuf.setAndInit(new int64_t[10], 10);
        bs.floatBuf.setAndInit(new float[10], 10);
        bs.doubleBuf.setAndInit(new double[10], 10);

        BufferStruct rs = t->opBufferStruct(bs);
        test(rs == bs);

        OutputStream os(communicator);
        os.write(rs);
        ByteSeq bytes;
        os.finished(bytes);

        InputStream is(communicator, os.getEncoding(), bytes);
        BufferStruct rs2;
        is.read(rs2);
        test(rs == rs2);
    }
    cout << "ok" << endl;

    cout << "testing alternate sequences with AMI... " << flush;
    {
        {
            ShortSeq in(50);
            for (size_t i = 0; i < in.size(); ++i)
            {
                in[i] = static_cast<int16_t>(i + 1);
            }
            pair<const int16_t*, const int16_t*> inPair(in.data(), in.data() + in.size());

            auto r = t->opShortArrayAsync(inPair).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            DoubleSeq in(5);
            in[0] = 3.14;
            in[1] = 1 / 3;
            in[2] = 0.375;
            in[3] = 4 / 3;
            in[4] = -5.725;
            double inArray[5];
            for (size_t i = 0; i < 5; ++i)
            {
                inArray[i] = in[i];
            }
            pair<const double*, const double*> inPair(inArray, inArray + 5);
            auto r = t->opDoubleArrayAsync(false, inPair).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            BoolSeq in(5);
            in[0] = false;
            in[1] = true;
            in[2] = true;
            in[3] = false;
            in[4] = true;
            bool inArray[5];
            for (size_t i = 0; i < 5; ++i)
            {
                inArray[i] = in[i];
            }
            pair<const bool*, const bool*> inPair(inArray, inArray + 5);

            auto r = t->opBoolArrayAsync(inPair).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            ByteList in;
            byte inArray[5];
            inArray[0] = byte{'1'};
            in.push_back(inArray[0]);
            inArray[1] = byte{'2'};
            in.push_back(inArray[1]);
            inArray[2] = byte{'3'};
            in.push_back(inArray[2]);
            inArray[3] = byte{'4'};
            in.push_back(inArray[3]);
            inArray[4] = byte{'5'};
            in.push_back(inArray[4]);
            pair<const byte*, const byte*> inPair(inArray, inArray + 5);

            auto r = t->opByteArrayAsync(inPair).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            VariableList in;
            Variable inArray[5];
            inArray[0].s = "THESE";
            in.push_back(inArray[0]);
            inArray[1].s = "ARE";
            in.push_back(inArray[1]);
            inArray[2].s = "FIVE";
            in.push_back(inArray[2]);
            inArray[3].s = "SHORT";
            in.push_back(inArray[3]);
            inArray[4].s = "STRINGS.";
            in.push_back(inArray[4]);
            pair<const Variable*, const Variable*> inPair(inArray, inArray + 5);

            auto r = t->opVariableArrayAsync(inPair).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            deque<bool> in(5);
            in[0] = false;
            in[1] = true;
            in[2] = true;
            in[3] = false;
            in[4] = true;

            auto r = t->opBoolSeqAsync(in).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            list<bool> in;
            in.push_back(false);
            in.push_back(true);
            in.push_back(true);
            in.push_back(false);
            in.push_back(true);

            auto r = t->opBoolListAsync(in).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            deque<byte> in(5);
            in[0] = byte{'1'};
            in[1] = byte{'2'};
            in[2] = byte{'3'};
            in[3] = byte{'4'};
            in[4] = byte{'5'};

            auto r = t->opByteSeqAsync(in).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            list<byte> in;
            in.push_back(byte{'1'});
            in.push_back(byte{'2'});
            in.push_back(byte{'3'});
            in.push_back(byte{'4'});
            in.push_back(byte{'5'});

            auto r = t->opByteListAsync(in).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            MyByteSeq in(5);
            int i = 0;
            for (auto& p : in)
            {
                p = static_cast<byte>('1' + i++);
            }

            auto r = t->opMyByteSeqAsync(in).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            deque<string> in(5);
            in[0] = "THESE";
            in[1] = "ARE";
            in[2] = "FIVE";
            in[3] = "SHORT";
            in[4] = "STRINGS.";

            auto r = t->opStringSeqAsync(in).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            list<string> in;
            in.emplace_back("THESE");
            in.emplace_back("ARE");
            in.emplace_back("FIVE");
            in.emplace_back("SHORT");
            in.emplace_back("STRINGS.");

            auto r = t->opStringListAsync(in).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            deque<Test::Fixed> in(5);
            in[0].s = 1;
            in[1].s = 2;
            in[2].s = 3;
            in[3].s = 4;
            in[4].s = 5;

            auto r = t->opFixedSeqAsync(in).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            list<Test::Fixed> in(5);
            short num = 1;
            for (auto& p : in)
            {
                p.s = num++;
            }

            auto r = t->opFixedListAsync(in).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            deque<Variable> in(5);
            in[0].s = "THESE";
            in[1].s = "ARE";
            in[2].s = "FIVE";
            in[3].s = "SHORT";
            in[4].s = "STRINGS.";

            auto r = t->opVariableSeqAsync(in).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            list<Variable> in;
            Variable v;
            v.s = "THESE";
            in.push_back(v);
            v.s = "ARE";
            in.push_back(v);
            v.s = "FIVE";
            in.push_back(v);
            v.s = "SHORT";
            in.push_back(v);
            v.s = "STRINGS.";
            in.push_back(v);

            auto r = t->opVariableListAsync(in).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            deque<StringStringDict> in(5);
            in[0]["A"] = "A";
            in[1]["B"] = "B";
            in[2]["C"] = "C";
            in[3]["D"] = "D";
            in[4]["E"] = "E";

            auto r = t->opStringStringDictSeqAsync(in).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            list<StringStringDict> in;
            StringStringDict ssd;
            ssd["A"] = "A";
            in.push_back(ssd);
            ssd["B"] = "B";
            in.push_back(ssd);
            ssd["C"] = "C";
            in.push_back(ssd);
            ssd["D"] = "D";
            in.push_back(ssd);
            ssd["E"] = "E";
            in.push_back(ssd);

            auto r = t->opStringStringDictListAsync(in).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            deque<E> in(5);
            in[0] = E::E1;
            in[1] = E::E2;
            in[2] = E::E3;
            in[3] = E::E1;
            in[4] = E::E3;

            auto r = t->opESeqAsync(in).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            list<E> in;
            in.push_back(E::E1);
            in.push_back(E::E2);
            in.push_back(E::E3);
            in.push_back(E::E1);
            in.push_back(E::E3);

            auto r = t->opEListAsync(in).get();
            test(std::get<1>(r) == in);
            test(std::get<0>(r) == in);
        }

        {
            deque<optional<DPrx>> in(5);
            in[0] = DPrx(communicator, "C1:" + endp + " -t 10000");
            in[1] = DPrx(communicator, "C2:" + endp + " -t 10001");
            in[2] = DPrx(communicator, "C3:" + endp + " -t 10002");
            in[3] = DPrx(communicator, "C4:" + endp + " -t 10003");
            in[4] = DPrx(communicator, "C5:" + endp + " -t 10004");

            auto r = t->opDPrxSeqAsync(in).get();

            test(std::get<1>(r).size() == in.size());
            test(std::get<0>(r).size() == in.size());

            auto op = std::get<1>(r).begin();
            auto rp = std::get<0>(r).begin();

            for (const auto& i : in)
            {
                test(*op++ == i);
                test(*rp++ == i);
            }
        }

        {
            list<optional<DPrx>> in;
            in.emplace_back(DPrx(communicator, "C1:" + endp + " -t 10000"));
            in.emplace_back(DPrx(communicator, "C2:" + endp + " -t 10001"));
            in.emplace_back(DPrx(communicator, "C3:" + endp + " -t 10002"));
            in.emplace_back(DPrx(communicator, "C4:" + endp + " -t 10003"));
            in.emplace_back(DPrx(communicator, "C5:" + endp + " -t 10004"));

            auto r = t->opDPrxListAsync(in).get();

            test(std::get<1>(r).size() == in.size());
            test(std::get<0>(r).size() == in.size());

            auto op = std::get<1>(r).begin();
            auto rp = std::get<0>(r).begin();

            for (const auto& i : in)
            {
                test(*op++ == i);
                test(*rp++ == i);
            }
        }

        {
            deque<CPtr> in(5);
            in[0] = make_shared<C>();
            in[1] = in[0];
            in[2] = in[0];
            in[3] = in[0];
            in[4] = in[0];

            auto r = t->opCSeqAsync(in).get();
            test(std::get<1>(r).size() == in.size());
            test(std::get<0>(r).size() == in.size());

            auto rp = std::get<0>(r).begin();
            for (const auto& o : std::get<1>(r))
            {
                test(o == std::get<1>(r)[0]);
                test(*rp++ == o);
            }
        }

        {
            list<CPtr> in;
            in.push_back(make_shared<C>());
            in.push_back(make_shared<C>());
            in.push_back(make_shared<C>());
            in.push_back(make_shared<C>());
            in.push_back(make_shared<C>());

            auto r = t->opCListAsync(in).get();
            test(std::get<1>(r).size() == in.size());
            test(std::get<0>(r).size() == in.size());
            test(std::get<1>(r) == std::get<0>(r));
        }

        {
            ByteSeq in;
            in.push_back(byte{'1'});
            in.push_back(byte{'2'});
            in.push_back(byte{'3'});
            in.push_back(byte{'4'});

            auto r = t->opOutArrayByteSeqAsync(in).get();
            test(r.size() == in.size());
            test(r == in);
        }
    }
    cout << "ok" << endl;

    cout << "testing alternate sequences with AMI callbacks... " << flush;

    {
        DoubleSeq in(5);
        in[0] = 3.14;
        in[1] = 1 / 3;
        in[2] = 0.375;
        in[3] = 4 / 3;
        in[4] = -5.725;
        double inArray[5];
        for (size_t i = 0; i < 5; ++i)
        {
            inArray[i] = in[i];
        }
        pair<const double*, const double*> inPair(inArray, inArray + 5);

        promise<bool> done;

        t->opDoubleArrayAsync(
            false,
            inPair,
            [&](pair<const double*, const double*> ret, pair<const double*, const double*> out)
            {
                test(arrayRangeEquals<double>(out, inPair));
                test(arrayRangeEquals<double>(ret, inPair));
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        BoolSeq in(5);
        in[0] = false;
        in[1] = true;
        in[2] = true;
        in[3] = false;
        in[4] = true;
        bool inArray[5];
        for (size_t i = 0; i < 5; ++i)
        {
            inArray[i] = in[i];
        }
        pair<const bool*, const bool*> inPair(inArray, inArray + 5);

        promise<bool> done;

        t->opBoolArrayAsync(
            inPair,
            [&](pair<const bool*, const bool*> ret, pair<const bool*, const bool*> out)
            {
                test(arrayRangeEquals<bool>(out, inPair));
                test(arrayRangeEquals<bool>(ret, inPair));
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        byte in[5];
        in[0] = byte{'1'};
        in[1] = byte{'2'};
        in[2] = byte{'3'};
        in[3] = byte{'4'};
        in[4] = byte{'5'};
        pair<const byte*, const byte*> inPair(in, in + 5);

        promise<bool> done;

        t->opByteArrayAsync(
            inPair,
            [&](pair<const byte*, const byte*> ret, pair<const byte*, const byte*> out)
            {
                test(arrayRangeEquals<byte>(out, inPair));
                test(arrayRangeEquals<byte>(ret, inPair));
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        VariableList in;
        Variable inArray[5];
        inArray[0].s = "THESE";
        in.push_back(inArray[0]);
        inArray[1].s = "ARE";
        in.push_back(inArray[1]);
        inArray[2].s = "FIVE";
        in.push_back(inArray[2]);
        inArray[3].s = "SHORT";
        in.push_back(inArray[3]);
        inArray[4].s = "STRINGS.";
        in.push_back(inArray[4]);
        pair<const Variable*, const Variable*> inPair(inArray, inArray + 5);

        promise<bool> done;

        t->opVariableArrayAsync(
            inPair,
            [&](pair<const Variable*, const Variable*> ret, pair<const Variable*, const Variable*> out)
            {
                test(arrayRangeEquals<Variable>(out, inPair));
                test(arrayRangeEquals<Variable>(ret, inPair));
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        deque<bool> in(5);
        in[0] = false;
        in[1] = true;
        in[2] = true;
        in[3] = false;
        in[4] = true;

        promise<bool> done;

        t->opBoolSeqAsync(
            in,
            [&](const deque<bool>& ret, const deque<bool>& out)
            {
                test(ret == out);
                test(ret == in);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        list<bool> in;
        in.push_back(false);
        in.push_back(true);
        in.push_back(true);
        in.push_back(false);
        in.push_back(true);

        promise<bool> done;

        t->opBoolListAsync(
            in,
            [&](const list<bool>& ret, const list<bool>& out)
            {
                test(ret == out);
                test(ret == in);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        deque<byte> in(5);
        in[0] = byte{'1'};
        in[1] = byte{'2'};
        in[2] = byte{'3'};
        in[3] = byte{'4'};
        in[4] = byte{'5'};

        promise<bool> done;

        t->opByteSeqAsync(
            in,
            [&](const deque<byte>& ret, const deque<byte>& out)
            {
                test(ret == out);
                test(ret == in);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        list<byte> in;
        in.push_back(byte{'1'});
        in.push_back(byte{'2'});
        in.push_back(byte{'3'});
        in.push_back(byte{'4'});
        in.push_back(byte{'5'});

        promise<bool> done;

        t->opByteListAsync(
            in,
            [&](const list<byte>& ret, const list<byte>& out)
            {
                test(ret == out);
                test(ret == in);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        MyByteSeq in(5);
        int i = 0;
        for (auto& p : in)
        {
            p = static_cast<byte>('1' + i++);
        }

        promise<bool> done;

        t->opMyByteSeqAsync(
            in,
            [&](const MyByteSeq& ret, const MyByteSeq& out)
            {
                test(ret == out);
                test(ret == in);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        deque<string> in(5);
        in[0] = "THESE";
        in[1] = "ARE";
        in[2] = "FIVE";
        in[3] = "SHORT";
        in[4] = "STRINGS.";

        promise<bool> done;

        t->opStringSeqAsync(
            in,
            [&](const deque<string>& ret, const deque<string>& out)
            {
                test(ret == out);
                test(ret == in);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        list<string> in;
        in.emplace_back("THESE");
        in.emplace_back("ARE");
        in.emplace_back("FIVE");
        in.emplace_back("SHORT");
        in.emplace_back("STRINGS.");

        promise<bool> done;

        t->opStringListAsync(
            in,
            [&](const list<string>& ret, const list<string>& out)
            {
                test(ret == out);
                test(ret == in);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        deque<Test::Fixed> in(5);
        in[0].s = 1;
        in[1].s = 2;
        in[2].s = 3;
        in[3].s = 4;
        in[4].s = 5;

        promise<bool> done;

        t->opFixedSeqAsync(
            in,
            [&](const deque<Test::Fixed>& ret, const deque<Test::Fixed>& out)
            {
                test(ret == out);
                test(ret == in);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        list<Test::Fixed> in(5);
        short num = 1;
        for (auto& p : in)
        {
            p.s = num++;
        }

        promise<bool> done;

        t->opFixedListAsync(
            in,
            [&](const list<Test::Fixed>& ret, const list<Test::Fixed>& out)
            {
                test(ret == out);
                test(ret == in);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        deque<Variable> in(5);
        in[0].s = "THESE";
        in[1].s = "ARE";
        in[2].s = "FIVE";
        in[3].s = "SHORT";
        in[4].s = "STRINGS.";

        promise<bool> done;

        t->opVariableSeqAsync(
            in,
            [&](const deque<Variable>& ret, const deque<Variable>& out)
            {
                test(ret == out);
                test(ret == in);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        list<Variable> in;
        Variable v;
        v.s = "THESE";
        in.push_back(v);
        v.s = "ARE";
        in.push_back(v);
        v.s = "FIVE";
        in.push_back(v);
        v.s = "SHORT";
        in.push_back(v);
        v.s = "STRINGS.";
        in.push_back(v);

        promise<bool> done;

        t->opVariableListAsync(
            in,
            [&](const list<Variable>& ret, const list<Variable>& out)
            {
                test(ret == out);
                test(ret == in);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        deque<StringStringDict> in(5);
        in[0]["A"] = "A";
        in[1]["B"] = "B";
        in[2]["C"] = "C";
        in[3]["D"] = "D";
        in[4]["E"] = "E";

        promise<bool> done;

        t->opStringStringDictSeqAsync(
            in,
            [&](const deque<StringStringDict>& ret, const deque<StringStringDict>& out)
            {
                test(ret == out);
                test(ret == in);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        list<StringStringDict> in;
        StringStringDict ssd;
        ssd["A"] = "A";
        in.push_back(ssd);
        ssd["B"] = "B";
        in.push_back(ssd);
        ssd["C"] = "C";
        in.push_back(ssd);
        ssd["D"] = "D";
        in.push_back(ssd);
        ssd["E"] = "E";
        in.push_back(ssd);

        promise<bool> done;

        t->opStringStringDictListAsync(
            in,
            [&](const list<StringStringDict>& ret, const list<StringStringDict>& out)
            {
                test(ret == out);
                test(ret == in);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        deque<E> in(5);
        in[0] = E::E1;
        in[1] = E::E2;
        in[2] = E::E3;
        in[3] = E::E1;
        in[4] = E::E3;

        promise<bool> done;

        t->opESeqAsync(
            in,
            [&](const deque<E>& ret, const deque<E>& out)
            {
                test(ret == out);
                test(ret == in);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        list<E> in;
        in.push_back(E::E1);
        in.push_back(E::E2);
        in.push_back(E::E3);
        in.push_back(E::E1);
        in.push_back(E::E3);

        promise<bool> done;

        t->opEListAsync(
            in,
            [&](const list<E>& ret, const list<E>& out)
            {
                test(ret == out);
                test(ret == in);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        deque<optional<DPrx>> in(5);
        in[0] = DPrx(communicator, "C1:" + endp + " -t 10000");
        in[1] = DPrx(communicator, "C2:" + endp + " -t 10001");
        in[2] = DPrx(communicator, "C3:" + endp + " -t 10002");
        in[3] = DPrx(communicator, "C4:" + endp + " -t 10003");
        in[4] = DPrx(communicator, "C5:" + endp + " -t 10004");

        promise<bool> done;

        t->opDPrxSeqAsync(
            in,
            [&](deque<optional<DPrx>> ret, deque<optional<DPrx>> out)
            {
                test(ret.size() == in.size());
                auto op = out.begin();
                auto rp = ret.begin();
                for (const auto& i : in)
                {
                    test(*op++ == i);
                    test(*rp++ == i);
                }
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        list<optional<DPrx>> in;
        in.emplace_back(DPrx(communicator, "C1:" + endp + " -t 10000"));
        in.emplace_back(DPrx(communicator, "C2:" + endp + " -t 10001"));
        in.emplace_back(DPrx(communicator, "C3:" + endp + " -t 10002"));
        in.emplace_back(DPrx(communicator, "C4:" + endp + " -t 10003"));
        in.emplace_back(DPrx(communicator, "C5:" + endp + " -t 10004"));

        promise<bool> done;

        t->opDPrxListAsync(
            in,
            [&](list<optional<DPrx>> ret, list<optional<DPrx>> out)
            {
                test(ret.size() == in.size());
                auto op = out.begin();
                auto rp = ret.begin();
                for (const auto& i : in)
                {
                    test(*op++ == i);
                    test(*rp++ == i);
                }
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        deque<CPtr> in(5);
        in[0] = make_shared<C>();
        in[1] = in[0];
        in[2] = in[0];
        in[3] = in[0];
        in[4] = in[0];

        promise<bool> done;

        t->opCSeqAsync(
            in,
            [&](const deque<shared_ptr<C>>& ret, const deque<shared_ptr<C>>& out)
            {
                test(ret == out);
                test(ret.size() == in.size());
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        list<CPtr> in;
        in.push_back(make_shared<C>());
        in.push_back(make_shared<C>());
        in.push_back(make_shared<C>());
        in.push_back(make_shared<C>());
        in.push_back(make_shared<C>());

        promise<bool> done;

        t->opCListAsync(
            in,
            [&](const list<shared_ptr<C>>& ret, const list<shared_ptr<C>>& out)
            {
                test(ret == out);
                test(ret.size() == in.size());
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    {
        ByteSeq in;
        in.push_back(byte{'1'});
        in.push_back(byte{'2'});
        in.push_back(byte{'3'});
        in.push_back(byte{'4'});

        promise<bool> done;

        t->opOutArrayByteSeqAsync(
            in,
            [&](pair<const byte*, const byte*> out)
            {
                test(arrayRangeEquals<byte>(make_pair<const byte*>(&in[0], &in[0] + in.size()), out));
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    cout << "ok" << endl;

    cout << "testing alternate dictionaries with new AMI... " << flush;
    {
        {
            IntStringDict idict;

            idict[1] = "ONE";
            idict[2] = "TWO";
            idict[3] = "THREE";
            idict[-1] = "MINUS ONE";

            auto r = t->opIntStringDictAsync(idict).get();
            test(std::get<1>(r) == idict);
            test(std::get<0>(r) == idict);
        }

        {
            CustomMap<std::string, int32_t> idict;

            idict["ONE"] = 1;
            idict["TWO"] = 2;
            idict["THREE"] = 3;
            idict["MINUS ONE"] = -1;

            auto r = t->opVarDictAsync(idict).get();
            test(std::get<1>(r) == idict);
            test(std::get<0>(r).size() == 1000);
            for (const auto& i : std::get<0>(r))
            {
                test(i.second == i.first * i.first);
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing alternate dictionaries with new AMI callbacks... " << flush;
    {
        {
            IntStringDict idict;

            idict[1] = "ONE";
            idict[2] = "TWO";
            idict[3] = "THREE";
            idict[-1] = "MINUS ONE";

            promise<bool> done;

            t->opIntStringDictAsync(
                idict,
                [&](const map<int, string>& ret, const map<int, string>& out)
                {
                    test(ret == out);
                    test(ret == idict);
                    done.set_value(true);
                },
                [&](std::exception_ptr) { done.set_value(false); });

            test(done.get_future().get());
        }

        {
            CustomMap<std::string, int32_t> idict;

            idict["ONE"] = 1;
            idict["TWO"] = 2;
            idict["THREE"] = 3;
            idict["MINUS ONE"] = -1;

            promise<bool> done;

            t->opVarDictAsync(
                idict,
                [&](const CustomMap<int64_t, int64_t>& ret, const CustomMap<string, int>& out)
                {
                    test(out == idict);
                    for (const auto& i : ret)
                    {
                        test(i.second == i.first * i.first);
                    }

                    done.set_value(true);
                },
                [&](std::exception_ptr) { done.set_value(false); });

            test(done.get_future().get());
        }
    }
    cout << "ok" << endl;

    cout << "testing wstring... " << flush;

    Test1::WstringSeq wseq1;
    wseq1.emplace_back(L"WIDE STRING");

    Test2::WstringSeq wseq2;
    wseq2 = wseq1;

    Test1::WstringWStringDict wdict1;
    wdict1[L"KEY"] = L"VALUE";

    Test2::WstringWStringDict wdict2;
    wdict2 = wdict1;

    Test1::WstringClassPrx wsc1(
        communicator,
        communicator->getProperties()->getPropertyWithDefault("Custom.WstringProxy1", "wstring1:" + endp));

    Test2::WstringClassPrx wsc2(
        communicator,
        communicator->getProperties()->getPropertyWithDefault("Custom.WstringProxy2", "wstring2:" + endp));

    wstring wstr = L"A WIDE STRING";
    wstring out;
    wstring ret = wsc1->opString(wstr, out);
    test(out == wstr);
    test(ret == wstr);

    {
        auto r = wsc1->opStringAsync(wstr).get();
        test(std::get<1>(r) == wstr);
        test(std::get<0>(r) == wstr);
    }

    {
        promise<bool> done;

        wsc1->opStringAsync(
            wstr,
            [&](const wstring& retP, const wstring& outP)
            {
                test(outP == wstr);
                test(retP == wstr);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

    ret = wsc2->opString(wstr, out);
    test(out == wstr);
    test(ret == wstr);

    {
        auto r = wsc2->opStringAsync(wstr).get();
        test(std::get<1>(r) == wstr);
        test(std::get<0>(r) == wstr);
    }

    {
        promise<bool> done;

        wsc2->opStringAsync(
            wstr,
            [&](const wstring& retP, const wstring& outP)
            {
                test(outP == wstr);
                test(retP == wstr);
                done.set_value(true);
            },
            [&](std::exception_ptr) { done.set_value(false); });

        test(done.get_future().get());
    }

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
    catch (const Test1::WstringException& ex)
    {
        test(ex.reason == wstr);
    }

    {
        auto f = wsc1->throwExceptAsync(wstr);
        try
        {
            f.get();
            test(false);
        }
        catch (const Test1::WstringException& ex)
        {
            test(ex.reason == wstr);
        }
    }

    {
        promise<bool> done;

        wsc1->throwExceptAsync(
            wstr,
            [&]() { done.set_value(false); },
            [&](std::exception_ptr eptr)
            {
                try
                {
                    std::rethrow_exception(eptr);
                }
                catch (const Test1::WstringException& ex)
                {
                    test(ex.reason == wstr);
                    done.set_value(true);
                }
                catch (...)
                {
                    done.set_value(false);
                }
            });

        test(done.get_future().get());
    }

    try
    {
        wsc2->throwExcept(wstr);
    }
    catch (const Test2::WstringException& ex)
    {
        test(ex.reason == wstr);
    }

    {
        auto f = wsc2->throwExceptAsync(wstr);
        try
        {
            f.get();
            test(false);
        }
        catch (const Test2::WstringException& ex)
        {
            test(ex.reason == wstr);
        }
    }

    {
        promise<bool> done;

        wsc2->throwExceptAsync(
            wstr,
            [&]() { done.set_value(false); },
            [&](std::exception_ptr eptr)
            {
                try
                {
                    std::rethrow_exception(eptr);
                }
                catch (const Test2::WstringException& ex)
                {
                    test(ex.reason == wstr);
                    done.set_value(true);
                }
                catch (...)
                {
                    done.set_value(false);
                }
            });

        test(done.get_future().get());
    }

    cout << "ok" << endl;

    return t;
}
