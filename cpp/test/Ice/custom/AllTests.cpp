// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

using namespace std;

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

    bool check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        while(!_called)
        {
            if(!timedWait(IceUtil::Time::seconds(5)))
            {
                return false;
            }
        }
        _called = false;
        return true;
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

class AMI_TestIntf_opBoolArrayI : public Test::AMI_TestIntf_opBoolArray, public CallbackBase
{
public:

    AMI_TestIntf_opBoolArrayI(Test::BoolSeq in)
        : _in(in)
    {
    }

    virtual void ice_response(const Test::BoolSeq& ret, const Test::BoolSeq& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    Test::BoolSeq _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opBoolArrayI> AMI_TestIntf_opBoolArrayIPtr;

class AMI_TestIntf_opByteArrayI : public Test::AMI_TestIntf_opByteArray, public CallbackBase
{
public:

    AMI_TestIntf_opByteArrayI(const pair<const Ice::Byte*, const Ice::Byte*>& in)
        : _in(in)
    {
    }

    virtual void ice_response(const pair<const Ice::Byte*, const Ice::Byte*>& ret,
                              const pair<const Ice::Byte*, const Ice::Byte*>& out)
    {
        test(_in.second - _in.first == out.second - out.first);
        test(_in.second - _in.first == ret.second - ret.first);
        Ice::Byte* b = const_cast<Ice::Byte*>(_in.first);
        Ice::Byte* r = const_cast<Ice::Byte*>(ret.first);
        Ice::Byte* o = const_cast<Ice::Byte*>(out.first);
        while(b != _in.second)
        {
            test(*r++ == *b);
            test(*o++ == *b++);
        }
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    pair<const Ice::Byte*, const Ice::Byte*> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opByteArrayI> AMI_TestIntf_opByteArrayIPtr;

class AMI_TestIntf_opVariableArrayI : public Test::AMI_TestIntf_opVariableArray, public CallbackBase
{
public:

    AMI_TestIntf_opVariableArrayI(Test::VariableList in)
        : _in(in)
    {
    }

    virtual void ice_response(const Test::VariableList& ret, const Test::VariableList& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    Test::VariableList _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opVariableArrayI> AMI_TestIntf_opVariableArrayIPtr;

class AMI_TestIntf_opBoolRangeI : public Test::AMI_TestIntf_opBoolRange, public CallbackBase
{
public:

    AMI_TestIntf_opBoolRangeI(Test::BoolSeq in)
        : _in(in)
    {
    }

    virtual void ice_response(const Test::BoolSeq& ret, const Test::BoolSeq& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    Test::BoolSeq _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opBoolRangeI> AMI_TestIntf_opBoolRangeIPtr;

class AMI_TestIntf_opByteRangeI : public Test::AMI_TestIntf_opByteRange, public CallbackBase
{
public:

    AMI_TestIntf_opByteRangeI(Test::ByteList in)
        : _in(in)
    {
    }

    virtual void ice_response(const pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>& ret,
                              const pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator>& out)
    {
        test(ice_distance(out.first, out.second) == static_cast<Ice::Int>(_in.size()));
        test(ice_distance(ret.first, ret.second) == static_cast<Ice::Int>(_in.size()));
        Test::ByteList::const_iterator b;
        Test::ByteList::const_iterator o = out.first;
        Test::ByteList::const_iterator r = ret.first;
        for(b = _in.begin(); b != _in.end(); ++b)
        {
            test(*b == *o++);
            test(*b == *r++);
        }
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    Test::ByteList _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opByteRangeI> AMI_TestIntf_opByteRangeIPtr;

class AMI_TestIntf_opVariableRangeI : public Test::AMI_TestIntf_opVariableRange, public CallbackBase
{
public:

    AMI_TestIntf_opVariableRangeI(Test::VariableList in)
        : _in(in)
    {
    }

    virtual void ice_response(const Test::VariableList& ret, const Test::VariableList& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    Test::VariableList _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opVariableRangeI> AMI_TestIntf_opVariableRangeIPtr;

class AMI_TestIntf_opBoolRangeTypeI : public Test::AMI_TestIntf_opBoolRangeType, public CallbackBase
{
public:

    AMI_TestIntf_opBoolRangeTypeI(Test::BoolSeq in)
        : _in(in)
    {
    }

    virtual void ice_response(const Test::BoolSeq& ret, const Test::BoolSeq& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    Test::BoolSeq _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opBoolRangeTypeI> AMI_TestIntf_opBoolRangeTypeIPtr;

class AMI_TestIntf_opByteRangeTypeI : public Test::AMI_TestIntf_opByteRangeType, public CallbackBase
{
public:

    AMI_TestIntf_opByteRangeTypeI(Test::ByteList in)
        : _in(in)
    {
    }

    virtual void ice_response(const Test::ByteList& ret, const Test::ByteList& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    Test::ByteList _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opByteRangeTypeI> AMI_TestIntf_opByteRangeTypeIPtr;

class AMI_TestIntf_opVariableRangeTypeI : public Test::AMI_TestIntf_opVariableRangeType, public CallbackBase
{
public:

    AMI_TestIntf_opVariableRangeTypeI(Test::VariableList in)
        : _in(in)
    {
    }

    virtual void ice_response(const Test::VariableList& ret, const Test::VariableList& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    Test::VariableList _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opVariableRangeTypeI> AMI_TestIntf_opVariableRangeTypeIPtr;

class AMI_TestIntf_opBoolSeqI : public Test::AMI_TestIntf_opBoolSeq, public CallbackBase
{
public:

    AMI_TestIntf_opBoolSeqI(deque<bool> in)
        : _in(in)
    {
    }

    virtual void ice_response(const deque<bool>& ret, const deque<bool>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    deque<bool> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opBoolSeqI> AMI_TestIntf_opBoolSeqIPtr;

class AMI_TestIntf_opBoolListI : public Test::AMI_TestIntf_opBoolList, public CallbackBase
{
public:

    AMI_TestIntf_opBoolListI(list<bool> in)
        : _in(in)
    {
    }

    virtual void ice_response(const list<bool>& ret, const list<bool>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    list<bool> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opBoolListI> AMI_TestIntf_opBoolListIPtr;

class AMI_TestIntf_opByteSeqI : public Test::AMI_TestIntf_opByteSeq, public CallbackBase
{
public:

    AMI_TestIntf_opByteSeqI(deque<Ice::Byte> in)
        : _in(in)
    {
    }

    virtual void ice_response(const deque<Ice::Byte>& ret, const deque<Ice::Byte>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    deque<Ice::Byte> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opByteSeqI> AMI_TestIntf_opByteSeqIPtr;

class AMI_TestIntf_opByteListI : public Test::AMI_TestIntf_opByteList, public CallbackBase
{
public:

    AMI_TestIntf_opByteListI(list<Ice::Byte> in)
        : _in(in)
    {
    }

    virtual void ice_response(const list<Ice::Byte>& ret, const list<Ice::Byte>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    list<Ice::Byte> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opByteListI> AMI_TestIntf_opByteListIPtr;

class AMI_TestIntf_opMyByteSeqI : public Test::AMI_TestIntf_opMyByteSeq, public CallbackBase
{
public:

    AMI_TestIntf_opMyByteSeqI(MyByteSeq in)
        : _in(in)
    {
    }

    virtual void ice_response(const MyByteSeq& ret, const MyByteSeq& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    MyByteSeq _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opMyByteSeqI> AMI_TestIntf_opMyByteSeqIPtr;

class AMI_TestIntf_opStringSeqI : public Test::AMI_TestIntf_opStringSeq, public CallbackBase
{
public:

    AMI_TestIntf_opStringSeqI(deque<string> in)
        : _in(in)
    {
    }

    virtual void ice_response(const deque<string>& ret, const deque<string>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    deque<string> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opStringSeqI> AMI_TestIntf_opStringSeqIPtr;

class AMI_TestIntf_opStringListI : public Test::AMI_TestIntf_opStringList, public CallbackBase
{
public:

    AMI_TestIntf_opStringListI(list<string> in)
        : _in(in)
    {
    }

    virtual void ice_response(const list<string>& ret, const list<string>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    list<string> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opStringListI> AMI_TestIntf_opStringListIPtr;

class AMI_TestIntf_opFixedSeqI : public Test::AMI_TestIntf_opFixedSeq, public CallbackBase
{
public:

    AMI_TestIntf_opFixedSeqI(deque<Test::Fixed> in)
        : _in(in)
    {
    }

    virtual void ice_response(const deque<Test::Fixed>& ret, const deque<Test::Fixed>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    deque<Test::Fixed> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opFixedSeqI> AMI_TestIntf_opFixedSeqIPtr;

class AMI_TestIntf_opFixedListI : public Test::AMI_TestIntf_opFixedList, public CallbackBase
{
public:

    AMI_TestIntf_opFixedListI(list<Test::Fixed> in)
        : _in(in)
    {
    }

    virtual void ice_response(const list<Test::Fixed>& ret, const list<Test::Fixed>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    list<Test::Fixed> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opFixedListI> AMI_TestIntf_opFixedListIPtr;

class AMI_TestIntf_opVariableSeqI : public Test::AMI_TestIntf_opVariableSeq, public CallbackBase
{
public:

    AMI_TestIntf_opVariableSeqI(deque<Test::Variable> in)
        : _in(in)
    {
    }

    virtual void ice_response(const deque<Test::Variable>& ret, const deque<Test::Variable>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    deque<Test::Variable> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opVariableSeqI> AMI_TestIntf_opVariableSeqIPtr;

class AMI_TestIntf_opVariableListI : public Test::AMI_TestIntf_opVariableList, public CallbackBase
{
public:

    AMI_TestIntf_opVariableListI(list<Test::Variable> in)
        : _in(in)
    {
    }

    virtual void ice_response(const list<Test::Variable>& ret, const list<Test::Variable>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    list<Test::Variable> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opVariableListI> AMI_TestIntf_opVariableListIPtr;

class AMI_TestIntf_opStringStringDictSeqI : public Test::AMI_TestIntf_opStringStringDictSeq, public CallbackBase
{
public:

    AMI_TestIntf_opStringStringDictSeqI(deque<Test::StringStringDict> in)
        : _in(in)
    {
    }

    virtual void ice_response(const deque<Test::StringStringDict>& ret, const deque<Test::StringStringDict>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    deque<Test::StringStringDict> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opStringStringDictSeqI> AMI_TestIntf_opStringStringDictSeqIPtr;

class AMI_TestIntf_opStringStringDictListI : public Test::AMI_TestIntf_opStringStringDictList, public CallbackBase
{
public:

    AMI_TestIntf_opStringStringDictListI(list<Test::StringStringDict> in)
        : _in(in)
    {
    }

    virtual void ice_response(const list<Test::StringStringDict>& ret, const list<Test::StringStringDict>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    list<Test::StringStringDict> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opStringStringDictListI> AMI_TestIntf_opStringStringDictListIPtr;

class AMI_TestIntf_opESeqI : public Test::AMI_TestIntf_opESeq, public CallbackBase
{
public:

    AMI_TestIntf_opESeqI(deque<Test::E> in)
        : _in(in)
    {
    }

    virtual void ice_response(const deque<Test::E>& ret, const deque<Test::E>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    deque<Test::E> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opESeqI> AMI_TestIntf_opESeqIPtr;

class AMI_TestIntf_opEListI : public Test::AMI_TestIntf_opEList, public CallbackBase
{
public:

    AMI_TestIntf_opEListI(list<Test::E> in)
        : _in(in)
    {
    }

    virtual void ice_response(const list<Test::E>& ret, const list<Test::E>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    list<Test::E> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opEListI> AMI_TestIntf_opEListIPtr;

class AMI_TestIntf_opCPrxSeqI : public Test::AMI_TestIntf_opCPrxSeq, public CallbackBase
{
public:

    AMI_TestIntf_opCPrxSeqI(deque<Test::CPrx> in)
        : _in(in)
    {
    }

    virtual void ice_response(const deque<Test::CPrx>& ret, const deque<Test::CPrx>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    deque<Test::CPrx> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opCPrxSeqI> AMI_TestIntf_opCPrxSeqIPtr;

class AMI_TestIntf_opCPrxListI : public Test::AMI_TestIntf_opCPrxList, public CallbackBase
{
public:

    AMI_TestIntf_opCPrxListI(list<Test::CPrx> in)
        : _in(in)
    {
    }

    virtual void ice_response(const list<Test::CPrx>& ret, const list<Test::CPrx>& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    list<Test::CPrx> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opCPrxListI> AMI_TestIntf_opCPrxListIPtr;

class AMI_TestIntf_opCSeqI : public Test::AMI_TestIntf_opCSeq, public CallbackBase
{
public:

    AMI_TestIntf_opCSeqI(deque<Test::CPtr> in)
        : _in(in)
    {
    }

    virtual void ice_response(const deque<Test::CPtr>& ret, const deque<Test::CPtr>& out)
    {
        test(out.size() == _in.size());
        test(ret.size() == _in.size());
        for(unsigned int i = 1; i < _in.size(); ++i)
        {
            test(out[i] == out[0]);
            test(ret[i] == out[i]);
        }
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    deque<Test::CPtr> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opCSeqI> AMI_TestIntf_opCSeqIPtr;

class AMI_TestIntf_opCListI : public Test::AMI_TestIntf_opCList, public CallbackBase
{
public:

    AMI_TestIntf_opCListI(list<Test::CPtr> in)
        : _in(in)
    {
    }

    virtual void ice_response(const list<Test::CPtr>& ret, const list<Test::CPtr>& out)
    {
        test(out.size() == _in.size());
        test(ret.size() == _in.size());
        list<Test::CPtr>::const_iterator p1;
        list<Test::CPtr>::const_iterator p2;
        for(p1 = out.begin(), p2 = ret.begin(); p1 != out.end(); ++p1, ++p2)
        {
            test(*p1 == *p2);
        }
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    list<Test::CPtr> _in;
};

typedef IceUtil::Handle<AMI_TestIntf_opCListI> AMI_TestIntf_opCListIPtr;

class AMI_TestIntf_opClassStructI : public Test::AMI_TestIntf_opClassStruct, public CallbackBase
{
public:

    AMI_TestIntf_opClassStructI(const Test::ClassStructPtr& cs, const Test::ClassStructSeq& csseq1) :
        _cs(cs), _csseq1(csseq1)
    {
    }

    virtual void ice_response(const ::Test::ClassStructPtr& ret,
                              const ::Test::ClassStructPtr& cs1,
                              const ::Test::ClassStructSeq& seq)
    {
        test(ret == _cs);
        test(cs1 == _cs);
        test(seq == _csseq1);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    const Test::ClassStructPtr _cs;
    const Test::ClassStructSeq _csseq1;
};
typedef IceUtil::Handle<AMI_TestIntf_opClassStructI> AMI_TestIntf_opClassStructIPtr;

class AMI_Test1_opStringI : public Test1::AMI_WstringClass_opString, public CallbackBase
{
public:

    AMI_Test1_opStringI(wstring in)
        : _in(in)
    {
    }

    virtual void ice_response(const wstring& ret, const wstring& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    wstring _in;
};

typedef IceUtil::Handle<AMI_Test1_opStringI> AMI_Test1_opStringIPtr;

class AMI_Test2_opStringI : public Test2::AMI_WstringClass_opString, public CallbackBase
{
public:

    AMI_Test2_opStringI(wstring in)
        : _in(in)
    {
    }

    virtual void ice_response(const wstring& ret, const wstring& out)
    {
        test(out == _in);
        test(ret == _in);
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }

private:

    wstring _in;
};

typedef IceUtil::Handle<AMI_Test2_opStringI> AMI_Test2_opStringIPtr;

class AMI_Test1_throwExceptI : public Test1::AMI_WstringClass_throwExcept, public CallbackBase
{
public:

    AMI_Test1_throwExceptI(wstring in)
        : _in(in)
    {
    }

    virtual void ice_response()
    {
        test(false);
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
        try
        {
            ex.ice_throw();
        }
        catch(const Test1::WstringException& e)
        {
            test(e.reason == _in);
            called();
        }
        catch(...)
        {
            test(false);
        }
    }

private:

    wstring _in;
};

typedef IceUtil::Handle<AMI_Test1_throwExceptI> AMI_Test1_throwExceptIPtr;

class AMI_Test2_throwExceptI : public Test2::AMI_WstringClass_throwExcept, public CallbackBase
{
public:

    AMI_Test2_throwExceptI(wstring in)
        : _in(in)
    {
    }

    virtual void ice_response()
    {
        test(false);
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
        try
        {
            ex.ice_throw();
        }
        catch(const Test2::WstringException& e)
        {
            test(e.reason == _in);
            called();
        }
        catch(...)
        {
            test(false);
        }
    }

private:

    wstring _in;
};

typedef IceUtil::Handle<AMI_Test2_throwExceptI> AMI_Test2_throwExceptIPtr;

Test::TestIntfPrx
allTests(const Ice::CommunicatorPtr& communicator, bool collocated)
{
    cout << "testing stringToProxy... " << flush;
    string ref = communicator->getProperties()->getPropertyWithDefault(
        "Custom.Proxy", "test:default -p 12010 -t 10000");
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    Test::TestIntfPrx t = Test::TestIntfPrx::checkedCast(base);
    test(t);
    test(t == base);
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

    {
        deque<Test::CPtr> in(5);
        in[0] = new Test::C();
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
        in.push_back(new Test::C());
        in.push_back(new Test::C());
        in.push_back(new Test::C());
        in.push_back(new Test::C());
        in.push_back(new Test::C());

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

    if(!collocated)
    {
        cout << "testing alternate sequences with AMI... " << flush;

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

            AMI_TestIntf_opBoolArrayIPtr cb = new AMI_TestIntf_opBoolArrayI(in);
            t->opBoolArray_async(cb, inPair);
            test(cb->check());
        }

        {
            Ice::Byte in[5];
            in[0] = '1';
            in[1] = '2';
            in[2] = '3';
            in[3] = '4';
            in[4] = '5';
            pair<const Ice::Byte*, const Ice::Byte*> inPair(in, in + 5);

            AMI_TestIntf_opByteArrayIPtr cb = new AMI_TestIntf_opByteArrayI(inPair);
            t->opByteArray_async(cb, inPair);
            test(cb->check());
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

            AMI_TestIntf_opVariableArrayIPtr cb = new AMI_TestIntf_opVariableArrayI(in);
            t->opVariableArray_async(cb, inPair);
            test(cb->check());
        }

        {
            Test::BoolSeq in(5);
            in[0] = false;
            in[1] = true;
            in[2] = true;
            in[3] = false;
            in[4] = true;
            pair<Test::BoolSeq::const_iterator, Test::BoolSeq::const_iterator> inPair(in.begin(), in.end());

            AMI_TestIntf_opBoolRangeIPtr cb = new AMI_TestIntf_opBoolRangeI(in);
            t->opBoolRange_async(cb, inPair);
            test(cb->check());
        }

        {
            Test::ByteList in;
            in.push_back('1');
            in.push_back('2');
            in.push_back('3');
            in.push_back('4');
            in.push_back('5');
            pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator> inPair(in.begin(), in.end());

            AMI_TestIntf_opByteRangeIPtr cb = new AMI_TestIntf_opByteRangeI(in);
            t->opByteRange_async(cb, inPair);
            test(cb->check());
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

            AMI_TestIntf_opVariableRangeIPtr cb = new AMI_TestIntf_opVariableRangeI(in);
            t->opVariableRange_async(cb, inPair);
            test(cb->check());
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

            AMI_TestIntf_opBoolRangeTypeIPtr cb = new AMI_TestIntf_opBoolRangeTypeI(in);
            t->opBoolRangeType_async(cb, inPair);
            test(cb->check());
        }

        {
            Test::ByteList in;
            in.push_back('1');
            in.push_back('2');
            in.push_back('3');
            in.push_back('4');
            in.push_back('5');
            pair<Test::ByteList::const_iterator, Test::ByteList::const_iterator> inPair(in.begin(), in.end());

            AMI_TestIntf_opByteRangeTypeIPtr cb = new AMI_TestIntf_opByteRangeTypeI(in);
            t->opByteRangeType_async(cb, inPair);
            test(cb->check());
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

            AMI_TestIntf_opVariableRangeTypeIPtr cb = new AMI_TestIntf_opVariableRangeTypeI(in);
            t->opVariableRangeType_async(cb, inPair);
            test(cb->check());
        }

        {
            deque<bool> in(5);
            in[0] = false;
            in[1] = true;
            in[2] = true;
            in[3] = false;
            in[4] = true;

            AMI_TestIntf_opBoolSeqIPtr cb = new AMI_TestIntf_opBoolSeqI(in);
            t->opBoolSeq_async(cb, in);
            test(cb->check());
        }

        {
            list<bool> in;
            in.push_back(false);
            in.push_back(true);
            in.push_back(true);
            in.push_back(false);
            in.push_back(true);

            AMI_TestIntf_opBoolListIPtr cb = new AMI_TestIntf_opBoolListI(in);
            t->opBoolList_async(cb, in);
            test(cb->check());
        }

        {
            deque< ::Ice::Byte> in(5);
            in[0] = '1';
            in[1] = '2';
            in[2] = '3';
            in[3] = '4';
            in[4] = '5';

            AMI_TestIntf_opByteSeqIPtr cb = new AMI_TestIntf_opByteSeqI(in);
            t->opByteSeq_async(cb, in);
            test(cb->check());
        }

        {
            list< ::Ice::Byte> in;
            in.push_back('1');
            in.push_back('2');
            in.push_back('3');
            in.push_back('4');
            in.push_back('5');

            AMI_TestIntf_opByteListIPtr cb = new AMI_TestIntf_opByteListI(in);
            t->opByteList_async(cb, in);
            test(cb->check());
        }

        {
            MyByteSeq in(5);
            int i = 0;
            for(MyByteSeq::iterator p = in.begin(); p != in.end(); ++p)
            {
                *p = '1' + i++;
            }

            AMI_TestIntf_opMyByteSeqIPtr cb = new AMI_TestIntf_opMyByteSeqI(in);
            t->opMyByteSeq_async(cb, in);
            test(cb->check());
        }

        {
            deque<string> in(5);
            in[0] = "These";
            in[1] = "are";
            in[2] = "five";
            in[3] = "short";
            in[4] = "strings.";

            AMI_TestIntf_opStringSeqIPtr cb = new AMI_TestIntf_opStringSeqI(in);
            t->opStringSeq_async(cb, in);
            test(cb->check());
        }

        {
            list<string> in;
            in.push_back("These");
            in.push_back("are");
            in.push_back("five");
            in.push_back("short");
            in.push_back("strings.");

            AMI_TestIntf_opStringListIPtr cb = new AMI_TestIntf_opStringListI(in);
            t->opStringList_async(cb, in);
            test(cb->check());
        }

        {
            deque<Test::Fixed> in(5);
            in[0].s = 1;
            in[1].s = 2;
            in[2].s = 3;
            in[3].s = 4;
            in[4].s = 5;

            AMI_TestIntf_opFixedSeqIPtr cb = new AMI_TestIntf_opFixedSeqI(in);
            t->opFixedSeq_async(cb, in);
            test(cb->check());
        }

        {
            list<Test::Fixed> in(5);
            short num = 1;
            for(list<Test::Fixed>::iterator p = in.begin(); p != in.end(); ++p)
            {
                (*p).s = num++;
            }

            AMI_TestIntf_opFixedListIPtr cb = new AMI_TestIntf_opFixedListI(in);
            t->opFixedList_async(cb, in);
            test(cb->check());
        }

        {
            deque<Test::Variable> in(5);
            in[0].s = "These";
            in[1].s = "are";
            in[2].s = "five";
            in[3].s = "short";
            in[4].s = "strings.";

            AMI_TestIntf_opVariableSeqIPtr cb = new AMI_TestIntf_opVariableSeqI(in);
            t->opVariableSeq_async(cb, in);
            test(cb->check());
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

            AMI_TestIntf_opVariableListIPtr cb = new AMI_TestIntf_opVariableListI(in);
            t->opVariableList_async(cb, in);
            test(cb->check());
        }

        {
            deque<Test::StringStringDict> in(5);
            in[0]["A"] = "a";
            in[1]["B"] = "b";
            in[2]["C"] = "c";
            in[3]["D"] = "d";
            in[4]["E"] = "e";

            AMI_TestIntf_opStringStringDictSeqIPtr cb = new AMI_TestIntf_opStringStringDictSeqI(in);
            t->opStringStringDictSeq_async(cb, in);
            test(cb->check());
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

            AMI_TestIntf_opStringStringDictListIPtr cb = new AMI_TestIntf_opStringStringDictListI(in);
            t->opStringStringDictList_async(cb, in);
            test(cb->check());
        }

        {
            deque<Test::E> in(5);
            in[0] = Test::E1;
            in[1] = Test::E2;
            in[2] = Test::E3;
            in[3] = Test::E1;
            in[4] = Test::E3;

            AMI_TestIntf_opESeqIPtr cb = new AMI_TestIntf_opESeqI(in);
            t->opESeq_async(cb, in);
            test(cb->check());
        }

        {
            list<Test::E> in;
            in.push_back(Test::E1);
            in.push_back(Test::E2);
            in.push_back(Test::E3);
            in.push_back(Test::E1);
            in.push_back(Test::E3);

            AMI_TestIntf_opEListIPtr cb = new AMI_TestIntf_opEListI(in);
            t->opEList_async(cb, in);
            test(cb->check());
        }

        {
            deque<Test::CPrx> in(5);
            in[0] = Test::CPrx::uncheckedCast(communicator->stringToProxy("C1:default -p 12010 -t 10000"));
            in[1] = Test::CPrx::uncheckedCast(communicator->stringToProxy("C2:default -p 12010 -t 10001"));
            in[2] = Test::CPrx::uncheckedCast(communicator->stringToProxy("C3:default -p 12010 -t 10002"));
            in[3] = Test::CPrx::uncheckedCast(communicator->stringToProxy("C4:default -p 12010 -t 10003"));
            in[4] = Test::CPrx::uncheckedCast(communicator->stringToProxy("C5:default -p 12010 -t 10004"));

            AMI_TestIntf_opCPrxSeqIPtr cb = new AMI_TestIntf_opCPrxSeqI(in);
            t->opCPrxSeq_async(cb, in);
            test(cb->check());
        }

        {
            list<Test::CPrx> in;
            in.push_back(Test::CPrx::uncheckedCast(communicator->stringToProxy("C1:default -p 12010 -t 10000")));
            in.push_back(Test::CPrx::uncheckedCast(communicator->stringToProxy("C2:default -p 12010 -t 10001")));
            in.push_back(Test::CPrx::uncheckedCast(communicator->stringToProxy("C3:default -p 12010 -t 10002")));
            in.push_back(Test::CPrx::uncheckedCast(communicator->stringToProxy("C4:default -p 12010 -t 10003")));
            in.push_back(Test::CPrx::uncheckedCast(communicator->stringToProxy("C5:default -p 12010 -t 10004")));

            AMI_TestIntf_opCPrxListIPtr cb = new AMI_TestIntf_opCPrxListI(in);
            t->opCPrxList_async(cb, in);
            test(cb->check());
        }

        {
            deque<Test::CPtr> in(5);
            in[0] = new Test::C();
            in[1] = in[0];
            in[2] = in[0];
            in[3] = in[0];
            in[4] = in[0];

            AMI_TestIntf_opCSeqIPtr cb = new AMI_TestIntf_opCSeqI(in);
            t->opCSeq_async(cb, in);
            test(cb->check());
        }
    
        {
            list<Test::CPtr> in;
            in.push_back(new Test::C());
            in.push_back(new Test::C());
            in.push_back(new Test::C());
            in.push_back(new Test::C());
            in.push_back(new Test::C());

            AMI_TestIntf_opCListIPtr cb = new AMI_TestIntf_opCListI(in);
            t->opCList_async(cb, in);
            test(cb->check());
        }

        cout << "ok" << endl;
    }

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

    if(!collocated)
    {
        cout << "testing class mapped structs with AMI... " << flush;
        AMI_TestIntf_opClassStructIPtr cb = new AMI_TestIntf_opClassStructI(cs, csseq1);
        t->opClassStruct_async(cb, cs, csseq1);
        test(cb->check());
        cout << "ok" << endl;
    }

    cout << "testing wstring... " << flush;

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

    if(!collocated)
    {
        AMI_Test1_opStringIPtr cb = new AMI_Test1_opStringI(wstr);
        wsc1->opString_async(cb, wstr);
        test(cb->check());
    }

    ret = wsc2->opString(wstr, out);
    test(out == wstr);
    test(ret == wstr);

    if(!collocated)
    {
        AMI_Test2_opStringIPtr cb = new AMI_Test2_opStringI(wstr);
        wsc2->opString_async(cb, wstr);
        test(cb->check());
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

    if(!collocated)
    {
        AMI_Test1_throwExceptIPtr cb = new AMI_Test1_throwExceptI(wstr);
        wsc1->throwExcept_async(cb, wstr);
        test(cb->check());
    }

    try
    {
        wsc2->throwExcept(wstr);
    }
    catch(const Test2::WstringException& ex)
    {
        test(ex.reason == wstr);
    }

    if(!collocated)
    {
        AMI_Test2_throwExceptIPtr cb = new AMI_Test2_throwExceptI(wstr);
        wsc2->throwExcept_async(cb, wstr);
        test(cb->check());
    }

    cout << "ok" << endl;

    return t;
}
