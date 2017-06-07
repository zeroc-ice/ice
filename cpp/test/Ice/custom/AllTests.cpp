// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/Iterator.h>
#include <TestCommon.h>
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

#ifndef ICE_CPP11_MAPPING

class CallbackBase : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    CallbackBase() :
        _called(false)
    {
    }

    virtual ~CallbackBase()
    {
    }

    void check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        while(!_called)
        {
            wait();
        }
        _called = false;
    }

protected:

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(!_called);
        _called = true;
        notify();
    }

private:

    bool _called;
};

class InParam : public Ice::LocalObject
{
};
typedef IceUtil::Handle<InParam> InParamPtr;

template<class T>
class InParamT : public InParam
{
public:

    InParamT(const T& v) : in(v)
    {
    }

    T in;
};

template<typename T> InParamPtr newInParam(const T& v)
{
    return new InParamT<T>(v);
}

template<typename T> const T& getIn(const InParamPtr& cookie)
{
    return dynamic_cast<InParamT<T>* >(cookie.get())->in;
}

class Callback : public CallbackBase, public IceUtil::Shared
{
public:

    void opString(const Util::string_view& ret,
                  const Util::string_view& out,
                  const InParamPtr& cookie)
    {
        const Util::string_view& in = getIn<Util::string_view>(cookie);
        test(out == ret);
        test(in.size() == out.size());
        called();
    }

    void opDoubleArray(const ::std::pair<const double*, const double*>& ret,
                       const ::std::pair<const double*, const double*>& out,
                       const InParamPtr& cookie)
    {
        const ::std::pair<const double*, const double*>& in = getIn<std::pair<const double*, const double*> >(cookie);
        test(arrayRangeEquals<double>(out, in));
        test(arrayRangeEquals<double>(ret, in));
        called();
    }

    void opBoolArray(const ::std::pair<const bool*, const bool*>& ret,
                     const ::std::pair<const bool*, const bool*>& out,
                     const InParamPtr& cookie)
    {
        const ::std::pair<const bool*, const bool*>& in = getIn<std::pair<const bool*, const bool*> >(cookie);
        test(arrayRangeEquals<bool>(out, in));
        test(arrayRangeEquals<bool>(ret, in));
        called();
    }

    void opByteArray(const pair<const Ice::Byte*, const Ice::Byte*>& ret,
                     const pair<const Ice::Byte*, const Ice::Byte*>& out,
                     const InParamPtr& cookie)
    {
        const pair<const Ice::Byte*, const Ice::Byte*>& in = getIn<pair<const Ice::Byte*, const Ice::Byte*> >(cookie);
        test(arrayRangeEquals<Ice::Byte>(out, in));
        test(arrayRangeEquals<Ice::Byte>(ret, in));
        called();
    }

    void opVariableArray(const pair<const Test::Variable*, const Test::Variable*>& ret,
                         const pair<const Test::Variable*, const Test::Variable*>& out,
                         const InParamPtr& cookie)
    {
        const pair<const Test::Variable*, const Test::Variable*>& in =
            getIn<pair<const Test::Variable*, const Test::Variable*> >(cookie);
        test(arrayRangeEquals<Test::Variable>(out, in));
        test(arrayRangeEquals<Test::Variable>(ret, in));
        called();
    }

    void opBoolRange(const pair<Test::BoolSeq::const_iterator, Test::BoolSeq::const_iterator>& ret,
                     const pair<Test::BoolSeq::const_iterator, Test::BoolSeq::const_iterator>& out,
                     const InParamPtr& cookie)
    {
        const pair<Test::BoolSeq::const_iterator, Test::BoolSeq::const_iterator>& in
            = getIn<pair<Test::BoolSeq::const_iterator, Test::BoolSeq::const_iterator> >(cookie);
        test(equal(out.first, out.second, in.first));
        test(equal(ret.first, ret.second, in.first));
        called();
    }

    void opByteRange(const pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>& ret,
                     const pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>& out,
                     const InParamPtr& cookie)
    {
        const pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>& in =
            getIn<pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator> >(cookie);
        test(equal(out.first, out.second, in.first));
        test(equal(ret.first, ret.second, in.first));
        called();
    }

    void opVariableRange(const pair<Test::VariableList::const_iterator, Test::VariableList::const_iterator>& ret,
                         const pair<Test::VariableList::const_iterator, Test::VariableList::const_iterator>& out,
                         const InParamPtr& cookie)
    {
        const pair<Test::VariableList::const_iterator, Test::VariableList::const_iterator>& in =
            getIn<pair<Test::VariableList::const_iterator, Test::VariableList::const_iterator> >(cookie);
        test(equal(out.first, out.second, in.first));
        test(equal(ret.first, ret.second, in.first));
        called();
    }

    void opByteRangeType(const pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>& ret,
                         const pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>& out,
                         const InParamPtr& cookie)
    {
        const pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>& in =
            getIn<pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator> >(cookie);
        test(equal(out.first, out.second, in.first));
        test(equal(ret.first, ret.second, in.first));
        called();
    }

    void opVariableRangeType(const pair<deque<Test::Variable>::const_iterator,
                             deque<Test::Variable>::const_iterator>& ret,
                             const pair<deque<Test::Variable>::const_iterator,
                             deque<Test::Variable>::const_iterator>& out,
                             const InParamPtr& cookie)
    {
        const pair<deque<Test::Variable>::const_iterator, deque<Test::Variable>::const_iterator>& in =
            getIn<pair<deque<Test::Variable>::const_iterator, deque<Test::Variable>::const_iterator> >(cookie);

        test(equal(out.first, out.second, in.first));
        test(equal(ret.first, ret.second, in.first));
        called();
    }

    void opBoolSeq(const deque<bool>& ret, const deque<bool>& out, const InParamPtr& cookie)
    {
        const deque<bool>& in = getIn<deque<bool> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opBoolList(const list<bool>& ret, const list<bool>& out, const InParamPtr& cookie)
    {
        const list<bool>& in = getIn<list<bool> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opByteSeq(const deque<Ice::Byte>& ret, const deque<Ice::Byte>& out, const InParamPtr& cookie)
    {
        const deque<Ice::Byte>& in = getIn< deque<Ice::Byte> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opByteList(const list<Ice::Byte>& ret, const list<Ice::Byte>& out, const InParamPtr& cookie)
    {
        const list<Ice::Byte>& in = getIn<list<Ice::Byte> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opMyByteSeq(const MyByteSeq& ret, const MyByteSeq& out, const InParamPtr& cookie)
    {
        const MyByteSeq& in = getIn<MyByteSeq>(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opStringSeq(const deque<string>& ret, const deque<string>& out, const InParamPtr& cookie)
    {
        const deque<string>& in = getIn<deque<string> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opStringList(const list<string>& ret, const list<string>& out, const InParamPtr& cookie)
    {
        const list<string>& in = getIn<list<string> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opFixedSeq(const deque<Test::Fixed>& ret, const deque<Test::Fixed>& out, const InParamPtr& cookie)
    {
        const deque<Test::Fixed>& in = getIn<deque<Test::Fixed> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opFixedList(const list<Test::Fixed>& ret, const list<Test::Fixed>& out, const InParamPtr& cookie)
    {
        const list<Test::Fixed>& in = getIn<list<Test::Fixed> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opVariableSeq(const deque<Test::Variable>& ret, const deque<Test::Variable>& out,
                       const InParamPtr& cookie)
    {
        const deque<Test::Variable>& in = getIn<deque<Test::Variable> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opVariableList(const list<Test::Variable>& ret, const list<Test::Variable>& out, const InParamPtr& cookie)
    {
        const list<Test::Variable>& in = getIn<list<Test::Variable> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opStringStringDictSeq(const deque<Test::StringStringDict>& ret, const deque<Test::StringStringDict>& out,
                               const InParamPtr& cookie)
    {
        const deque<Test::StringStringDict>& in = getIn<deque<Test::StringStringDict> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opStringStringDictList(const list<Test::StringStringDict>& ret, const list<Test::StringStringDict>& out,
                                const InParamPtr& cookie)
    {
        const list<Test::StringStringDict>& in = getIn<list<Test::StringStringDict> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opESeq(const deque<Test::E>& ret, const deque<Test::E>& out, const InParamPtr& cookie)
    {
        const deque<Test::E>& in = getIn<deque<Test::E> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opEList(const list<Test::E>& ret, const list<Test::E>& out, const InParamPtr& cookie)
    {
        const list<Test::E>& in = getIn<list<Test::E> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opDPrxSeq(const deque<Test::DPrxPtr>& ret, const deque<Test::DPrxPtr>& out, const InParamPtr& cookie)
    {
        const deque<Test::DPrxPtr>& in = getIn<deque<Test::DPrxPtr> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opDPrxList(const list<Test::DPrxPtr>& ret, const list<Test::DPrxPtr>& out, const InParamPtr& cookie)
    {
        const list<Test::DPrxPtr>& in = getIn<list<Test::DPrxPtr> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opCSeq(const deque<Test::CPtr>& ret, const deque<Test::CPtr>& out, const InParamPtr& cookie)
    {
        const deque<Test::CPtr>& in = getIn<deque<Test::CPtr> >(cookie);
        test(out.size() == in.size());
        test(ret.size() == in.size());
        for(unsigned int i = 1; i < in.size(); ++i)
        {
            test(out[i] == out[0]);
            test(ret[i] == out[i]);
        }
        called();
    }

    void opCList(const list<Test::CPtr>& ret, const list<Test::CPtr>& out, const InParamPtr& cookie)
    {
        const list<Test::CPtr>& in = getIn<list<Test::CPtr> >(cookie);
        test(out.size() == in.size());
        test(ret.size() == in.size());
        list<Test::CPtr>::const_iterator p1;
        list<Test::CPtr>::const_iterator p2;
        for(p1 = out.begin(), p2 = ret.begin(); p1 != out.end(); ++p1, ++p2)
        {
            test(*p1 == *p2);
        }
        called();
    }

    void opClassStruct(const ::Test::ClassStructPtr& ret,
                       const ::Test::ClassStructPtr& cs1,
                       const ::Test::ClassStructSeq& seq,
                       const InParamPtr& cookie)
    {
        pair< ::Test::ClassStructPtr, ::Test::ClassStructSeq> in =
            getIn<pair< ::Test::ClassStructPtr, ::Test::ClassStructSeq> >(cookie);
        test(ret == in.first);
        test(cs1 == in.first);
        test(seq == in.second);
        called();
    }

    void opString(const wstring& ret, const wstring& out, const InParamPtr& cookie)
    {
        const wstring& in = getIn<wstring>(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opOutArrayByteSeq(const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& data, const InParamPtr& cookie)
    {
        const Test::ByteSeq& in = getIn<Test::ByteSeq>(cookie);
        Test::ByteSeq out(data.first, data.second);
        Test::ByteSeq::const_iterator p1;
        Test::ByteSeq::const_iterator p2;

        for(p1 = out.begin(), p2 = in.begin(); p1 != out.end(); ++p1, ++p2)
        {
            test(*p1 == *p2);
        }
        called();
    }

    void opOutRangeByteSeq(const ::std::pair< ::Test::ByteSeq::const_iterator, ::Test::ByteSeq::const_iterator>& data,
                           const InParamPtr& cookie)
    {
        const Test::ByteSeq& in = getIn<Test::ByteSeq>(cookie);
        Test::ByteSeq out(data.first, data.second);
        Test::ByteSeq::const_iterator p1;
        Test::ByteSeq::const_iterator p2;

        for(p1 = out.begin(), p2 = in.begin(); p1 != out.end(); ++p1, ++p2)
        {
            test(*p1 == *p2);
        }
        called();
    }

    void opIntStringDict(const Test::IntStringDict& ret, const Test::IntStringDict& out, const InParamPtr& cookie)
    {
        const Test::IntStringDict& in = getIn<Test::IntStringDict>(cookie);

        test(ret == in);
        test(out == in);
        called();
    }

    void opVarDict(const Test::CustomMap<Ice::Long, Ice::Long>& ret,
                   const Test::CustomMap<std::string, Ice::Int>& out, const InParamPtr& cookie)
    {
        const Test::CustomMap<std::string, Ice::Int>& in = getIn<Test::CustomMap<std::string, Ice::Int> >(cookie);

        test(out == in);
        test(ret.size() == 1000);

        for(Test::CustomMap<Ice::Long, Ice::Long>::const_iterator i = ret.begin(); i != ret.end(); ++i)
        {
            test(i->second == i->first * i->first);
        }

        called();
    }

    void opCustomIntStringDict(const map<int, Util::string_view>& ret, const map<int, Util::string_view>& out, const InParamPtr& cookie)
    {
        const map<int, Util::string_view>& in = getIn<map<int, Util::string_view> >(cookie);

        test(out.size() == in.size());
        test(ret == out);

        for(map<int, Util::string_view>::const_iterator p = in.begin(); p != in.end(); ++p)
        {
            map<int, Util::string_view>::const_iterator q = out.find(p->first);
            test(q != out.end());
            test(q->second.size() == p->second.size());
        }

        called();
    }

    void throwExcept1(const Ice::AsyncResultPtr& result)
    {
        const wstring& in = getIn<wstring>(InParamPtr::dynamicCast(result->getCookie()));
        try
        {
            Test1::WstringClassPrx t = Test1::WstringClassPrx::uncheckedCast(result->getProxy());
            t->end_throwExcept(result);
        }
        catch(const Test1::WstringException& e)
        {
            test(e.reason == in);
            called();
        }
        catch(...)
        {
            test(false);
        }
    }

    void throwExcept1(const Ice::Exception& ex, const wstring& in)
    {
        try
        {
            ex.ice_throw();
        }
        catch(const Test1::WstringException& e)
        {
            test(e.reason == in);
            called();
        }
        catch(...)
        {
            test(false);
        }
    }

    void throwExcept2(const Ice::AsyncResultPtr& result)
    {
        const wstring& in = getIn<wstring>(InParamPtr::dynamicCast(result->getCookie()));
        try
        {
            Test2::WstringClassPrx t = Test2::WstringClassPrx::uncheckedCast(result->getProxy());
            t->end_throwExcept(result);
        }
        catch(const Test2::WstringException& e)
        {
            test(e.reason == in);
            called();
        }
        catch(...)
        {
            test(false);
        }
    }

    void throwExcept2(const Ice::Exception& ex, const wstring& in)
    {
        try
        {
            ex.ice_throw();
        }
        catch(const Test2::WstringException& e)
        {
            test(e.reason == in);
            called();
        }
        catch(...)
        {
            test(false);
        }
    }

    void
    noEx(const Ice::Exception& ex, const InParamPtr&)
    {
        cerr << ex << endl;
        test(false);
    }
};
typedef IceUtil::Handle<Callback> CallbackPtr;

#endif

Test::TestIntfPrxPtr
allTests(const Ice::CommunicatorPtr& communicator)
{
    const string endp = getTestEndpoint(communicator, 0);
    cout << "testing stringToProxy... " << flush;
    string ref = communicator->getProperties()->getPropertyWithDefault("Custom.Proxy", "test:" + endp);
    Ice::ObjectPrxPtr base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;

    Test::TestIntfPrxPtr t = ICE_CHECKED_CAST(Test::TestIntfPrx, base);
    test(t);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(t, base));
#else
    test(t == base);
#endif
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
#ifdef ICE_CPP11_MAPPING
        test(ret == in);
#else
        //
        // When a parameter is mapped as a string_view on both sides,
        // no conversion occurs, and all is good
        // However, when a parameter is mapped as a string on one side and
        // as a string_view on the other (e.g. sync invocation implemented with AMD)
        // the string converter converts the string but not the string_view,
        // and we have a mismatch
        //
        test(ret.size() == in.size());
#endif
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
        for(int i = 0; i < 5; ++i)
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
        Test::BoolSeq in(5);
        in[0] = false;
        in[1] = true;
        in[2] = true;
        in[3] = false;
        in[4] = true;
        Test::BoolSeq out;
#ifdef ICE_CPP11_MAPPING
        Test::BoolSeq ret = t->opBoolRange(in, out);
#else
        pair<Test::BoolSeq::const_iterator, Test::BoolSeq::const_iterator> inPair(in.begin(), in.end());
        Test::BoolSeq ret = t->opBoolRange(inPair, out);
#endif
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
        Test::ByteList out;
#ifdef ICE_CPP11_MAPPING
        Test::ByteList ret = t->opByteRange(in, out);
#else
        pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator> inPair(in.begin(), in.end());
        Test::ByteList ret = t->opByteRange(inPair, out);
#endif
        test(out == in);
        test(ret == in);
    }

    {
        Test::VariableList in;
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
        Test::VariableList out;
#ifdef ICE_CPP11_MAPPING
        Test::VariableList ret = t->opVariableRange(in, out);
#else
        pair<Test::VariableList::const_iterator, Test::VariableList::const_iterator> inPair(in.begin(), in.end());
        Test::VariableList ret = t->opVariableRange(inPair, out);
#endif
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
        Test::ByteList out;
#ifdef ICE_CPP11_MAPPING
        Test::ByteList ret = t->opByteRangeType(in, out);
#else
        pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator> inPair(in.begin(), in.end());
        Test::ByteList ret = t->opByteRangeType(inPair, out);
#endif
        test(out == in);
        test(ret == in);
    }

    {
        Test::VariableList in;
        deque<Test::Variable> inSeq;
        Test::Variable v;
        v.s = "THESE";
        in.push_back(v);
        inSeq.push_back(v);
        v.s = "ARE";
        in.push_back(v);
        inSeq.push_back(v);
        v.s = "FIVE";
        in.push_back(v);
        inSeq.push_back(v);
        v.s = "SHORT";
        in.push_back(v);
        inSeq.push_back(v);
        v.s = "STRINGS.";
        in.push_back(v);
        inSeq.push_back(v);
        Test::VariableList out;
#ifdef ICE_CPP11_MAPPING
        Test::VariableList ret = t->opVariableRangeType(in, out);
#else
        pair<deque<Test::Variable>::const_iterator, deque<Test::Variable>::const_iterator>
            inPair(inSeq.begin(), inSeq.end());
        Test::VariableList ret = t->opVariableRangeType(inPair, out);
#endif
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
        in[0] = Test:: ICE_ENUM(E, E1);
        in[1] = Test:: ICE_ENUM(E, E2);
        in[2] = Test:: ICE_ENUM(E, E3);
        in[3] = Test:: ICE_ENUM(E, E1);
        in[4] = Test:: ICE_ENUM(E, E3);

        deque<Test::E> out;
        deque<Test::E> ret = t->opESeq(in, out);
        test(out == in);
        test(ret == in);
    }

    {
        list<Test::E> in;
        in.push_back(Test:: ICE_ENUM(E, E1));
        in.push_back(Test:: ICE_ENUM(E, E2));
        in.push_back(Test:: ICE_ENUM(E, E3));
        in.push_back(Test:: ICE_ENUM(E, E1));
        in.push_back(Test:: ICE_ENUM(E, E3));

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

#ifdef ICE_CPP11_MAPPING
        auto op = out.begin();
        auto rp = ret.begin();

        for(auto i: in)
        {
            test(Ice::targetEqualTo(*op++, i));
            test(Ice::targetEqualTo(*rp++, i));
        }
#else
        test(out == in);
        test(ret == in);
#endif
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
#ifdef ICE_CPP11_MAPPING
        auto op = out.begin();
        auto rp = ret.begin();

        for(auto i: in)
        {
            test(Ice::targetEqualTo(*op++, i));
            test(Ice::targetEqualTo(*rp++, i));
        }
#else
        test(out == in);
        test(ret == in);
#endif
    }

    {
        deque<Test::CPtr> in(5);
        in[0] = ICE_MAKE_SHARED(Test::C);
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
        in.push_back(ICE_MAKE_SHARED(Test::C));
        in.push_back(ICE_MAKE_SHARED(Test::C));
        in.push_back(ICE_MAKE_SHARED(Test::C));
        in.push_back(ICE_MAKE_SHARED(Test::C));
        in.push_back(ICE_MAKE_SHARED(Test::C));

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
#ifdef ICE_CPP11_MAPPING
            test(out[p->first] == p->second.to_string());
#else
            test(out[p->first].size() == p->second.size());
#endif
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

#ifdef ICE_CPP11_MAPPING

        auto r = t->opStringAsync(in).get();
        test(r.returnValue == r.outString);
        test(r.returnValue == in);

#else
        Ice::AsyncResultPtr r = t->begin_opString(in);
        string out;
        string ret = t->end_opString(out, r);

        test(ret == out);
        test(ret.size() == in.size());
#endif
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
            for(int i = 0; i < 5; ++i)
            {
                inArray[i] = in[i];
            }
            pair<const Ice::Double*, const Ice::Double*> inPair(inArray, inArray + 5);

#ifdef ICE_CPP11_MAPPING
            auto r = t->opDoubleArrayAsync(inPair).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            Test::DoubleSeq out;
            Ice::AsyncResultPtr r = t->begin_opDoubleArray(inPair);
            Test::DoubleSeq ret = t->end_opDoubleArray(out, r);
            test(out == in);
            test(ret == in);
#endif
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

#ifdef ICE_CPP11_MAPPING
            auto r = t->opBoolArrayAsync(inPair).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else

            Test::BoolSeq out;
            Ice::AsyncResultPtr r = t->begin_opBoolArray(inPair);
            Test::BoolSeq ret = t->end_opBoolArray(out, r);
            test(out == in);
            test(ret == in);
#endif
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

#ifdef ICE_CPP11_MAPPING
            auto r = t->opByteArrayAsync(inPair).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            Test::ByteList out;
            Ice::AsyncResultPtr r = t->begin_opByteArray(inPair);
            Test::ByteList ret = t->end_opByteArray(out, r);
            test(out == in);
            test(ret == in);
#endif
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

#ifdef ICE_CPP11_MAPPING
            auto r = t->opVariableArrayAsync(inPair).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            Test::VariableList out;
            Ice::AsyncResultPtr r = t->begin_opVariableArray(inPair);
            Test::VariableList ret = t->end_opVariableArray(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            Test::BoolSeq in(5);
            in[0] = false;
            in[1] = true;
            in[2] = true;
            in[3] = false;
            in[4] = true;
#ifdef ICE_CPP11_MAPPING
            auto r = t->opBoolRangeAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            pair<Test::BoolSeq::const_iterator, Test::BoolSeq::const_iterator> inPair(in.begin(), in.end());
            Test::BoolSeq out;
            Ice::AsyncResultPtr r = t->begin_opBoolRange(inPair);
            Test::BoolSeq ret = t->end_opBoolRange(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            Test::ByteList in;
            in.push_back('1');
            in.push_back('2');
            in.push_back('3');
            in.push_back('4');
            in.push_back('5');

#ifdef ICE_CPP11_MAPPING
            auto r = t->opByteRangeAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator> inPair(in.begin(), in.end());
            Test::ByteList out;
            Ice::AsyncResultPtr r = t->begin_opByteRange(inPair);
            Test::ByteList ret = t->end_opByteRange(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            Test::VariableList in;
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

#ifdef ICE_CPP11_MAPPING
            auto r = t->opVariableRangeAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            pair<Test::VariableList::const_iterator, Test::VariableList::const_iterator> inPair(in.begin(), in.end());
            Test::VariableList out;
            Ice::AsyncResultPtr r = t->begin_opVariableRange(inPair);
            Test::VariableList ret = t->end_opVariableRange(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            Test::ByteList in;
            in.push_back('1');
            in.push_back('2');
            in.push_back('3');
            in.push_back('4');
            in.push_back('5');
#ifdef ICE_CPP11_MAPPING
            auto r = t->opByteRangeTypeAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator> inPair(in.begin(), in.end());
            Test::ByteList out;
            Ice::AsyncResultPtr r = t->begin_opByteRangeType(inPair);
            Test::ByteList ret = t->end_opByteRangeType(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            Test::VariableList in;
            deque<Test::Variable> inSeq;
            Test::Variable v;
            v.s = "THESE";
            in.push_back(v);
            inSeq.push_back(v);
            v.s = "ARE";
            in.push_back(v);
            inSeq.push_back(v);
            v.s = "FIVE";
            in.push_back(v);
            inSeq.push_back(v);
            v.s = "SHORT";
            in.push_back(v);
            inSeq.push_back(v);
            v.s = "STRINGS.";
            in.push_back(v);
            inSeq.push_back(v);

#ifdef ICE_CPP11_MAPPING
            auto r = t->opVariableRangeTypeAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            pair<deque<Test::Variable>::const_iterator, deque<Test::Variable>::const_iterator>
                inPair(inSeq.begin(), inSeq.end());
            Test::VariableList out;
            Ice::AsyncResultPtr r = t->begin_opVariableRangeType(inPair);
            Test::VariableList ret = t->end_opVariableRangeType(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            deque<bool> in(5);
            in[0] = false;
            in[1] = true;
            in[2] = true;
            in[3] = false;
            in[4] = true;

#ifdef ICE_CPP11_MAPPING
            auto r = t->opBoolSeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            deque<bool> out;
            Ice::AsyncResultPtr r = t->begin_opBoolSeq(in);
            deque<bool> ret = t->end_opBoolSeq(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            list<bool> in;
            in.push_back(false);
            in.push_back(true);
            in.push_back(true);
            in.push_back(false);
            in.push_back(true);

#ifdef ICE_CPP11_MAPPING
            auto r = t->opBoolListAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            list<bool> out;
            Ice::AsyncResultPtr r = t->begin_opBoolList(in);
            list<bool> ret = t->end_opBoolList(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            deque< ::Ice::Byte> in(5);
            in[0] = '1';
            in[1] = '2';
            in[2] = '3';
            in[3] = '4';
            in[4] = '5';

#ifdef ICE_CPP11_MAPPING
            auto r = t->opByteSeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            deque< ::Ice::Byte> out;
            Ice::AsyncResultPtr r = t->begin_opByteSeq(in);
            deque< ::Ice::Byte> ret = t->end_opByteSeq(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            list< ::Ice::Byte> in;
            in.push_back('1');
            in.push_back('2');
            in.push_back('3');
            in.push_back('4');
            in.push_back('5');

#ifdef ICE_CPP11_MAPPING
            auto r = t->opByteListAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            list< ::Ice::Byte> out;
            Ice::AsyncResultPtr r = t->begin_opByteList(in);
            list< ::Ice::Byte> ret = t->end_opByteList(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            MyByteSeq in(5);
            int i = 0;
            for(MyByteSeq::iterator p = in.begin(); p != in.end(); ++p)
            {
                *p = '1' + i++;
            }

#ifdef ICE_CPP11_MAPPING
            auto r = t->opMyByteSeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            MyByteSeq out;
            Ice::AsyncResultPtr r = t->begin_opMyByteSeq(in);
            MyByteSeq ret = t->end_opMyByteSeq(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            deque<string> in(5);
            in[0] = "THESE";
            in[1] = "ARE";
            in[2] = "FIVE";
            in[3] = "SHORT";
            in[4] = "STRINGS.";

#ifdef ICE_CPP11_MAPPING
            auto r = t->opStringSeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            deque<string> out;
            Ice::AsyncResultPtr r = t->begin_opStringSeq(in);
            deque<string> ret = t->end_opStringSeq(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            list<string> in;
            in.push_back("THESE");
            in.push_back("ARE");
            in.push_back("FIVE");
            in.push_back("SHORT");
            in.push_back("STRINGS.");

#ifdef ICE_CPP11_MAPPING
            auto r = t->opStringListAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            list<string> out;
            Ice::AsyncResultPtr r = t->begin_opStringList(in);
            list<string> ret = t->end_opStringList(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            deque<Test::Fixed> in(5);
            in[0].s = 1;
            in[1].s = 2;
            in[2].s = 3;
            in[3].s = 4;
            in[4].s = 5;

#ifdef ICE_CPP11_MAPPING
            auto r = t->opFixedSeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            deque<Test::Fixed> out;
            Ice::AsyncResultPtr r = t->begin_opFixedSeq(in);
            deque<Test::Fixed> ret = t->end_opFixedSeq(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            list<Test::Fixed> in(5);
            short num = 1;
            for(list<Test::Fixed>::iterator p = in.begin(); p != in.end(); ++p)
            {
                (*p).s = num++;
            }

#ifdef ICE_CPP11_MAPPING
            auto r = t->opFixedListAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            list<Test::Fixed> out;
            Ice::AsyncResultPtr r = t->begin_opFixedList(in);
            list<Test::Fixed> ret = t->end_opFixedList(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            deque<Test::Variable> in(5);
            in[0].s = "THESE";
            in[1].s = "ARE";
            in[2].s = "FIVE";
            in[3].s = "SHORT";
            in[4].s = "STRINGS.";

#ifdef ICE_CPP11_MAPPING
            auto r = t->opVariableSeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            deque<Test::Variable> out;
            Ice::AsyncResultPtr r = t->begin_opVariableSeq(in);
            deque<Test::Variable> ret = t->end_opVariableSeq(out, r);
            test(out == in);
            test(ret == in);
#endif
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

#ifdef ICE_CPP11_MAPPING
            auto r = t->opVariableListAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            list<Test::Variable> out;
            Ice::AsyncResultPtr r = t->begin_opVariableList(in);
            list<Test::Variable> ret = t->end_opVariableList(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            deque<Test::StringStringDict> in(5);
            in[0]["A"] = "A";
            in[1]["B"] = "B";
            in[2]["C"] = "C";
            in[3]["D"] = "D";
            in[4]["E"] = "E";

#ifdef ICE_CPP11_MAPPING
            auto r = t->opStringStringDictSeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            deque<Test::StringStringDict> out;
            Ice::AsyncResultPtr r = t->begin_opStringStringDictSeq(in);
            deque<Test::StringStringDict> ret = t->end_opStringStringDictSeq(out, r);
            test(out == in);
            test(ret == in);
#endif
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

#ifdef ICE_CPP11_MAPPING
            auto r = t->opStringStringDictListAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            list<Test::StringStringDict> out;
            Ice::AsyncResultPtr r = t->begin_opStringStringDictList(in);
            list<Test::StringStringDict> ret = t->end_opStringStringDictList(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            deque<Test::E> in(5);
            in[0] = Test:: ICE_ENUM(E, E1);
            in[1] = Test:: ICE_ENUM(E, E2);
            in[2] = Test:: ICE_ENUM(E, E3);
            in[3] = Test:: ICE_ENUM(E, E1);
            in[4] = Test:: ICE_ENUM(E, E3);

#ifdef ICE_CPP11_MAPPING
            auto r = t->opESeqAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            deque<Test::E> out;
            Ice::AsyncResultPtr r = t->begin_opESeq(in);
            deque<Test::E> ret = t->end_opESeq(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            list<Test::E> in;
            in.push_back(Test:: ICE_ENUM(E, E1));
            in.push_back(Test:: ICE_ENUM(E, E2));
            in.push_back(Test:: ICE_ENUM(E, E3));
            in.push_back(Test:: ICE_ENUM(E, E1));
            in.push_back(Test:: ICE_ENUM(E, E3));

#ifdef ICE_CPP11_MAPPING
            auto r = t->opEListAsync(in).get();
            test(r.outSeq == in);
            test(r.returnValue == in);
#else
            list<Test::E> out;
            Ice::AsyncResultPtr r = t->begin_opEList(in);
            list<Test::E> ret = t->end_opEList(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            deque<Test::DPrxPtr> in(5);
            in[0] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C1:" + endp + " -t 10000"));
            in[1] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C2:" + endp + " -t 10001"));
            in[2] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C3:" + endp + " -t 10002"));
            in[3] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C4:" + endp + " -t 10003"));
            in[4] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C5:" + endp + " -t 10004"));

#ifdef ICE_CPP11_MAPPING
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
#else
            deque<Test::DPrx> out;
            Ice::AsyncResultPtr r = t->begin_opDPrxSeq(in);
            deque<Test::DPrx> ret = t->end_opDPrxSeq(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            list<Test::DPrxPtr> in;
            in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C1:" + endp + " -t 10000")));
            in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C2:" + endp + " -t 10001")));
            in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C3:" + endp + " -t 10002")));
            in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C4:" + endp + " -t 10003")));
            in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C5:" + endp + " -t 10004")));

#ifdef ICE_CPP11_MAPPING
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
#else
            list<Test::DPrx> out;
            Ice::AsyncResultPtr r = t->begin_opDPrxList(in);
            list<Test::DPrx> ret = t->end_opDPrxList(out, r);
            test(out == in);
            test(ret == in);
#endif
        }

        {
            deque<Test::CPtr> in(5);
            in[0] = ICE_MAKE_SHARED(Test::C);
            in[1] = in[0];
            in[2] = in[0];
            in[3] = in[0];
            in[4] = in[0];

#ifdef ICE_CPP11_MAPPING
            auto r = t->opCSeqAsync(in).get();
            test(r.outSeq.size() == in.size());
            test(r.returnValue.size() == in.size());

            auto rp = r.returnValue.begin();
            for(auto o: r.outSeq)
            {
                test(o == r.outSeq[0]);
                test(*rp++ == o);
            }
#else
            deque<Test::CPtr> out;
            Ice::AsyncResultPtr r = t->begin_opCSeq(in);
            deque<Test::CPtr> ret = t->end_opCSeq(out, r);
            test(out.size() == in.size());
            test(ret.size() == in.size());
            for(unsigned int i = 1; i < in.size(); ++i)
            {
                test(out[i] == out[0]);
                test(ret[i] == out[i]);
            }
#endif
        }

        {
            list<Test::CPtr> in;
            in.push_back(ICE_MAKE_SHARED(Test::C));
            in.push_back(ICE_MAKE_SHARED(Test::C));
            in.push_back(ICE_MAKE_SHARED(Test::C));
            in.push_back(ICE_MAKE_SHARED(Test::C));
            in.push_back(ICE_MAKE_SHARED(Test::C));

#ifdef ICE_CPP11_MAPPING
            auto r = t->opCListAsync(in).get();
            test(r.outSeq.size() == in.size());
            test(r.returnValue.size() == in.size());
            test(r.outSeq == r.returnValue);
#else
            list<Test::CPtr> out;
            Ice::AsyncResultPtr r = t->begin_opCList(in);
            list<Test::CPtr> ret = t->end_opCList(out, r);
            test(out.size() == in.size());
            test(ret.size() == in.size());
            list<Test::CPtr>::const_iterator p1;
            list<Test::CPtr>::const_iterator p2;
            for(p1 = out.begin(), p2 = ret.begin(); p1 != out.end(); ++p1, ++p2)
            {
                test(*p1 == *p2);
            }
#endif
        }

        {
            Test::ByteSeq in;
            in.push_back('1');
            in.push_back('2');
            in.push_back('3');
            in.push_back('4');

#ifdef ICE_CPP11_MAPPING
            auto r = t->opOutArrayByteSeqAsync(in).get();
            test(r.size() == in.size());
            test(r == in);
#else
            Ice::AsyncResultPtr r = t->begin_opOutArrayByteSeq(in);

            Test::ByteSeq out;
            t->end_opOutArrayByteSeq(out, r);

            test(out.size() == in.size());
            Test::ByteSeq::const_iterator p1;
            Test::ByteSeq::const_iterator p2;

            for(p1 = out.begin(), p2 = in.begin(); p1 != out.end(); ++p1, ++p2)
            {
                test(*p1 == *p2);
            }
#endif
        }

        {
            Test::ByteSeq in;
            in.push_back('1');
            in.push_back('2');
            in.push_back('3');
            in.push_back('4');

#ifdef ICE_CPP11_MAPPING
            auto r = t->opOutRangeByteSeqAsync(in).get();
            test(r.size() == in.size());
            test(r == in);
#else
            Ice::AsyncResultPtr r = t->begin_opOutRangeByteSeq(in);

            Test::ByteSeq out;
            t->end_opOutRangeByteSeq(out, r);

            test(out.size() == in.size());
            Test::ByteSeq::const_iterator p1;
            Test::ByteSeq::const_iterator p2;

            for(p1 = out.begin(), p2 = in.begin(); p1 != out.end(); ++p1, ++p2)
            {
                test(*p1 == *p2);
            }
#endif
        }
    }
    cout << "ok" << endl;

    cout << "testing alternate strings with AMI callbacks... " << flush;
    {
        Util::string_view in = "HELLO WORLD!";

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();

        Test::Callback_TestIntf_opStringPtr callback =
            Test::newCallback_TestIntf_opString(cb, &Callback::opString, &Callback::noEx);
        t->begin_opString(in, callback, newInParam(in));
        cb->check();
#endif
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
        for(int i = 0; i < 5; ++i)
        {
            inArray[i] = in[i];
        }
        pair<const Ice::Double*, const Ice::Double*> inPair(inArray, inArray + 5);

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opDoubleArrayPtr callback =
            Test::newCallback_TestIntf_opDoubleArray(cb, &Callback::opDoubleArray, &Callback::noEx);
        t->begin_opDoubleArray(inPair, callback, newInParam(inPair));
        cb->check();
#endif
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

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opBoolArrayPtr callback =
            Test::newCallback_TestIntf_opBoolArray(cb, &Callback::opBoolArray, &Callback::noEx);
        t->begin_opBoolArray(inPair, callback, newInParam(inPair));
        cb->check();
#endif
    }

    {
        Ice::Byte in[5];
        in[0] = '1';
        in[1] = '2';
        in[2] = '3';
        in[3] = '4';
        in[4] = '5';
        pair<const Ice::Byte*, const Ice::Byte*> inPair(in, in + 5);

#ifdef ICE_CPP11_MAPPING

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
#else

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opByteArrayPtr callback =
            Test::newCallback_TestIntf_opByteArray(cb, &Callback::opByteArray, &Callback::noEx);
        t->begin_opByteArray(inPair, callback, newInParam(inPair));
        cb->check();
#endif
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

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opVariableArrayPtr callback =
            Test::newCallback_TestIntf_opVariableArray(cb, &Callback::opVariableArray, &Callback::noEx);
        t->begin_opVariableArray(inPair, callback, newInParam(inPair));
        cb->check();
#endif
    }

    {
        Test::BoolSeq in(5);
        in[0] = false;
        in[1] = true;
        in[2] = true;
        in[3] = false;
        in[4] = true;

#ifdef ICE_CPP11_MAPPING

        promise<bool> done;

        t->opBoolRangeAsync(in,
                            [&](Test::BoolSeq ret, Test::BoolSeq out)
                            {
                                   test(ret == in);
                                   test(out == in);
                                   done.set_value(true);
                            },
                            [&](std::exception_ptr)
                            {
                                done.set_value(false);
                            });

        test(done.get_future().get());
#else
        pair<Test::BoolSeq::const_iterator, Test::BoolSeq::const_iterator> inPair(in.begin(), in.end());

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opBoolRangePtr callback =
            Test::newCallback_TestIntf_opBoolRange(cb, &Callback::opBoolRange, &Callback::noEx);
        t->begin_opBoolRange(inPair, callback, newInParam(inPair));
        cb->check();
#endif
    }

    {
        Test::ByteList in;
        in.push_back('1');
        in.push_back('2');
        in.push_back('3');
        in.push_back('4');
        in.push_back('5');

#ifdef ICE_CPP11_MAPPING

        promise<bool> done;

        t->opByteRangeAsync(in,
                            [&](Test::ByteList ret, Test::ByteList out)
                            {
                                test(ret == in);
                                test(out == in);
                                done.set_value(true);
                            },
                            [&](std::exception_ptr)
                            {
                                done.set_value(false);
                            });

        test(done.get_future().get());
#else
        pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator> inPair(in.begin(), in.end());
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opByteRangePtr callback =
            Test::newCallback_TestIntf_opByteRange(cb, &Callback::opByteRange, &Callback::noEx);
        t->begin_opByteRange(inPair, callback, newInParam(inPair));
        cb->check();
#endif
    }

    {
        Test::VariableList in;
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

#ifdef ICE_CPP11_MAPPING

        promise<bool> done;

        t->opVariableRangeAsync(in,
                                [&](Test::VariableList ret, Test::VariableList out)
                                {
                                    test(ret == in);
                                    test(out == in);
                                    done.set_value(true);
                                },
                                [&](std::exception_ptr)
                                {
                                    done.set_value(false);
                                });

        test(done.get_future().get());
#else
        pair<Test::VariableList::const_iterator, Test::VariableList::const_iterator> inPair(in.begin(), in.end());
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opVariableRangePtr callback =
            Test::newCallback_TestIntf_opVariableRange(cb, &Callback::opVariableRange, &Callback::noEx);
        t->begin_opVariableRange(inPair, callback, newInParam(inPair));
        cb->check();
#endif
    }

    {
        Test::ByteList in;
        in.push_back('1');
        in.push_back('2');
        in.push_back('3');
        in.push_back('4');
        in.push_back('5');

#ifdef ICE_CPP11_MAPPING

        promise<bool> done;

        t->opByteRangeTypeAsync(in,
                                [&](Test::ByteList ret, Test::ByteList out)
                                {
                                    test(ret == in);
                                    test(out == in);
                                    done.set_value(true);
                                },
                                [&](std::exception_ptr)
                                {
                                    done.set_value(false);
                                });

        test(done.get_future().get());
#else
        pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator> inPair(in.begin(), in.end());
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opByteRangeTypePtr callback =
            Test::newCallback_TestIntf_opByteRangeType(cb, &Callback::opByteRangeType, &Callback::noEx);
        t->begin_opByteRangeType(inPair, callback, newInParam(inPair));
        cb->check();
#endif
    }

    {
        Test::VariableList in;
        deque<Test::Variable> inSeq;
        Test::Variable v;
        v.s = "THESE";
        in.push_back(v);
        inSeq.push_back(v);
        v.s = "ARE";
        in.push_back(v);
        inSeq.push_back(v);
        v.s = "FIVE";
        in.push_back(v);
        inSeq.push_back(v);
        v.s = "SHORT";
        in.push_back(v);
        inSeq.push_back(v);
        v.s = "STRINGS.";
        in.push_back(v);
        inSeq.push_back(v);
#ifdef ICE_CPP11_MAPPING

        promise<bool> done;

        t->opVariableRangeTypeAsync(in,
                                    [&](Test::VariableList ret, Test::VariableList out)
                                    {
                                        test(ret == in);
                                        test(out == in);
                                        done.set_value(true);
                                    },
                                    [&](std::exception_ptr)
                                    {
                                        done.set_value(false);
                                    });

        test(done.get_future().get());
#else
        pair<deque<Test::Variable>::const_iterator, deque<Test::Variable>::const_iterator> inPair(inSeq.begin(),
                                                                                                  inSeq.end());
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opVariableRangeTypePtr callback =
            Test::newCallback_TestIntf_opVariableRangeType(cb, &Callback::opVariableRangeType, &Callback::noEx);
        t->begin_opVariableRangeType(inPair, callback, newInParam(inPair));
        cb->check();
#endif
    }

    {
        deque<bool> in(5);
        in[0] = false;
        in[1] = true;
        in[2] = true;
        in[3] = false;
        in[4] = true;

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opBoolSeqPtr callback =
            Test::newCallback_TestIntf_opBoolSeq(cb, &Callback::opBoolSeq, &Callback::noEx);
        t->begin_opBoolSeq(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        list<bool> in;
        in.push_back(false);
        in.push_back(true);
        in.push_back(true);
        in.push_back(false);
        in.push_back(true);

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opBoolListPtr callback =
            Test::newCallback_TestIntf_opBoolList(cb, &Callback::opBoolList, &Callback::noEx);
        t->begin_opBoolList(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        deque< ::Ice::Byte> in(5);
        in[0] = '1';
        in[1] = '2';
        in[2] = '3';
        in[3] = '4';
        in[4] = '5';

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opByteSeqPtr callback =
            Test::newCallback_TestIntf_opByteSeq(cb, &Callback::opByteSeq, &Callback::noEx);
        t->begin_opByteSeq(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        list< ::Ice::Byte> in;
        in.push_back('1');
        in.push_back('2');
        in.push_back('3');
        in.push_back('4');
        in.push_back('5');

#ifdef ICE_CPP11_MAPPING

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
#else

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opByteListPtr callback =
            Test::newCallback_TestIntf_opByteList(cb, &Callback::opByteList, &Callback::noEx);
        t->begin_opByteList(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        MyByteSeq in(5);
        int i = 0;
        for(MyByteSeq::iterator p = in.begin(); p != in.end(); ++p)
        {
            *p = '1' + i++;
        }

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opMyByteSeqPtr callback =
            Test::newCallback_TestIntf_opMyByteSeq(cb, &Callback::opMyByteSeq, &Callback::noEx);
        t->begin_opMyByteSeq(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        deque<string> in(5);
        in[0] = "THESE";
        in[1] = "ARE";
        in[2] = "FIVE";
        in[3] = "SHORT";
        in[4] = "STRINGS.";

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opStringSeqPtr callback =
            Test::newCallback_TestIntf_opStringSeq(cb, &Callback::opStringSeq, &Callback::noEx);
        t->begin_opStringSeq(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        list<string> in;
        in.push_back("THESE");
        in.push_back("ARE");
        in.push_back("FIVE");
        in.push_back("SHORT");
        in.push_back("STRINGS.");

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opStringListPtr callback =
            Test::newCallback_TestIntf_opStringList(cb, &Callback::opStringList, &Callback::noEx);
        t->begin_opStringList(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        deque<Test::Fixed> in(5);
        in[0].s = 1;
        in[1].s = 2;
        in[2].s = 3;
        in[3].s = 4;
        in[4].s = 5;

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opFixedSeqPtr callback =
            Test::newCallback_TestIntf_opFixedSeq(cb, &Callback::opFixedSeq, &Callback::noEx);
        t->begin_opFixedSeq(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        list<Test::Fixed> in(5);
        short num = 1;
        for(list<Test::Fixed>::iterator p = in.begin(); p != in.end(); ++p)
        {
            (*p).s = num++;
        }

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opFixedListPtr callback =
            Test::newCallback_TestIntf_opFixedList(cb, &Callback::opFixedList, &Callback::noEx);
        t->begin_opFixedList(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        deque<Test::Variable> in(5);
        in[0].s = "THESE";
        in[1].s = "ARE";
        in[2].s = "FIVE";
        in[3].s = "SHORT";
        in[4].s = "STRINGS.";

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opVariableSeqPtr callback =
            Test::newCallback_TestIntf_opVariableSeq(cb, &Callback::opVariableSeq, &Callback::noEx);
        t->begin_opVariableSeq(in, callback, newInParam(in));
        cb->check();
#endif
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

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opVariableListPtr callback =
            Test::newCallback_TestIntf_opVariableList(cb, &Callback::opVariableList, &Callback::noEx);
        t->begin_opVariableList(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        deque<Test::StringStringDict> in(5);
        in[0]["A"] = "A";
        in[1]["B"] = "B";
        in[2]["C"] = "C";
        in[3]["D"] = "D";
        in[4]["E"] = "E";

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opStringStringDictSeqPtr callback =
            Test::newCallback_TestIntf_opStringStringDictSeq(cb, &Callback::opStringStringDictSeq, &Callback::noEx);
        t->begin_opStringStringDictSeq(in, callback, newInParam(in));
        cb->check();
#endif
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

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opStringStringDictListPtr callback =
            Test::newCallback_TestIntf_opStringStringDictList(cb, &Callback::opStringStringDictList, &Callback::noEx);
        t->begin_opStringStringDictList(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        deque<Test::E> in(5);
        in[0] = Test:: ICE_ENUM(E, E1);
        in[1] = Test:: ICE_ENUM(E, E2);
        in[2] = Test:: ICE_ENUM(E, E3);
        in[3] = Test:: ICE_ENUM(E, E1);
        in[4] = Test:: ICE_ENUM(E, E3);

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opESeqPtr callback =
            Test::newCallback_TestIntf_opESeq(cb, &Callback::opESeq, &Callback::noEx);
        t->begin_opESeq(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        list<Test::E> in;
        in.push_back(Test:: ICE_ENUM(E, E1));
        in.push_back(Test:: ICE_ENUM(E, E2));
        in.push_back(Test:: ICE_ENUM(E, E3));
        in.push_back(Test:: ICE_ENUM(E, E1));
        in.push_back(Test:: ICE_ENUM(E, E3));

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opEListPtr callback =
            Test::newCallback_TestIntf_opEList(cb, &Callback::opEList, &Callback::noEx);
        t->begin_opEList(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        deque<Test::DPrxPtr> in(5);
        in[0] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C1:" + endp + " -t 10000"));
        in[1] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C2:" + endp + " -t 10001"));
        in[2] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C3:" + endp + " -t 10002"));
        in[3] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C4:" + endp + " -t 10003"));
        in[4] = ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C5:" + endp + " -t 10004"));

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opDPrxSeqPtr callback =
            Test::newCallback_TestIntf_opDPrxSeq(cb, &Callback::opDPrxSeq, &Callback::noEx);
        t->begin_opDPrxSeq(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        list<Test::DPrxPtr> in;
        in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C1:" + endp + " -t 10000")));
        in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C2:" + endp + " -t 10001")));
        in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C3:" + endp + " -t 10002")));
        in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C4:" + endp + " -t 10003")));
        in.push_back(ICE_UNCHECKED_CAST(Test::DPrx, communicator->stringToProxy("C5:" + endp + " -t 10004")));

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opDPrxListPtr callback =
            Test::newCallback_TestIntf_opDPrxList(cb, &Callback::opDPrxList, &Callback::noEx);
        t->begin_opDPrxList(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        deque<Test::CPtr> in(5);
        in[0] = ICE_MAKE_SHARED(Test::C);
        in[1] = in[0];
        in[2] = in[0];
        in[3] = in[0];
        in[4] = in[0];

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opCSeqPtr callback = Test::newCallback_TestIntf_opCSeq(cb, &Callback::opCSeq, &Callback::noEx);
        t->begin_opCSeq(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        list<Test::CPtr> in;
        in.push_back(ICE_MAKE_SHARED(Test::C));
        in.push_back(ICE_MAKE_SHARED(Test::C));
        in.push_back(ICE_MAKE_SHARED(Test::C));
        in.push_back(ICE_MAKE_SHARED(Test::C));
        in.push_back(ICE_MAKE_SHARED(Test::C));

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opCListPtr callback =
            Test::newCallback_TestIntf_opCList(cb, &Callback::opCList, &Callback::noEx);
        t->begin_opCList(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        Test::ByteSeq in;
        in.push_back('1');
        in.push_back('2');
        in.push_back('3');
        in.push_back('4');

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opOutArrayByteSeqPtr callback = Test::newCallback_TestIntf_opOutArrayByteSeq(cb,
                                                                                                             &Callback::opOutArrayByteSeq, &Callback::noEx);

        t->begin_opOutArrayByteSeq(in, callback, newInParam(in));
        cb->check();
#endif
    }

    {
        Test::ByteSeq in;
        in.push_back('1');
        in.push_back('2');
        in.push_back('3');
        in.push_back('4');

#ifdef ICE_CPP11_MAPPING

        promise<bool> done;

        t->opOutRangeByteSeqAsync(in,
                                  [&](Test::ByteSeq out)
                                  {
                                      test(out == in);
                                      done.set_value(true);
                                  },
                                  [&](std::exception_ptr)
                                  {
                                      done.set_value(false);
                                  });

        test(done.get_future().get());
#else
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opOutRangeByteSeqPtr callback = Test::newCallback_TestIntf_opOutRangeByteSeq(cb,
                                                                                                             &Callback::opOutRangeByteSeq, &Callback::noEx);

        t->begin_opOutRangeByteSeq(in, callback, newInParam(in));
        cb->check();
#endif
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

#ifdef ICE_CPP11_MAPPING
            auto r = t->opIntStringDictAsync(idict).get();
            test(r.odict == idict);
            test(r.returnValue == idict);
#else
            Test::IntStringDict out;
            out[5] = "FIVE";

            Ice::AsyncResultPtr r = t->begin_opIntStringDict(idict);
            Test::IntStringDict ret = t->end_opIntStringDict(out, r);
            test(out == idict);
            test(ret == idict);
#endif
        }

        {
            Test::CustomMap<std::string, Ice::Int> idict;

            idict["ONE"] = 1;
            idict["TWO"] = 2;
            idict["THREE"] = 3;
            idict["MINUS ONE"] = -1;

#ifdef ICE_CPP11_MAPPING
            auto r = t->opVarDictAsync(idict).get();
            test(r.odict == idict);
            test(r.returnValue.size() == 1000);
            for(auto i: r.returnValue)
            {
                test(i.second == i.first * i.first);
            }
#else
            Test::CustomMap<std::string, Ice::Int> out;
            out["FIVE"] = 5;

            Ice::AsyncResultPtr r = t->begin_opVarDict(idict);
            Test::CustomMap<Ice::Long, Ice::Long> ret = t->end_opVarDict(out, r);

            test(out == idict);
            test(ret.size() == 1000);
            for(Test::CustomMap<Ice::Long, Ice::Long>::const_iterator i = ret.begin(); i != ret.end(); ++i)
            {
                test(i->second == i->first * i->first);
            }
#endif
        }

        {
            std::map<int, Util::string_view> idict;

            idict[1] = "ONE";
            idict[2] = "TWO";
            idict[3] = "THREE";
            idict[-1] = "MINUS ONE";

#ifdef ICE_CPP11_MAPPING
            auto r = t->opCustomIntStringDictAsync(idict).get();
            test(r.odict.size() == idict.size());

            test(r.odict == r.returnValue);

            for(auto i: idict)
            {
                test(r.odict[i.first] == i.second);
            }
#else
            Test::IntStringDict out;
            out[5] = "FIVE";

            Ice::AsyncResultPtr r = t->begin_opCustomIntStringDict(idict);
            Test::IntStringDict ret = t->end_opCustomIntStringDict(out, r);

            test(out.size() == idict.size());
            test(out == ret);
            for(std::map<int, Util::string_view>::const_iterator p = idict.begin();
                p != idict.end(); ++p)
            {
                test(out[p->first].size() == p->second.size());
                //  test(out[p->first] == p->second.to_string()); does not always work due to string converter
            }
#endif
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

#ifdef ICE_CPP11_MAPPING
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

#else
            CallbackPtr cb = new Callback();
            Test::Callback_TestIntf_opIntStringDictPtr callback =
                Test::newCallback_TestIntf_opIntStringDict(cb, &Callback::opIntStringDict, &Callback::noEx);
            t->begin_opIntStringDict(idict, callback, newInParam(idict));
            cb->check();
#endif
        }

        {
            Test::CustomMap<std::string, Ice::Int> idict;

            idict["ONE"] = 1;
            idict["TWO"] = 2;
            idict["THREE"] = 3;
            idict["MINUS ONE"] = -1;

#ifdef ICE_CPP11_MAPPING
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

#else

            CallbackPtr cb = new Callback();
            Test::Callback_TestIntf_opVarDictPtr callback =
                Test::newCallback_TestIntf_opVarDict(cb, &Callback::opVarDict, &Callback::noEx);
            t->begin_opVarDict(idict, callback, newInParam(idict));
            cb->check();
#endif
        }

        {
            std::map<int, Util::string_view> idict;

            idict[1] = "ONE";
            idict[2] = "TWO";
            idict[3] = "THREE";
            idict[-1] = "MINUS ONE";

#ifdef ICE_CPP11_MAPPING
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
#else
            CallbackPtr cb = new Callback();
            Test::Callback_TestIntf_opCustomIntStringDictPtr callback =
                Test::newCallback_TestIntf_opCustomIntStringDict(cb, &Callback::opCustomIntStringDict, &Callback::noEx);
            t->begin_opCustomIntStringDict(idict, callback, newInParam(idict));
            cb->check();
#endif
        }

    }
    cout << "ok" << endl;

#ifndef ICE_CPP11_MAPPING

    cout << "testing class mapped structs ... " << flush;
    Test::ClassStructPtr cs = new Test::ClassStruct();
    cs->y = 10;
    cs->other = new Test::ClassOtherStruct;
    cs->other->x = 20;
    cs->otherSeq.push_back(new Test::ClassOtherStruct);
    cs->otherSeq[0]->x = 30;
    cs->otherSeq.push_back(new Test::ClassOtherStruct);
    cs->otherSeq[1]->x = 40;
    Test::ClassStructSeq csseq1;
    csseq1.push_back(cs);
    Test::ClassStructPtr cs2;
    Test::ClassStructSeq csseq2;
    Test::ClassStructPtr cs3 = t->opClassStruct(cs, csseq1, cs2, csseq2);
    assert(cs3 == cs);
    assert(csseq1.size() == csseq2.size());
    assert(csseq1[0] == csseq2[0]);
    cout << "ok" << endl;

    cout << "testing class mapped structs with AMI... " << flush;
    {
        Test::ClassStructPtr cs2;
        Test::ClassStructSeq csseq2;
        Ice::AsyncResultPtr r = t->begin_opClassStruct(cs, csseq1);
        Test::ClassStructPtr cs3 = t->end_opClassStruct(cs2, csseq2, r);
        assert(cs3 == cs);
        assert(csseq1.size() == csseq2.size());
        assert(csseq1[0] == csseq2[0]);
    }
    {
        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opClassStructPtr callback =
            Test::newCallback_TestIntf_opClassStruct(cb, &Callback::opClassStruct, &Callback::noEx);
        t->begin_opClassStruct(cs, csseq1, callback, newInParam(make_pair(cs, csseq1)));
        cb->check();
    }
    cout << "ok" << endl;

#endif

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
#ifdef ICE_CPP11_MAPPING
        auto r = wsc1->opStringAsync(wstr).get();
        test(r.s2 == wstr);
        test(r.returnValue == wstr);
#else
        Ice::AsyncResultPtr r = wsc1->begin_opString(wstr);
        wstring out;
        wstring ret = wsc1->end_opString(out, r);
        test(out == wstr);
        test(ret == wstr);
#endif
    }
    {
#ifdef ICE_CPP11_MAPPING

        promise<bool> done;

        wsc1->opStringAsync(wstr,
                            [&](wstring ret, wstring out)
                            {
                                test(out == wstr);
                                test(ret == wstr);
                                done.set_value(true);
                            },
                            [&](std::exception_ptr)
                            {
                                done.set_value(false);
                            });

        test(done.get_future().get());
#else
        CallbackPtr cb = new Callback();
        wsc1->begin_opString(wstr, Test1::newCallback_WstringClass_opString(cb, &Callback::opString, &Callback::noEx),
                             newInParam(wstr));
        cb->check();
#endif
    }

    ret = wsc2->opString(wstr, out);
    test(out == wstr);
    test(ret == wstr);

    {
#ifdef ICE_CPP11_MAPPING
        auto r = wsc2->opStringAsync(wstr).get();
        test(r.s2 == wstr);
        test(r.returnValue == wstr);
#else
        Ice::AsyncResultPtr r = wsc2->begin_opString(wstr);
        wstring out;
        wstring ret = wsc2->end_opString(out, r);
        test(out == wstr);
        test(ret == wstr);
#endif
    }
    {
#ifdef ICE_CPP11_MAPPING

        promise<bool> done;

        wsc2->opStringAsync(wstr,
                            [&](wstring ret, wstring out)
                            {
                                test(out == wstr);
                                test(ret == wstr);
                                done.set_value(true);
                            },
                            [&](std::exception_ptr)
                            {
                                done.set_value(false);
                            });

        test(done.get_future().get());
#else
        CallbackPtr cb = new Callback();
        wsc2->begin_opString(wstr, Test2::newCallback_WstringClass_opString(cb, &Callback::opString,
                                                                            &Callback::noEx), newInParam(wstr));
        cb->check();
#endif
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
#ifdef ICE_CPP11_MAPPING

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
#else
        Ice::AsyncResultPtr r = wsc1->begin_throwExcept(wstr);
        try
        {
            wsc1->end_throwExcept(r);
            test(false);
        }
        catch(const Test1::WstringException& ex)
        {
            test(ex.reason == wstr);
        }
#endif
    }
    {

#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        wsc1->begin_throwExcept(wstr, Ice::newCallback(cb, &Callback::throwExcept1), newInParam(wstr));
        cb->check();
#endif
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

#ifdef ICE_CPP11_MAPPING

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
#else
        Ice::AsyncResultPtr r = wsc2->begin_throwExcept(wstr);
        try
        {
            wsc2->end_throwExcept(r);
            test(false);
        }
        catch(const Test2::WstringException& ex)
        {
            test(ex.reason == wstr);
        }
#endif
    }
    {
#ifdef ICE_CPP11_MAPPING

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
#else
        CallbackPtr cb = new Callback();
        wsc2->begin_throwExcept(wstr, Ice::newCallback(cb, &Callback::throwExcept2), newInParam(wstr));
        cb->check();
#endif
    }

    cout << "ok" << endl;

    return t;
}
