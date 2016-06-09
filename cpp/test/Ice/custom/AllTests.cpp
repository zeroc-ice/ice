// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

#ifdef ICE_CPP11_MAPPING
//
// Slice class C is mapped to ObjectPrx
//
namespace Test
{
typedef shared_ptr<Ice::ObjectPrx> CPrxPtr;
typedef Ice::ObjectPrx CPrx;

bool operator==(const Variable& lhs, const Variable& rhs)
{
    return std::tie(lhs.s, lhs.bl, lhs.ss) == std::tie(rhs.s, rhs.bl, rhs.ss);
}

bool operator!=(const Variable& lhs, const Variable& rhs)
{
    return !(lhs == rhs);
}

bool operator==(const BufferStruct& lhs, const BufferStruct& rhs)
{
    return std::tie(lhs.byteBuf, lhs.boolBuf, lhs.shortBuf, lhs.intBuf, lhs.longBuf, lhs.floatBuf, lhs.doubleBuf)
        == std::tie(rhs.byteBuf, lhs.boolBuf, rhs.shortBuf, rhs.intBuf, rhs.longBuf, rhs.floatBuf, rhs.doubleBuf);
}

bool operator!=(const BufferStruct& lhs, const BufferStruct& rhs)
{
    return !(lhs == rhs);
}
}
#endif


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

    void opCPrxSeq(const deque<Test::CPrxPtr>& ret, const deque<Test::CPrxPtr>& out, const InParamPtr& cookie)
    {
        const deque<Test::CPrxPtr>& in = getIn<deque<Test::CPrxPtr> >(cookie);
        test(out == in);
        test(ret == in);
        called();
    }

    void opCPrxList(const list<Test::CPrxPtr>& ret, const list<Test::CPrxPtr>& out, const InParamPtr& cookie)
    {
        const list<Test::CPrxPtr>& in = getIn<list<Test::CPrxPtr> >(cookie);
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

#ifndef ICE_CPP11_MAPPING

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
#endif

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

#if defined(_MSC_VER) && (_MSC_VER == 1600)
        //
        // operator== for std::unordered_map does not work with Visual Studio 2010
        //
        test(out.size() == in.size());

        for(Test::CustomMap<std::string, Ice::Int>::const_iterator p = in.begin(); p != in.end(); ++p)
        {
            Test::CustomMap<std::string, Ice::Int>::const_iterator q = out.find(p->first);
            test(q != out.end() && q->second == p->second);
        }
#else
        test(out == in);
#endif
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

#ifndef ICE_CPP11_MAPPING
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
#endif

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

#ifndef ICE_CPP11_MAPPING
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
#endif

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
    test(Ice::targetEquals(t, base));
#else
    test(t == base);
#endif
    cout << "ok" << endl;

    cout << "testing alternate strings... " << flush;
    {
        Util::string_view in = "Hello World!";
        string out;
        string ret = t->opString(in, out);

        //
        // TODO: improve test
        // When a parameter is mapped as a string_view on both sides,
        // no conversion occurs, and all is good
        // However, when a parameter is mapped as a string on one side and
        // as a string_view on the other (e.g. sync invocation implemented with AMD)
        // the string converter converts the string but not the string_view,
        // and we have a mismatch
        //
        test(ret == out);
        test(ret.size() == in.size());
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
        deque<Test::CPrxPtr> in(5);
        in[0] = ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C1:" + endp + " -t 10000"));
        in[1] = ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C2:" + endp + " -t 10001"));
        in[2] = ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C3:" + endp + " -t 10002"));
        in[3] = ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C4:" + endp + " -t 10003"));
        in[4] = ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C5:" + endp + " -t 10004"));

        deque<Test::CPrxPtr> out;
        deque<Test::CPrxPtr> ret = t->opCPrxSeq(in, out);

#ifdef ICE_CPP11_MAPPING
        auto op = out.begin();
        auto rp = ret.begin();

        for(auto i: in)
        {
            test(Ice::targetEquals(*op++, i));
            test(Ice::targetEquals(*rp++, i));
        }
#else
        test(out == in);
        test(ret == in);
#endif
    }

    {
        list<Test::CPrxPtr> in;
        in.push_back(ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C1:" + endp + " -t 10000")));
        in.push_back(ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C2:" + endp + " -t 10001")));
        in.push_back(ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C3:" + endp + " -t 10002")));
        in.push_back(ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C4:" + endp + " -t 10003")));
        in.push_back(ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C5:" + endp + " -t 10004")));

        list<Test::CPrxPtr> out;
        list<Test::CPrxPtr> ret = t->opCPrxList(in, out);
#ifdef ICE_CPP11_MAPPING
        auto op = out.begin();
        auto rp = ret.begin();

        for(auto i: in)
        {
            test(Ice::targetEquals(*op++, i));
            test(Ice::targetEquals(*rp++, i));
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

        idict[1] = "one";
        idict[2] = "two";
        idict[3] = "three";
        idict[-1] = "minus one";

        Test::IntStringDict out;
        out[5] = "five";

        Test::IntStringDict ret = t->opIntStringDict(idict, out);
        test(out == idict);
        test(ret == idict);
    }

    {
        Test::CustomMap<std::string, Ice::Int> idict;

        idict["one"] = 1;
        idict["two"] = 2;
        idict["three"] = 3;
        idict["minus one"] = -1;

        Test::CustomMap<std::string, Ice::Int> out;
        out["five"] = 5;

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

        idict[1] = "one";
        idict[2] = "two";
        idict[3] = "three";
        idict[-1] = "minus one";

        Test::IntStringDict out;
        out[5] = "five";

        Test::IntStringDict ret = t->opCustomIntStringDict(idict, out);
        test(out.size() == idict.size());
        test(out == ret);
        for(std::map<int, Util::string_view>::const_iterator p = idict.begin();
            p != idict.end(); ++p)
        {
            test(out[p->first].size() == p->second.size());
            //  test(out[p->first] == p->second.to_string()); does not always work due to string converter
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

#ifndef ICE_CPP11_MAPPING

    cout << "testing alternate strings with AMI... " << flush;
    {
        Util::string_view in = "Hello World!";

        Ice::AsyncResultPtr r = t->begin_opString(in);
        string out;
        string ret = t->end_opString(out, r);

        test(ret == out);
        test(ret.size() == in.size());
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

            Test::DoubleSeq out;
            Ice::AsyncResultPtr r = t->begin_opDoubleArray(inPair);
            Test::DoubleSeq ret = t->end_opDoubleArray(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opBoolArray(inPair);
            Test::BoolSeq ret = t->end_opBoolArray(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opByteArray(inPair);
            Test::ByteList ret = t->end_opByteArray(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opVariableArray(inPair);
            Test::VariableList ret = t->end_opVariableArray(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opBoolRange(inPair);
            Test::BoolSeq ret = t->end_opBoolRange(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opByteRange(inPair);
            Test::ByteList ret = t->end_opByteRange(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opVariableRange(inPair);
            Test::VariableList ret = t->end_opVariableRange(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opByteRangeType(inPair);
            Test::ByteList ret = t->end_opByteRangeType(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opVariableRangeType(inPair);
            Test::VariableList ret = t->end_opVariableRangeType(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opBoolSeq(in);
            deque<bool> ret = t->end_opBoolSeq(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opBoolList(in);
            list<bool> ret = t->end_opBoolList(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opByteSeq(in);
            deque< ::Ice::Byte> ret = t->end_opByteSeq(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opByteList(in);
            list< ::Ice::Byte> ret = t->end_opByteList(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opMyByteSeq(in);
            MyByteSeq ret = t->end_opMyByteSeq(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opStringSeq(in);
            deque<string> ret = t->end_opStringSeq(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opStringList(in);
            list<string> ret = t->end_opStringList(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opFixedSeq(in);
            deque<Test::Fixed> ret = t->end_opFixedSeq(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opFixedList(in);
            list<Test::Fixed> ret = t->end_opFixedList(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opVariableSeq(in);
            deque<Test::Variable> ret = t->end_opVariableSeq(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opVariableList(in);
            list<Test::Variable> ret = t->end_opVariableList(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opStringStringDictSeq(in);
            deque<Test::StringStringDict> ret = t->end_opStringStringDictSeq(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opStringStringDictList(in);
            list<Test::StringStringDict> ret = t->end_opStringStringDictList(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opESeq(in);
            deque<Test::E> ret = t->end_opESeq(out, r);
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
            Ice::AsyncResultPtr r = t->begin_opEList(in);
            list<Test::E> ret = t->end_opEList(out, r);
            test(out == in);
            test(ret == in);
        }

        {
            deque<Test::CPrx> in(5);
            in[0] = ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C1:" + endp + " -t 10000"));
            in[1] = ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C2:" + endp + " -t 10001"));
            in[2] = ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C3:" + endp + " -t 10002"));
            in[3] = ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C4:" + endp + " -t 10003"));
            in[4] = ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C5:" + endp + " -t 10004"));

            deque<Test::CPrx> out;
            Ice::AsyncResultPtr r = t->begin_opCPrxSeq(in);
            deque<Test::CPrx> ret = t->end_opCPrxSeq(out, r);
            test(out == in);
            test(ret == in);
        }

        {
            list<Test::CPrx> in;
            in.push_back(ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C1:" + endp + " -t 10000")));
            in.push_back(ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C2:" + endp + " -t 10001")));
            in.push_back(ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C3:" + endp + " -t 10002")));
            in.push_back(ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C4:" + endp + " -t 10003")));
            in.push_back(ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C5:" + endp + " -t 10004")));

            list<Test::CPrx> out;
            Ice::AsyncResultPtr r = t->begin_opCPrxList(in);
            list<Test::CPrx> ret = t->end_opCPrxList(out, r);
            test(out == in);
            test(ret == in);
        }

        {
            deque<Test::CPtr> in(5);
            in[0] = ICE_MAKE_SHARED(Test::C);
            in[1] = in[0];
            in[2] = in[0];
            in[3] = in[0];
            in[4] = in[0];

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
        }

        {
            list<Test::CPtr> in;
            in.push_back(ICE_MAKE_SHARED(Test::C));
            in.push_back(ICE_MAKE_SHARED(Test::C));
            in.push_back(ICE_MAKE_SHARED(Test::C));
            in.push_back(ICE_MAKE_SHARED(Test::C));
            in.push_back(ICE_MAKE_SHARED(Test::C));

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
        }


        {
            Test::ByteSeq in;
            in.push_back('1');
            in.push_back('2');
            in.push_back('3');
            in.push_back('4');

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
        }

        {
            Test::ByteSeq in;
            in.push_back('1');
            in.push_back('2');
            in.push_back('3');
            in.push_back('4');

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
        }
    }
    cout << "ok" << endl;

    cout << "testing alternate strings with new AMI callbacks... " << flush;
    {
        Util::string_view in = "Hello World!";

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opStringPtr callback =
            Test::newCallback_TestIntf_opString(cb, &Callback::opString, &Callback::noEx);
        t->begin_opString(in, callback, newInParam(in));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "testing alternate sequences with new AMI callbacks... " << flush;

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

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opDoubleArrayPtr callback =
            Test::newCallback_TestIntf_opDoubleArray(cb, &Callback::opDoubleArray, &Callback::noEx);
        t->begin_opDoubleArray(inPair, callback, newInParam(inPair));
        cb->check();
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

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opBoolArrayPtr callback =
            Test::newCallback_TestIntf_opBoolArray(cb, &Callback::opBoolArray, &Callback::noEx);
        t->begin_opBoolArray(inPair, callback, newInParam(inPair));
        cb->check();
    }

    {
        Ice::Byte in[5];
        in[0] = '1';
        in[1] = '2';
        in[2] = '3';
        in[3] = '4';
        in[4] = '5';
        pair<const Ice::Byte*, const Ice::Byte*> inPair(in, in + 5);

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opByteArrayPtr callback =
            Test::newCallback_TestIntf_opByteArray(cb, &Callback::opByteArray, &Callback::noEx);
        t->begin_opByteArray(inPair, callback, newInParam(inPair));
        cb->check();
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

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opVariableArrayPtr callback =
            Test::newCallback_TestIntf_opVariableArray(cb, &Callback::opVariableArray, &Callback::noEx);
        t->begin_opVariableArray(inPair, callback, newInParam(inPair));
        cb->check();
    }

    {
        Test::BoolSeq in(5);
        in[0] = false;
        in[1] = true;
        in[2] = true;
        in[3] = false;
        in[4] = true;
        pair<Test::BoolSeq::const_iterator, Test::BoolSeq::const_iterator> inPair(in.begin(), in.end());

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opBoolRangePtr callback =
            Test::newCallback_TestIntf_opBoolRange(cb, &Callback::opBoolRange, &Callback::noEx);
        t->begin_opBoolRange(inPair, callback, newInParam(inPair));
        cb->check();
    }

    {
        Test::ByteList in;
        in.push_back('1');
        in.push_back('2');
        in.push_back('3');
        in.push_back('4');
        in.push_back('5');
        pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator> inPair(in.begin(), in.end());

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opByteRangePtr callback =
            Test::newCallback_TestIntf_opByteRange(cb, &Callback::opByteRange, &Callback::noEx);
        t->begin_opByteRange(inPair, callback, newInParam(inPair));
        cb->check();
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

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opVariableRangePtr callback =
            Test::newCallback_TestIntf_opVariableRange(cb, &Callback::opVariableRange, &Callback::noEx);
        t->begin_opVariableRange(inPair, callback, newInParam(inPair));
        cb->check();
    }

    {
        Test::ByteList in;
        in.push_back('1');
        in.push_back('2');
        in.push_back('3');
        in.push_back('4');
        in.push_back('5');
        pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator> inPair(in.begin(), in.end());

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opByteRangeTypePtr callback =
            Test::newCallback_TestIntf_opByteRangeType(cb, &Callback::opByteRangeType, &Callback::noEx);
        t->begin_opByteRangeType(inPair, callback, newInParam(inPair));
        cb->check();
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
        pair<deque<Test::Variable>::const_iterator, deque<Test::Variable>::const_iterator> inPair(inSeq.begin(),
                                                                                                  inSeq.end());

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opVariableRangeTypePtr callback =
            Test::newCallback_TestIntf_opVariableRangeType(cb, &Callback::opVariableRangeType, &Callback::noEx);
        t->begin_opVariableRangeType(inPair, callback, newInParam(inPair));
        cb->check();
    }

    {
        deque<bool> in(5);
        in[0] = false;
        in[1] = true;
        in[2] = true;
        in[3] = false;
        in[4] = true;

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opBoolSeqPtr callback =
            Test::newCallback_TestIntf_opBoolSeq(cb, &Callback::opBoolSeq, &Callback::noEx);
        t->begin_opBoolSeq(in, callback, newInParam(in));
        cb->check();
    }

    {
        list<bool> in;
        in.push_back(false);
        in.push_back(true);
        in.push_back(true);
        in.push_back(false);
        in.push_back(true);

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opBoolListPtr callback =
            Test::newCallback_TestIntf_opBoolList(cb, &Callback::opBoolList, &Callback::noEx);
        t->begin_opBoolList(in, callback, newInParam(in));
        cb->check();
    }

    {
        deque< ::Ice::Byte> in(5);
        in[0] = '1';
        in[1] = '2';
        in[2] = '3';
        in[3] = '4';
        in[4] = '5';

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opByteSeqPtr callback =
            Test::newCallback_TestIntf_opByteSeq(cb, &Callback::opByteSeq, &Callback::noEx);
        t->begin_opByteSeq(in, callback, newInParam(in));
        cb->check();
    }

    {
        list< ::Ice::Byte> in;
        in.push_back('1');
        in.push_back('2');
        in.push_back('3');
        in.push_back('4');
        in.push_back('5');

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opByteListPtr callback =
            Test::newCallback_TestIntf_opByteList(cb, &Callback::opByteList, &Callback::noEx);
        t->begin_opByteList(in, callback, newInParam(in));
        cb->check();
    }

    {
        MyByteSeq in(5);
        int i = 0;
        for(MyByteSeq::iterator p = in.begin(); p != in.end(); ++p)
        {
            *p = '1' + i++;
        }

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opMyByteSeqPtr callback =
            Test::newCallback_TestIntf_opMyByteSeq(cb, &Callback::opMyByteSeq, &Callback::noEx);
        t->begin_opMyByteSeq(in, callback, newInParam(in));
        cb->check();
    }

    {
        deque<string> in(5);
        in[0] = "These";
        in[1] = "are";
        in[2] = "five";
        in[3] = "short";
        in[4] = "strings.";

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opStringSeqPtr callback =
            Test::newCallback_TestIntf_opStringSeq(cb, &Callback::opStringSeq, &Callback::noEx);
        t->begin_opStringSeq(in, callback, newInParam(in));
        cb->check();
    }

    {
        list<string> in;
        in.push_back("These");
        in.push_back("are");
        in.push_back("five");
        in.push_back("short");
        in.push_back("strings.");

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opStringListPtr callback =
            Test::newCallback_TestIntf_opStringList(cb, &Callback::opStringList, &Callback::noEx);
        t->begin_opStringList(in, callback, newInParam(in));
        cb->check();
    }

    {
        deque<Test::Fixed> in(5);
        in[0].s = 1;
        in[1].s = 2;
        in[2].s = 3;
        in[3].s = 4;
        in[4].s = 5;

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opFixedSeqPtr callback =
            Test::newCallback_TestIntf_opFixedSeq(cb, &Callback::opFixedSeq, &Callback::noEx);
        t->begin_opFixedSeq(in, callback, newInParam(in));
        cb->check();
    }

    {
        list<Test::Fixed> in(5);
        short num = 1;
        for(list<Test::Fixed>::iterator p = in.begin(); p != in.end(); ++p)
        {
            (*p).s = num++;
        }

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opFixedListPtr callback =
            Test::newCallback_TestIntf_opFixedList(cb, &Callback::opFixedList, &Callback::noEx);
        t->begin_opFixedList(in, callback, newInParam(in));
        cb->check();
    }

    {
        deque<Test::Variable> in(5);
        in[0].s = "These";
        in[1].s = "are";
        in[2].s = "five";
        in[3].s = "short";
        in[4].s = "strings.";

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opVariableSeqPtr callback =
            Test::newCallback_TestIntf_opVariableSeq(cb, &Callback::opVariableSeq, &Callback::noEx);
        t->begin_opVariableSeq(in, callback, newInParam(in));
        cb->check();
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

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opVariableListPtr callback =
            Test::newCallback_TestIntf_opVariableList(cb, &Callback::opVariableList, &Callback::noEx);
        t->begin_opVariableList(in, callback, newInParam(in));
        cb->check();
    }

    {
        deque<Test::StringStringDict> in(5);
        in[0]["A"] = "a";
        in[1]["B"] = "b";
        in[2]["C"] = "c";
        in[3]["D"] = "d";
        in[4]["E"] = "e";

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opStringStringDictSeqPtr callback =
            Test::newCallback_TestIntf_opStringStringDictSeq(cb, &Callback::opStringStringDictSeq, &Callback::noEx);
        t->begin_opStringStringDictSeq(in, callback, newInParam(in));
        cb->check();
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

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opStringStringDictListPtr callback =
            Test::newCallback_TestIntf_opStringStringDictList(cb, &Callback::opStringStringDictList, &Callback::noEx);
        t->begin_opStringStringDictList(in, callback, newInParam(in));
        cb->check();
    }

    {
        deque<Test::E> in(5);
        in[0] = Test:: ICE_ENUM(E, E1);
        in[1] = Test:: ICE_ENUM(E, E2);
        in[2] = Test:: ICE_ENUM(E, E3);
        in[3] = Test:: ICE_ENUM(E, E1);
        in[4] = Test:: ICE_ENUM(E, E3);

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opESeqPtr callback =
            Test::newCallback_TestIntf_opESeq(cb, &Callback::opESeq, &Callback::noEx);
        t->begin_opESeq(in, callback, newInParam(in));
        cb->check();
    }

    {
        list<Test::E> in;
        in.push_back(Test:: ICE_ENUM(E, E1));
        in.push_back(Test:: ICE_ENUM(E, E2));
        in.push_back(Test:: ICE_ENUM(E, E3));
        in.push_back(Test:: ICE_ENUM(E, E1));
        in.push_back(Test:: ICE_ENUM(E, E3));

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opEListPtr callback =
            Test::newCallback_TestIntf_opEList(cb, &Callback::opEList, &Callback::noEx);
        t->begin_opEList(in, callback, newInParam(in));
        cb->check();
    }

    {
        deque<Test::CPrx> in(5);
        in[0] = ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C1:" + endp + " -t 10000"));
        in[1] = ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C2:" + endp + " -t 10001"));
        in[2] = ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C3:" + endp + " -t 10002"));
        in[3] = ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C4:" + endp + " -t 10003"));
        in[4] = ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C5:" + endp + " -t 10004"));

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opCPrxSeqPtr callback =
            Test::newCallback_TestIntf_opCPrxSeq(cb, &Callback::opCPrxSeq, &Callback::noEx);
        t->begin_opCPrxSeq(in, callback, newInParam(in));
        cb->check();
    }

    {
        list<Test::CPrx> in;
        in.push_back(ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C1:" + endp + " -t 10000")));
        in.push_back(ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C2:" + endp + " -t 10001")));
        in.push_back(ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C3:" + endp + " -t 10002")));
        in.push_back(ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C4:" + endp + " -t 10003")));
        in.push_back(ICE_UNCHECKED_CAST(Test::CPrx, communicator->stringToProxy("C5:" + endp + " -t 10004")));

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opCPrxListPtr callback =
            Test::newCallback_TestIntf_opCPrxList(cb, &Callback::opCPrxList, &Callback::noEx);
        t->begin_opCPrxList(in, callback, newInParam(in));
        cb->check();
    }

    {
        deque<Test::CPtr> in(5);
        in[0] = ICE_MAKE_SHARED(Test::C);
        in[1] = in[0];
        in[2] = in[0];
        in[3] = in[0];
        in[4] = in[0];

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opCSeqPtr callback = Test::newCallback_TestIntf_opCSeq(cb, &Callback::opCSeq, &Callback::noEx);
        t->begin_opCSeq(in, callback, newInParam(in));
        cb->check();
    }

    {
        list<Test::CPtr> in;
        in.push_back(ICE_MAKE_SHARED(Test::C));
        in.push_back(ICE_MAKE_SHARED(Test::C));
        in.push_back(ICE_MAKE_SHARED(Test::C));
        in.push_back(ICE_MAKE_SHARED(Test::C));
        in.push_back(ICE_MAKE_SHARED(Test::C));

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opCListPtr callback =
            Test::newCallback_TestIntf_opCList(cb, &Callback::opCList, &Callback::noEx);
        t->begin_opCList(in, callback, newInParam(in));
        cb->check();
    }


    {
        Test::ByteSeq in;
        in.push_back('1');
        in.push_back('2');
        in.push_back('3');
        in.push_back('4');

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opOutArrayByteSeqPtr callback = Test::newCallback_TestIntf_opOutArrayByteSeq(cb,
                                                                                                             &Callback::opOutArrayByteSeq, &Callback::noEx);

        t->begin_opOutArrayByteSeq(in, callback, newInParam(in));
        cb->check();
    }

    {
        Test::ByteSeq in;
        in.push_back('1');
        in.push_back('2');
        in.push_back('3');
        in.push_back('4');

        CallbackPtr cb = new Callback();
        Test::Callback_TestIntf_opOutRangeByteSeqPtr callback = Test::newCallback_TestIntf_opOutRangeByteSeq(cb,
                                                                                                             &Callback::opOutRangeByteSeq, &Callback::noEx);

        t->begin_opOutRangeByteSeq(in, callback, newInParam(in));
        cb->check();
    }

    cout << "ok" << endl;

    cout << "testing alternate dictionaries with new AMI... " << flush;
    {
        {
            Test::IntStringDict idict;

            idict[1] = "one";
            idict[2] = "two";
            idict[3] = "three";
            idict[-1] = "minus one";

            Test::IntStringDict out;
            out[5] = "five";

            Ice::AsyncResultPtr r = t->begin_opIntStringDict(idict);
            Test::IntStringDict ret = t->end_opIntStringDict(out, r);
            test(out == idict);
            test(ret == idict);
        }

        {
            Test::CustomMap<std::string, Ice::Int> idict;

            idict["one"] = 1;
            idict["two"] = 2;
            idict["three"] = 3;
            idict["minus one"] = -1;

            Test::CustomMap<std::string, Ice::Int> out;
            out["five"] = 5;

            Ice::AsyncResultPtr r = t->begin_opVarDict(idict);
            Test::CustomMap<Ice::Long, Ice::Long> ret = t->end_opVarDict(out, r);

            test(out == idict);
            test(ret.size() == 1000);
            for(Test::CustomMap<Ice::Long, Ice::Long>::const_iterator i = ret.begin(); i != ret.end(); ++i)
            {
                test(i->second == i->first * i->first);
            }
        }

        {
            std::map<int, Util::string_view> idict;

            idict[1] = "one";
            idict[2] = "two";
            idict[3] = "three";
            idict[-1] = "minus one";

            Test::IntStringDict out;
            out[5] = "five";

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
        }
    }
    cout << "ok" << endl;

    cout << "testing alternate dictionaries with new AMI callbacks... " << flush;
    {
        {
            Test::IntStringDict idict;

            idict[1] = "one";
            idict[2] = "two";
            idict[3] = "three";
            idict[-1] = "minus one";

            CallbackPtr cb = new Callback();
            Test::Callback_TestIntf_opIntStringDictPtr callback =
                Test::newCallback_TestIntf_opIntStringDict(cb, &Callback::opIntStringDict, &Callback::noEx);
            t->begin_opIntStringDict(idict, callback, newInParam(idict));
            cb->check();
        }

        {
            Test::CustomMap<std::string, Ice::Int> idict;

            idict["one"] = 1;
            idict["two"] = 2;
            idict["three"] = 3;
            idict["minus one"] = -1;

            CallbackPtr cb = new Callback();
            Test::Callback_TestIntf_opVarDictPtr callback =
                Test::newCallback_TestIntf_opVarDict(cb, &Callback::opVarDict, &Callback::noEx);
            t->begin_opVarDict(idict, callback, newInParam(idict));
            cb->check();
        }

        {
            std::map<int, Util::string_view> idict;

            idict[1] = "one";
            idict[2] = "two";
            idict[3] = "three";
            idict[-1] = "minus one";

            CallbackPtr cb = new Callback();
            Test::Callback_TestIntf_opCustomIntStringDictPtr callback =
                Test::newCallback_TestIntf_opCustomIntStringDict(cb, &Callback::opCustomIntStringDict, &Callback::noEx);
            t->begin_opCustomIntStringDict(idict, callback, newInParam(idict));
            cb->check();
        }

    }
    cout << "ok" << endl;

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

    cout << "testing wstring... " << flush;

    Test1::WstringSeq wseq1;
    wseq1.push_back(L"Wide String");

    Test2::WstringSeq wseq2;
    wseq2 = wseq1;

    Test1::WstringWStringDict wdict1;
    wdict1[L"Key"] = L"Value";

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

    wstring wstr = L"A Wide String";
    wstring out;
    wstring ret = wsc1->opString(wstr, out);
    test(out == wstr);
    test(ret == wstr);

    {
        Ice::AsyncResultPtr r = wsc1->begin_opString(wstr);
        wstring out;
        wstring ret = wsc1->end_opString(out, r);
        test(out == wstr);
        test(ret == wstr);
    }
    {
        CallbackPtr cb = new Callback();
        wsc1->begin_opString(wstr, Test1::newCallback_WstringClass_opString(cb, &Callback::opString, &Callback::noEx),
                             newInParam(wstr));
        cb->check();
    }

    ret = wsc2->opString(wstr, out);
    test(out == wstr);
    test(ret == wstr);

    {
        Ice::AsyncResultPtr r = wsc2->begin_opString(wstr);
        wstring out;
        wstring ret = wsc2->end_opString(out, r);
        test(out == wstr);
        test(ret == wstr);
    }
    {
        CallbackPtr cb = new Callback();
        wsc2->begin_opString(wstr, Test2::newCallback_WstringClass_opString(cb, &Callback::opString,
                                                                            &Callback::noEx), newInParam(wstr));
        cb->check();
    }
    {
        CallbackPtr cb = new Callback();
        wsc2->begin_opString(wstr, Test2::newCallback_WstringClass_opString(cb, &Callback::opString,
                                                                            &Callback::noEx), newInParam(wstr));
        cb->check();
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
    }
    {
        CallbackPtr cb = new Callback();
        wsc1->begin_throwExcept(wstr, Ice::newCallback(cb, &Callback::throwExcept1), newInParam(wstr));
        cb->check();
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
    }
    {
        CallbackPtr cb = new Callback();
        wsc2->begin_throwExcept(wstr, Ice::newCallback(cb, &Callback::throwExcept2), newInParam(wstr));
        cb->check();
    }
#endif

    cout << "ok" << endl;

    return t;
}
