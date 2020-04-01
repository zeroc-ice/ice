//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceUtil/Iterator.h>
#include <TestHelper.h>
#include <Test.h>
#include <Wstring.h>

#ifdef _MSC_VER
#   pragma warning( 4 : 4503 ) // C4503: ... : decorated name length exceeded, name was truncated
#endif

using namespace std;

namespace
{

template<typename T> bool
arrayRangeEquals(pair<const T*, const T*> lhs, pair<const T*, const T*> rhs)
{
    if(lhs.second - lhs.first != rhs.second - rhs.first)
    {
        return false;
    }

    T* l = const_cast<T*>(lhs.first);
    T* r = const_cast<T*>(rhs.first);
    while(l != lhs.second)
    {
        if(*l++ != *r++)
        {
            return false;
        }
    }
    return true;
}

}

Test::TestIntfPrxPtr
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    const string endp = helper->getTestEndpoint();
    cout << "testing stringToProxy... " << flush;
    string ref = communicator->getProperties()->getPropertyWithDefault("Custom.Proxy", "test:" + endp);
    Ice::ObjectPrxPtr base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;

    Test::TestIntfPrxPtr t = ICE_CHECKED_CAST(Test::TestIntfPrx, base);
    test(t);
    test(Ice::targetEqualTo(t, base));
    cout << "ok" << endl;

    cout << "testing ice_id and ice_ids with string converter... " << flush;
    test(t->ice_id() == Test::TestIntfPrx::ice_staticId());
    test(t->ice_ids()[0] == Ice::ObjectPrx::ice_staticId());
    test(t->ice_ids()[1] == Test::TestIntfPrx::ice_staticId());
    cout << "ok" << endl;

    cout << "testing alternate strings... " << flush;
    {
        Util::string_view in = "HELLO WORLD!";
        string out;
        string ret = t->opString(in, out);

        test(ret == out);
        test(ret == in);
    }
    cout << "ok" << endl;

    cout << "testing alternate sequences... " << flush;

    {
        Test::DoubleSeq in(5);
        in[0] = 3.14;
        in[1] = 1 / 3;
        in[2] = 0.375;
        in[3] = 4 / 3;
        in[4] = -5.725;
        Ice::Double inArray[5];
        for(size_t i = 0; i < 5; ++i)
        {
            inArray[i] = in[i];
        }
        pair<const Ice::Double*, const Ice::Double*> inPair(inArray, inArray + 5);

        Test::DoubleSeq out;
        Test::DoubleSeq ret = t->opDoubleArray(inPair, out);
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
        for(size_t i = 0; i < 5; ++i)
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
        pair<const Test::Variable*, const Test::Variable*> inPair(inArray, inArray + 5);

        Test::VariableList out;
        Test::VariableList ret = t->opVariableArray(inPair, out);

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
            *p = static_cast<Ice::Byte>('1' + i++);
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
        in.push_back("THESE");
        in.push_back("ARE");
        in.push_back("FIVE");
        in.push_back("SHORT");
        in.push_back("STRINGS.");

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
        in[0].s = "THESE";
        in[1].s = "ARE";
        in[2].s = "FIVE";
        in[3].s = "SHORT";
        in[4].s = "STRINGS.";

        deque<Test::Variable> out;
        deque<Test::Variable> ret = t->opVariableSeq(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        list<Test::Variable> in;
        Test::Variable v;
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

        list<Test::Variable> out;
        list<Test::Variable> ret = t->opVariableList(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        deque<Test::StringStringDict> in(5);
        in[0]["A"] = "A";
        in[1]["B"] = "B";
        in[2]["C"] = "C";
        in[3]["D"] = "D";
        in[4]["E"] = "E";

        deque<Test::StringStringDict> out;
        deque<Test::StringStringDict> ret = t->opStringStringDictSeq(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        list<Test::StringStringDict> in;
        Test::StringStringDict ssd;
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

        list<Test::StringStringDict> out;
        list<Test::StringStringDict> ret = t->opStringStringDictList(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        deque<Test::E> in(5);
        in[0] = Test:: E::E1;
        in[1] = Test:: E::E2;
        in[2] = Test:: E::E3;
        in[3] = Test:: E::E1;
        in[4] = Test:: E::E3;

        deque<Test::E> out;
        deque<Test::E> ret = t->opESeq(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        list<Test::E> in;
        in.push_back(Test:: E::E1);
        in.push_back(Test:: E::E2);
        in.push_back(Test:: E::E3);
        in.push_back(Test:: E::E1);
        in.push_back(Test:: E::E3);

        list<Test::E> out;
        list<Test::E> ret = t->opEList(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        deque<Test::DPrxPtr> in(5);
        in[0] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C1:" + endp + " -t 10000"));
        in[1] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C2:" + endp + " -t 10001"));
        in[2] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C3:" + endp + " -t 10002"));
        in[3] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C4:" + endp + " -t 10003"));
        in[4] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C5:" + endp + " -t 10004"));

        deque<Test::DPrxPtr> out;
        deque<Test::DPrxPtr> ret = t->opDPrxSeq(in, out);

        auto op = out.begin();
        auto rp = ret.begin();

        for(auto i: in)
        {
            test(Ice::targetEqualTo(*op++, i));
            test(Ice::targetEqualTo(*rp++, i));
        }
    }

    {
        list<Test::DPrxPtr> in;
        in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C1:" + endp + " -t 10000")));
        in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C2:" + endp + " -t 10001")));
        in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C3:" + endp + " -t 10002")));
        in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C4:" + endp + " -t 10003")));
        in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C5:" + endp + " -t 10004")));

        list<Test::DPrxPtr> out;
        list<Test::DPrxPtr> ret = t->opDPrxList(in, out);
        auto op = out.begin();
        auto rp = ret.begin();

        for(auto i: in)
        {
            test(Ice::targetEqualTo(*op++, i));
            test(Ice::targetEqualTo(*rp++, i));
        }
    }

    {
        deque<Test::CPtr> in(5);
        in[0] = std::make_shared<Test::C>();
        in[1] = in[0];
        in[2] = in[0];
        in[3] = in[0];
        in[4] = in[0];

        deque<Test::CPtr> out;
        deque<Test::CPtr> ret = t->opCSeq(in, out);
        test(out.size() == in.size());
        test(ret.size() == in.size());
        for(unsigned int i = 1; i < in.size(); ++i)
        {
            test(out[i] == out[0]);
            test(ret[i] == out[i]);
        }
    }

    {
        list<Test::CPtr> in;
        in.push_back(std::make_shared<Test::C>());
        in.push_back(std::make_shared<Test::C>());
        in.push_back(std::make_shared<Test::C>());
        in.push_back(std::make_shared<Test::C>());
        in.push_back(std::make_shared<Test::C>());

        list<Test::CPtr> out;
        list<Test::CPtr> ret = t->opCList(in, out);
        test(out.size() == in.size());
        test(ret.size() == in.size());
        list<Test::CPtr>::const_iterator p1;
        list<Test::CPtr>::const_iterator p2;
        for(p1 = out.begin(), p2 = ret.begin(); p1 != out.end(); ++p1, ++p2)
        {
            test(*p1 == *p2);
        }
    }

    cout << "ok" << endl;

    cout << "testing alternate dictionaries... " << flush;

    {
        Test::IntStringDict idict;

        idict[1] = "ONE";
        idict[2] = "TWO";
        idict[3] = "THREE";
        idict[-1] = "MINUS ONE";

        Test::IntStringDict out;
        out[5] = "FIVE";

        Test::IntStringDict ret = t->opIntStringDict(idict, out);

        test(out == idict);
        test(ret == idict);
    }

    {
        Test::CustomMap<std::string, Ice::Int> idict;

        idict["ONE"] = 1;
        idict["TWO"] = 2;
        idict["THREE"] = 3;
        idict["MINUS ONE"] = -1;

        Test::CustomMap<std::string, Ice::Int> out;
        out["FIVE"] = 5;

        Test::CustomMap<Ice::Long, Ice::Long> ret = t->opVarDict(idict, out);

        test(out == idict);

        test(ret.size() == 1000);
        for(Test::CustomMap<Ice::Long, Ice::Long>::const_iterator i = ret.begin(); i != ret.end(); ++i)
        {
            test(i->second == i->first * i->first);
        }
    }

    {
        std::map<int, Util::string_view> idict;

        idict[1] = "ONE";
        idict[2] = "TWO";
        idict[3] = "THREE";
        idict[-1] = "MINUS ONE";

        Test::IntStringDict out;
        out[5] = "FIVE";

        Test::IntStringDict ret = t->opCustomIntStringDict(idict, out);
        test(out.size() == idict.size());
        test(out == ret);
        for(std::map<int, Util::string_view>::const_iterator p = idict.begin();
            p != idict.end(); ++p)
        {
            test(out[p->first] == p->second.to_string());
        }
    }

    cout << "ok" << endl;

    cout << "testing alternate custom sequences... " << flush;
    {
        Test::ShortBuffer inS;
        inS.setAndInit(new Ice::Short[3], 3);
        Test::ShortBuffer outS;
        Test::ShortBuffer ret = t->opShortBuffer(inS, outS);

        test(outS == inS);
        test(ret == inS);

        Test::CustomBuffer<bool> inBS;
        inBS.setAndInit(new bool[2], 2);

        Test::CustomBuffer<bool> outBS;
        Test::CustomBuffer<bool> retBS = t->opBoolBuffer(inBS, outBS);

        test(outBS == inBS);
        test(retBS == inBS);

        Test::BufferStruct bs;
        bs.byteBuf.setAndInit(new Ice::Byte[10], 10);
        bs.boolBuf.setAndInit(new bool[10], 10);
        bs.shortBuf.setAndInit(new Ice::Short[10], 10);
        bs.intBuf.setAndInit(new Ice::Int[10], 10);
        bs.longBuf.setAndInit(new Ice::Long[10], 10);
        bs.floatBuf.setAndInit(new Ice::Float[10], 10);
        bs.doubleBuf.setAndInit(new Ice::Double[10], 10);

        Test::BufferStruct rs = t->opBufferStruct(bs);
        test(rs == bs);

        Ice::OutputStream os(communicator);
        os.write(rs);
        Ice::ByteSeq bytes;
        os.finished(bytes);

        Ice::InputStream is(communicator, os.getEncoding(), bytes);
        Test::BufferStruct rs2;
        is.read(rs2);
        test(rs == rs2);
    }
    cout << "ok" << endl;

    cout << "testing alternate strings with AMI... " << flush;
    {
        Util::string_view in = "HELLO WORLD!";

        auto r = t->opStringAsync(in).get();
        test(r.returnValue == r.outString);
        test(r.returnValue == in);

    }
    cout << "ok" << endl;

    cout << "testing alternate sequences with AMI... " << flush;
    {
        {
            Test::DoubleSeq in(5);
            in[0] = 3.14;
            in[1] = 1 / 3;
            in[2] = 0.375;
            in[3] = 4 / 3;
            in[4] = -5.725;
            Ice::Double inArray[5];
            for(size_t i = 0; i < 5; ++i)
            {
                inArray[i] = in[i];
            }
            pair<const Ice::Double*, const Ice::Double*> inPair(inArray, inArray + 5);

            auto r = t->opDoubleArrayAsync(inPair).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            Test::BoolSeq in(5);
            in[0] = false;
            in[1] = true;
            in[2] = true;
            in[3] = false;
            in[4] = true;
            bool inArray[5];
            for(size_t i = 0; i < 5; ++i)
            {
                inArray[i] = in[i];
            }
            pair<const bool*, const bool*> inPair(inArray, inArray + 5);

            auto r = t->opBoolArrayAsync(inPair).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
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

            auto r = t->opByteArrayAsync(inPair).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            Test::VariableList in;
            Test::Variable inArray[5];
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
            pair<const Test::Variable*, const Test::Variable*> inPair(inArray, inArray + 5);

            auto r = t->opVariableArrayAsync(inPair).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            deque<bool> in(5);
            in[0] = false;
            in[1] = true;
            in[2] = true;
            in[3] = false;
            in[4] = true;

            auto r = t->opBoolSeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            list<bool> in;
            in.push_back(false);
            in.push_back(true);
            in.push_back(true);
            in.push_back(false);
            in.push_back(true);

            auto r = t->opBoolListAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            deque< ::Ice::Byte> in(5);
            in[0] = '1';
            in[1] = '2';
            in[2] = '3';
            in[3] = '4';
            in[4] = '5';

            auto r = t->opByteSeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            list< ::Ice::Byte> in;
            in.push_back('1');
            in.push_back('2');
            in.push_back('3');
            in.push_back('4');
            in.push_back('5');

            auto r = t->opByteListAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            MyByteSeq in(5);
            int i = 0;
            for(MyByteSeq::iterator p = in.begin(); p != in.end(); ++p)
            {
                *p = static_cast<Ice::Byte>('1' + i++);
            }

            auto r = t->opMyByteSeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            deque<string> in(5);
            in[0] = "THESE";
            in[1] = "ARE";
            in[2] = "FIVE";
            in[3] = "SHORT";
            in[4] = "STRINGS.";

            auto r = t->opStringSeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            list<string> in;
            in.push_back("THESE");
            in.push_back("ARE");
            in.push_back("FIVE");
            in.push_back("SHORT");
            in.push_back("STRINGS.");

            auto r = t->opStringListAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            deque<Test::Fixed> in(5);
            in[0].s = 1;
            in[1].s = 2;
            in[2].s = 3;
            in[3].s = 4;
            in[4].s = 5;

            auto r = t->opFixedSeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            list<Test::Fixed> in(5);
            short num = 1;
            for(list<Test::Fixed>::iterator p = in.begin(); p != in.end(); ++p)
            {
                (*p).s = num++;
            }

            auto r = t->opFixedListAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            deque<Test::Variable> in(5);
            in[0].s = "THESE";
            in[1].s = "ARE";
            in[2].s = "FIVE";
            in[3].s = "SHORT";
            in[4].s = "STRINGS.";

            auto r = t->opVariableSeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            list<Test::Variable> in;
            Test::Variable v;
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
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            deque<Test::StringStringDict> in(5);
            in[0]["A"] = "A";
            in[1]["B"] = "B";
            in[2]["C"] = "C";
            in[3]["D"] = "D";
            in[4]["E"] = "E";

            auto r = t->opStringStringDictSeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            list<Test::StringStringDict> in;
            Test::StringStringDict ssd;
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
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            deque<Test::E> in(5);
            in[0] = Test:: E::E1;
            in[1] = Test:: E::E2;
            in[2] = Test:: E::E3;
            in[3] = Test:: E::E1;
            in[4] = Test:: E::E3;

            auto r = t->opESeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            list<Test::E> in;
            in.push_back(Test:: E::E1);
            in.push_back(Test:: E::E2);
            in.push_back(Test:: E::E3);
            in.push_back(Test:: E::E1);
            in.push_back(Test:: E::E3);

            auto r = t->opEListAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
        }

        {
            deque<Test::DPrxPtr> in(5);
            in[0] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C1:" + endp + " -t 10000"));
            in[1] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C2:" + endp + " -t 10001"));
            in[2] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C3:" + endp + " -t 10002"));
            in[3] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C4:" + endp + " -t 10003"));
            in[4] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C5:" + endp + " -t 10004"));

            auto r = t->opDPrxSeqAsync(in).get();

            test(r.outSeq.size() == in.size());
            test(r.returnValue.size() == in.size());

            auto op = r.outSeq.begin();
            auto rp = r.returnValue.begin();

            for(auto i: in)
            {
                test(Ice::targetEqualTo(*op++, i));
                test(Ice::targetEqualTo(*rp++, i));
            }
        }

        {
            list<Test::DPrxPtr> in;
            in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C1:" + endp + " -t 10000")));
            in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C2:" + endp + " -t 10001")));
            in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C3:" + endp + " -t 10002")));
            in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C4:" + endp + " -t 10003")));
            in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C5:" + endp + " -t 10004")));

            auto r = t->opDPrxListAsync(in).get();

            test(r.outSeq.size() == in.size());
            test(r.returnValue.size() == in.size());

            auto op = r.outSeq.begin();
            auto rp = r.returnValue.begin();

            for(auto i: in)
            {
                test(Ice::targetEqualTo(*op++, i));
                test(Ice::targetEqualTo(*rp++, i));
            }
        }

        {
            deque<Test::CPtr> in(5);
            in[0] = std::make_shared<Test::C>();
            in[1] = in[0];
            in[2] = in[0];
            in[3] = in[0];
            in[4] = in[0];

            auto r = t->opCSeqAsync(in).get();
            test(r.outSeq.size() == in.size());
            test(r.returnValue.size() == in.size());

            auto rp = r.returnValue.begin();
            for(auto o: r.outSeq)
            {
                test(o == r.outSeq[0]);
                test(*rp++ == o);
            }
        }

        {
            list<Test::CPtr> in;
            in.push_back(std::make_shared<Test::C>());
            in.push_back(std::make_shared<Test::C>());
            in.push_back(std::make_shared<Test::C>());
            in.push_back(std::make_shared<Test::C>());
            in.push_back(std::make_shared<Test::C>());

            auto r = t->opCListAsync(in).get();
            test(r.outSeq.size() == in.size());
            test(r.returnValue.size() == in.size());
            test(r.outSeq == r.returnValue);
        }

        {
            Test::ByteSeq in;
            in.push_back('1');
            in.push_back('2');
            in.push_back('3');
            in.push_back('4');

            auto r = t->opOutArrayByteSeqAsync(in).get();
            test(r.size() == in.size());
            test(r == in);
        }
    }
    cout << "ok" << endl;

    cout << "testing alternate strings with AMI callbacks... " << flush;
    {
        Util::string_view in = "HELLO WORLD!";

        promise<bool> done;

        t->opStringAsync(in,
            [&](Util::string_view ret, Util::string_view out)
            {
                test(out == ret);
                test(in == out);
                done.set_value(true);
            },
            [&](std::exception_ptr)
            {
                done.set_value(false);
            });

        test(done.get_future().get());
    }
    cout << "ok" << endl;

    cout << "testing alternate sequences with AMI callbacks... " << flush;

    {
        Test::DoubleSeq in(5);
        in[0] = 3.14;
        in[1] = 1 / 3;
        in[2] = 0.375;
        in[3] = 4 / 3;
        in[4] = -5.725;
        Ice::Double inArray[5];
        for(size_t i = 0; i < 5; ++i)
        {
            inArray[i] = in[i];
        }
        pair<const Ice::Double*, const Ice::Double*> inPair(inArray, inArray + 5);

        promise<bool> done;

        t->opDoubleArrayAsync(inPair,
                              [&](pair<const Ice::Double*, const Ice::Double*> ret,
                                  pair<const Ice::Double*, const Ice::Double*> out)
                              {
                                  test(arrayRangeEquals<double>(out, inPair));
                                  test(arrayRangeEquals<double>(ret, inPair));
                                  done.set_value(true);
                              },
                              [&](std::exception_ptr)
                              {
                                  done.set_value(false);
                              });

        test(done.get_future().get());
    }

    {
        Test::BoolSeq in(5);
        in[0] = false;
        in[1] = true;
        in[2] = true;
        in[3] = false;
        in[4] = true;
        bool inArray[5];
        for(size_t i = 0; i < 5; ++i)
        {
            inArray[i] = in[i];
        }
        pair<const bool*, const bool*> inPair(inArray, inArray + 5);

        promise<bool> done;

        t->opBoolArrayAsync(inPair,
                            [&](pair<const bool*, const bool*> ret,
                                pair<const bool*, const bool*> out)
                            {
                                test(arrayRangeEquals<bool>(out, inPair));
                                test(arrayRangeEquals<bool>(ret, inPair));
                                done.set_value(true);
                            },
                            [&](std::exception_ptr)
                            {
                                done.set_value(false);
                            });

        test(done.get_future().get());
    }

    {
        Ice::Byte in[5];
        in[0] = '1';
        in[1] = '2';
        in[2] = '3';
        in[3] = '4';
        in[4] = '5';
        pair<const Ice::Byte*, const Ice::Byte*> inPair(in, in + 5);

        promise<bool> done;

        t->opByteArrayAsync(inPair,
                            [&](pair<const Ice::Byte*, const Ice::Byte*> ret,
                                pair<const Ice::Byte*, const Ice::Byte*> out)
                            {
                                test(arrayRangeEquals<Ice::Byte>(out, inPair));
                                test(arrayRangeEquals<Ice::Byte>(ret, inPair));
                                done.set_value(true);
                            },
                            [&](std::exception_ptr)
                            {
                                done.set_value(false);
                            });

        test(done.get_future().get());
    }

    {
        Test::VariableList in;
        Test::Variable inArray[5];
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
        pair<const Test::Variable*, const Test::Variable*> inPair(inArray, inArray + 5);

        promise<bool> done;

        t->opVariableArrayAsync(inPair,
                                [&](pair<const Test::Variable*, const Test::Variable*> ret,
                                    pair<const Test::Variable*, const Test::Variable*> out)
                                {
                                    test(arrayRangeEquals<Test::Variable>(out, inPair));
                                    test(arrayRangeEquals<Test::Variable>(ret, inPair));
                                    done.set_value(true);
                                },
                                [&](std::exception_ptr)
                                {
                                    done.set_value(false);
                                });

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

        t->opBoolSeqAsync(in,
                          [&](deque<bool>ret, deque<bool> out)
                          {
                              test(ret == out);
                              test(ret == in);
                              done.set_value(true);
                          },
                          [&](std::exception_ptr)
                          {
                              done.set_value(false);
                          });

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

        t->opBoolListAsync(in,
                          [&](list<bool>ret, list<bool> out)
                          {
                              test(ret == out);
                              test(ret == in);
                              done.set_value(true);
                          },
                          [&](std::exception_ptr)
                          {
                              done.set_value(false);
                          });

        test(done.get_future().get());
    }

    {
        deque< ::Ice::Byte> in(5);
        in[0] = '1';
        in[1] = '2';
        in[2] = '3';
        in[3] = '4';
        in[4] = '5';

        promise<bool> done;

        t->opByteSeqAsync(in,
                          [&](deque<Ice::Byte> ret, deque<Ice::Byte> out)
                          {
                              test(ret == out);
                              test(ret == in);
                              done.set_value(true);
                          },
                          [&](std::exception_ptr)
                          {
                              done.set_value(false);
                          });

        test(done.get_future().get());
    }

    {
        list< ::Ice::Byte> in;
        in.push_back('1');
        in.push_back('2');
        in.push_back('3');
        in.push_back('4');
        in.push_back('5');

        promise<bool> done;

        t->opByteListAsync(in,
                           [&](list<Ice::Byte> ret, list<Ice::Byte> out)
                           {
                               test(ret == out);
                               test(ret == in);
                               done.set_value(true);
                           },
                           [&](std::exception_ptr)
                           {
                               done.set_value(false);
                           });

        test(done.get_future().get());
    }

    {
        MyByteSeq in(5);
        int i = 0;
        for(MyByteSeq::iterator p = in.begin(); p != in.end(); ++p)
        {
            *p = static_cast<Ice::Byte>('1' + i++);
        }

        promise<bool> done;

        t->opMyByteSeqAsync(in,
                            [&](MyByteSeq ret, MyByteSeq out)
                            {
                                test(ret == out);
                                test(ret == in);
                                done.set_value(true);
                            },
                            [&](std::exception_ptr)
                            {
                                done.set_value(false);
                            });

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

        t->opStringSeqAsync(in,
                            [&](deque<string> ret, deque<string> out)
                            {
                                test(ret == out);
                                test(ret == in);
                                done.set_value(true);
                            },
                            [&](std::exception_ptr)
                            {
                                done.set_value(false);
                            });

        test(done.get_future().get());
    }

    {
        list<string> in;
        in.push_back("THESE");
        in.push_back("ARE");
        in.push_back("FIVE");
        in.push_back("SHORT");
        in.push_back("STRINGS.");

        promise<bool> done;

        t->opStringListAsync(in,
                             [&](list<string> ret, list<string> out)
                             {
                                 test(ret == out);
                                 test(ret == in);
                                 done.set_value(true);
                             },
                             [&](std::exception_ptr)
                             {
                                 done.set_value(false);
                             });

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

        t->opFixedSeqAsync(in,
                           [&](deque<Test::Fixed> ret, deque<Test::Fixed> out)
                           {
                               test(ret == out);
                               test(ret == in);
                               done.set_value(true);
                           },
                           [&](std::exception_ptr)
                           {
                               done.set_value(false);
                           });

        test(done.get_future().get());
    }

    {
        list<Test::Fixed> in(5);
        short num = 1;
        for(list<Test::Fixed>::iterator p = in.begin(); p != in.end(); ++p)
        {
            (*p).s = num++;
        }

        promise<bool> done;

        t->opFixedListAsync(in,
                            [&](list<Test::Fixed> ret, list<Test::Fixed> out)
                            {
                                test(ret == out);
                                test(ret == in);
                                done.set_value(true);
                            },
                            [&](std::exception_ptr)
                            {
                                done.set_value(false);
                            });

        test(done.get_future().get());
    }

    {
        deque<Test::Variable> in(5);
        in[0].s = "THESE";
        in[1].s = "ARE";
        in[2].s = "FIVE";
        in[3].s = "SHORT";
        in[4].s = "STRINGS.";

        promise<bool> done;

        t->opVariableSeqAsync(in,
                              [&](deque<Test::Variable> ret, deque<Test::Variable> out)
                              {
                                  test(ret == out);
                                  test(ret == in);
                                  done.set_value(true);
                              },
                              [&](std::exception_ptr)
                              {
                                  done.set_value(false);
                              });

        test(done.get_future().get());
    }

    {
        list<Test::Variable> in;
        Test::Variable v;
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

        t->opVariableListAsync(in,
                               [&](list<Test::Variable> ret, list<Test::Variable> out)
                               {
                                   test(ret == out);
                                   test(ret == in);
                                   done.set_value(true);
                               },
                               [&](std::exception_ptr)
                               {
                                   done.set_value(false);
                               });

        test(done.get_future().get());
    }

    {
        deque<Test::StringStringDict> in(5);
        in[0]["A"] = "A";
        in[1]["B"] = "B";
        in[2]["C"] = "C";
        in[3]["D"] = "D";
        in[4]["E"] = "E";

        promise<bool> done;

        t->opStringStringDictSeqAsync(in,
                                      [&](deque<Test::StringStringDict> ret, deque<Test::StringStringDict> out)
                                      {
                                          test(ret == out);
                                          test(ret == in);
                                          done.set_value(true);
                                       },
                                      [&](std::exception_ptr)
                                      {
                                          done.set_value(false);
                                      });

        test(done.get_future().get());
    }

    {
        list<Test::StringStringDict> in;
        Test::StringStringDict ssd;
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

        t->opStringStringDictListAsync(in,
                                   [&](list<Test::StringStringDict> ret, list<Test::StringStringDict> out)
                                   {
                                       test(ret == out);
                                       test(ret == in);
                                       done.set_value(true);
                                   },
                                   [&](std::exception_ptr)
                                   {
                                       done.set_value(false);
                                   });

        test(done.get_future().get());
    }

    {
        deque<Test::E> in(5);
        in[0] = Test:: E::E1;
        in[1] = Test:: E::E2;
        in[2] = Test:: E::E3;
        in[3] = Test:: E::E1;
        in[4] = Test:: E::E3;

        promise<bool> done;

        t->opESeqAsync(in,
                       [&](deque<Test::E> ret, deque<Test::E> out)
                       {
                           test(ret == out);
                           test(ret == in);
                           done.set_value(true);
                       },
                       [&](std::exception_ptr)
                       {
                           done.set_value(false);
                       });

        test(done.get_future().get());
    }

    {
        list<Test::E> in;
        in.push_back(Test:: E::E1);
        in.push_back(Test:: E::E2);
        in.push_back(Test:: E::E3);
        in.push_back(Test:: E::E1);
        in.push_back(Test:: E::E3);

        promise<bool> done;

        t->opEListAsync(in,
                        [&](list<Test::E> ret, list<Test::E> out)
                        {
                            test(ret == out);
                            test(ret == in);
                            done.set_value(true);
                        },
                        [&](std::exception_ptr)
                        {
                            done.set_value(false);
                        });

        test(done.get_future().get());
    }

    {
        deque<Test::DPrxPtr> in(5);
        in[0] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C1:" + endp + " -t 10000"));
        in[1] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C2:" + endp + " -t 10001"));
        in[2] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C3:" + endp + " -t 10002"));
        in[3] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C4:" + endp + " -t 10003"));
        in[4] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C5:" + endp + " -t 10004"));

        promise<bool> done;

        t->opDPrxSeqAsync(in,
                          [&](deque<shared_ptr<Test::DPrx>> ret, deque<shared_ptr<Test::DPrx>> out)
                          {
                              test(ret.size() == in.size());
                              auto op = out.begin();
                              auto rp = ret.begin();
                              for(auto i: in)
                              {
                                  test(Ice::targetEqualTo(*op++, i));
                                  test(Ice::targetEqualTo(*rp++, i));
                              }
                              done.set_value(true);
                          },
                          [&](std::exception_ptr)
                          {
                              done.set_value(false);
                          });

        test(done.get_future().get());
    }

    {
        list<Test::DPrxPtr> in;
        in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C1:" + endp + " -t 10000")));
        in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C2:" + endp + " -t 10001")));
        in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C3:" + endp + " -t 10002")));
        in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C4:" + endp + " -t 10003")));
        in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C5:" + endp + " -t 10004")));

        promise<bool> done;

        t->opDPrxListAsync(in,
                           [&](list<shared_ptr<Test::DPrx>> ret, list<shared_ptr<Test::DPrx>> out)
                           {
                               test(ret.size() == in.size());
                              auto op = out.begin();
                              auto rp = ret.begin();
                              for(auto i: in)
                              {
                                  test(Ice::targetEqualTo(*op++, i));
                                  test(Ice::targetEqualTo(*rp++, i));
                              }
                              done.set_value(true);
                          },
                          [&](std::exception_ptr)
                          {
                              done.set_value(false);
                          });

        test(done.get_future().get());
    }

    {
        deque<Test::CPtr> in(5);
        in[0] = std::make_shared<Test::C>();
        in[1] = in[0];
        in[2] = in[0];
        in[3] = in[0];
        in[4] = in[0];

        promise<bool> done;

        t->opCSeqAsync(in,
                       [&](deque<shared_ptr<Test::C>> ret, deque<shared_ptr<Test::C>> out)
                       {
                           test(ret == out);
                           test(ret.size() == in.size());
                           done.set_value(true);
                       },
                       [&](std::exception_ptr)
                       {
                           done.set_value(false);
                       });

        test(done.get_future().get());
    }

    {
        list<Test::CPtr> in;
        in.push_back(std::make_shared<Test::C>());
        in.push_back(std::make_shared<Test::C>());
        in.push_back(std::make_shared<Test::C>());
        in.push_back(std::make_shared<Test::C>());
        in.push_back(std::make_shared<Test::C>());

        promise<bool> done;

        t->opCListAsync(in,
                        [&](list<shared_ptr<Test::C>> ret, list<shared_ptr<Test::C>> out)
                        {
                            test(ret == out);
                            test(ret.size() == in.size());
                            done.set_value(true);
                        },
                        [&](std::exception_ptr)
                        {
                            done.set_value(false);
                        });

        test(done.get_future().get());
    }

    {
        Test::ByteSeq in;
        in.push_back('1');
        in.push_back('2');
        in.push_back('3');
        in.push_back('4');

        promise<bool> done;

        t->opOutArrayByteSeqAsync(in,
                                  [&](pair<const Ice::Byte*, const Ice::Byte*> out)
                                  {
                                      test(arrayRangeEquals<Ice::Byte>(
                                               make_pair<const Ice::Byte*>(&in[0], &in[0] + in.size()), out));
                                      done.set_value(true);
                                  },
                                  [&](std::exception_ptr)
                                  {
                                      done.set_value(false);
                                  });

        test(done.get_future().get());
    }

    cout << "ok" << endl;

    cout << "testing alternate dictionaries with new AMI... " << flush;
    {
        {
            Test::IntStringDict idict;

            idict[1] = "ONE";
            idict[2] = "TWO";
            idict[3] = "THREE";
            idict[-1] = "MINUS ONE";

            auto r = t->opIntStringDictAsync(idict).get();
            test(r.odict == idict);
            test(r.returnValue == idict);
        }

        {
            Test::CustomMap<std::string, Ice::Int> idict;

            idict["ONE"] = 1;
            idict["TWO"] = 2;
            idict["THREE"] = 3;
            idict["MINUS ONE"] = -1;

            auto r = t->opVarDictAsync(idict).get();
            test(r.odict == idict);
            test(r.returnValue.size() == 1000);
            for(auto i: r.returnValue)
            {
                test(i.second == i.first * i.first);
            }
        }

        {
            std::map<int, Util::string_view> idict;

            idict[1] = "ONE";
            idict[2] = "TWO";
            idict[3] = "THREE";
            idict[-1] = "MINUS ONE";

            auto r = t->opCustomIntStringDictAsync(idict).get();
            test(r.odict.size() == idict.size());

            test(r.odict == r.returnValue);

            for(auto i: idict)
            {
                test(r.odict[i.first] == i.second);
            }
        }

    }
    cout << "ok" << endl;

    cout << "testing alternate dictionaries with new AMI callbacks... " << flush;
    {
        {
            Test::IntStringDict idict;

            idict[1] = "ONE";
            idict[2] = "TWO";
            idict[3] = "THREE";
            idict[-1] = "MINUS ONE";

            promise<bool> done;

            t->opIntStringDictAsync(idict,
                                    [&](map<int, string> ret, map<int, string> out)
                                    {
                                        test(ret == out);
                                        test(ret == idict);
                                        done.set_value(true);
                                    },
                                    [&](std::exception_ptr)
                                    {
                                        done.set_value(false);
                                    });

            test(done.get_future().get());

        }

        {
            Test::CustomMap<std::string, Ice::Int> idict;

            idict["ONE"] = 1;
            idict["TWO"] = 2;
            idict["THREE"] = 3;
            idict["MINUS ONE"] = -1;

            promise<bool> done;

            t->opVarDictAsync(idict,
                              [&](Test::CustomMap<long long, long long> ret, Test::CustomMap<string, int> out)
                              {
                                  test(out == idict);
                                  for(auto i: ret)
                                  {
                                      test(i.second == i.first * i.first);
                                  }

                                  done.set_value(true);
                              },
                              [&](std::exception_ptr)
                              {
                                  done.set_value(false);
                              });

            test(done.get_future().get());

        }

        {
            std::map<int, Util::string_view> idict;

            idict[1] = "ONE";
            idict[2] = "TWO";
            idict[3] = "THREE";
            idict[-1] = "MINUS ONE";

            promise<bool> done;

            t->opCustomIntStringDictAsync(idict,
                                          [&](map<int, Util::string_view> ret, map<int, Util::string_view> out)
                                          {
                                              test(ret == out);
                                              for(auto i: idict)
                                              {
                                                  test(ret[i.first] == i.second);
                                              }

                                              done.set_value(true);
                                           },
                                           [&](std::exception_ptr)
                                           {
                                              done.set_value(false);
                                           });

            test(done.get_future().get());
        }

    }
    cout << "ok" << endl;

    cout << "testing wstring... " << flush;

    Test1::WstringSeq wseq1;
    wseq1.push_back(L"WIDE STRING");

    Test2::WstringSeq wseq2;
    wseq2 = wseq1;

    Test1::WstringWStringDict wdict1;
    wdict1[L"KEY"] = L"VALUE";

    Test2::WstringWStringDict wdict2;
    wdict2 = wdict1;

    ref = communicator->getProperties()->getPropertyWithDefault("Custom.WstringProxy1", "wstring1:" + endp);
    base = communicator->stringToProxy(ref);
    test(base);
    Test1::WstringClassPrxPtr wsc1 = ICE_CHECKED_CAST(Test1::WstringClassPrx, base);
    test(t);

    ref = communicator->getProperties()->getPropertyWithDefault("Custom.WstringProxy2", "wstring2:" + endp);
    base = communicator->stringToProxy(ref);
    test(base);
    Test2::WstringClassPrxPtr wsc2 = ICE_CHECKED_CAST(Test2::WstringClassPrx, base);
    test(t);

    wstring wstr = L"A WIDE STRING";
    wstring out;
    wstring ret = wsc1->opString(wstr, out);
    test(out == wstr);
    test(ret == wstr);

    {
        auto r = wsc1->opStringAsync(wstr).get();
        test(r.s2 == wstr);
        test(r.returnValue == wstr);
    }
    {

        promise<bool> done;

        wsc1->opStringAsync(wstr,
                            [&](wstring retP, wstring outP)
                            {
                                test(outP == wstr);
                                test(retP == wstr);
                                done.set_value(true);
                            },
                            [&](std::exception_ptr)
                            {
                                done.set_value(false);
                            });

        test(done.get_future().get());
    }

    ret = wsc2->opString(wstr, out);
    test(out == wstr);
    test(ret == wstr);

    {
        auto r = wsc2->opStringAsync(wstr).get();
        test(r.s2 == wstr);
        test(r.returnValue == wstr);
    }
    {

        promise<bool> done;

        wsc2->opStringAsync(wstr,
                            [&](wstring retP, wstring outP)
                            {
                                test(outP == wstr);
                                test(retP == wstr);
                                done.set_value(true);
                            },
                            [&](std::exception_ptr)
                            {
                                done.set_value(false);
                            });

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
    catch(const Test1::WstringException& ex)
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
        catch(const Test1::WstringException& ex)
        {
            test(ex.reason == wstr);
        }
    }
    {

        promise<bool> done;

        wsc1->throwExceptAsync(wstr,
                               [&]()
                               {
                                   done.set_value(false);
                               },
                               [&](std::exception_ptr eptr)
                               {
                                   try
                                   {
                                       std::rethrow_exception(eptr);
                                   }
                                   catch(const Test1::WstringException& ex)
                                   {
                                       test(ex.reason == wstr);
                                       done.set_value(true);
                                   }
                                   catch(...)
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
    catch(const Test2::WstringException& ex)
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
        catch(const Test2::WstringException& ex)
        {
            test(ex.reason == wstr);
        }
    }
    {

        promise<bool> done;

        wsc2->throwExceptAsync(wstr,
                               [&]()
                               {
                                   done.set_value(false);
                               },
                               [&](std::exception_ptr eptr)
                               {
                                   try
                                   {
                                       std::rethrow_exception(eptr);
                                   }
                                   catch(const Test2::WstringException& ex)
                                   {
                                       test(ex.reason == wstr);
                                       done.set_value(true);
                                   }
                                   catch(...)
                                   {
                                       done.set_value(false);
                                   }
                               });

        test(done.get_future().get());
    }

    cout << "ok" << endl;

    return t;
}
