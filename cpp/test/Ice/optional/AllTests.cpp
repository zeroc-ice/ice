// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

#ifdef ICE_CPP11_MAPPING
namespace Test
{
using OneOptionalPrx = Ice::ObjectPrx;
using OneOptionalPrxPtr = ::std::shared_ptr<Ice::ObjectPrx>;
using MultiOptionalPrx = Ice::ObjectPrx;
}
#endif

using namespace std;
using namespace Test;

namespace
{

//
// Converts a vector to an "array range"
//
template<typename T> pair<const T*, const T*>
toArrayRange(const vector<T>& v)
{
    return make_pair(&v[0], &v[0] + v.size());
}

template<typename T> pair<const T*, const T*>
toArrayRange(const T* v, size_t sz)
{
    return make_pair(v, v + sz);
}

}

#ifdef ICE_CPP11_MAPPING
class TestObjectReader : public Ice::Value
#else
class TestObjectReader : public Ice::Object
#endif
{
public:
    virtual void _iceWrite(Ice::OutputStream*) const { }

    virtual void _iceRead(Ice::InputStream* in)
    {
        in->startValue();
        in->startSlice();
        in->endSlice();
        in->endValue(false);
    }

#ifdef ICE_CPP11_MAPPING

protected:

    virtual std::shared_ptr<Value> _iceCloneImpl() const
    {
        assert(0); // not used
        return nullptr;
    }

#endif
};

#ifdef ICE_CPP11_MAPPING
class BObjectReader : public Ice::Value
#else
class BObjectReader : public Ice::Object
#endif
{
public:
    virtual void _iceWrite(Ice::OutputStream*) const { }

    virtual void _iceRead(Ice::InputStream* in)
    {
        in->startValue();
        // ::Test::B
        in->startSlice();
        Ice::Int v;
        in->read(v);
        in->endSlice();
        // ::Test::A
        in->startSlice();
        in->read(v);
        in->endSlice();
        in->endValue(false);
    }

#ifdef ICE_CPP11_MAPPING

protected:

    virtual std::shared_ptr<Value> _iceCloneImpl() const
    {
        assert(0); // not used
        return nullptr;
    }

#endif
};

#ifdef ICE_CPP11_MAPPING
class CObjectReader : public Ice::Value
#else
class CObjectReader : public Ice::Object
#endif
{
public:
    virtual void _iceWrite(Ice::OutputStream*) const { }

    virtual void _iceRead(Ice::InputStream* in)
    {
        in->startValue();
        // ::Test::C
        in->startSlice();
        in->skipSlice();
        // ::Test::B
        in->startSlice();
        Ice::Int v;
        in->read(v);
        in->endSlice();
        // ::Test::A
        in->startSlice();
        in->read(v);
        in->endSlice();
        in->endValue(false);
    }

#ifdef ICE_CPP11_MAPPING

protected:

    virtual std::shared_ptr<Value> _iceCloneImpl() const
    {
        assert(0); // not used
        return nullptr;
    }

#endif
};

#ifdef ICE_CPP11_MAPPING
class DObjectWriter : public Ice::Value
#else
class DObjectWriter : public Ice::Object
#endif
{
public:

    virtual void _iceWrite(Ice::OutputStream* out) const
    {
        out->startValue(0);
        // ::Test::D
        out->startSlice("::Test::D", -1, false);
        string s = "test";
        out->write(s);
        IceUtil::Optional<vector<string> > o;
        o = vector<string>();
        o->push_back("test1");
        o->push_back("test2");
        o->push_back("test3");
        o->push_back("test4");
        out->write(1, o);
        APtr a = ICE_MAKE_SHARED(A);
        a->mc = 18;
        out->write(1000, IceUtil::Optional<APtr>(a));
        out->endSlice();
        // ::Test::B
        out->startSlice(B::ice_staticId(), -1, false);
        Ice::Int v = 14;
        out->write(v);
        out->endSlice();
        // ::Test::A
        out->startSlice(A::ice_staticId(), -1, true);
        out->write(v);
        out->endSlice();
        out->endValue();
    }

    virtual void _iceRead(Ice::InputStream*) { }

#ifdef ICE_CPP11_MAPPING

protected:

    virtual std::shared_ptr<Value> _iceCloneImpl() const
    {
        assert(0); // not used
        return nullptr;
    }

#endif

};

#ifdef ICE_CPP11_MAPPING
class DObjectReader : public Ice::Value
#else
class DObjectReader : public Ice::Object
#endif
{
public:
    virtual void _iceWrite(Ice::OutputStream*) const { }

    virtual void _iceRead(Ice::InputStream* in)
    {
        in->startValue();
        // ::Test::D
        in->startSlice();
        string s;
        in->read(s);
        test(s == "test");
        IceUtil::Optional<vector<string> > o;
        in->read(1, o);
        test(o && o->size() == 4 &&
             (*o)[0] == "test1" && (*o)[1] == "test2" && (*o)[2] == "test3" && (*o)[3] == "test4");
        in->read(1000, a);
        in->endSlice();
        // ::Test::B
        in->startSlice();
        Ice::Int v;
        in->read(v);
        in->endSlice();
        // ::Test::A
        in->startSlice();
        in->read(v);
        in->endSlice();
        in->endValue(false);
    }

    void check()
    {
        test((*a)->mc == 18);
    }

#ifdef ICE_CPP11_MAPPING

protected:

    virtual std::shared_ptr<Value> _iceCloneImpl() const
    {
        assert(0); // not used
        return nullptr;
    }

#endif

private:

    IceUtil::Optional<APtr> a;
};

#ifdef ICE_CPP11_MAPPING
class FObjectReader : public Ice::Value
#else
class FObjectReader : public Ice::Object
#endif
{
public:
    virtual void _iceWrite(Ice::OutputStream*) const { }

    virtual void _iceRead(Ice::InputStream* in)
    {
        _f = ICE_MAKE_SHARED(F);
        in->startValue();
        in->startSlice();
        // Don't read af on purpose
        //in.read(1, _f->af);
        in->endSlice();
        in->startSlice();
        in->read(_f->ae);
        in->endSlice();
        in->endValue(false);
    }

    FPtr
    getF()
    {
        return _f;
    }

#ifdef ICE_CPP11_MAPPING

protected:

    virtual std::shared_ptr<Value> _iceCloneImpl() const
    {
        assert(0); // not used
        return nullptr;
    }

#endif

private:

    FPtr _f;
};

class FactoryI
#ifndef ICE_CPP11_MAPPING
               : public Ice::ValueFactory
#endif

{
    bool _enabled;

public:

    FactoryI() : _enabled(false)
    {
    }

    Ice::ValuePtr
    create(const string& typeId)
    {
        if(!_enabled)
        {
            return 0;
        }

        if(typeId == "::Test::OneOptional")
        {
           return ICE_MAKE_SHARED(TestObjectReader);
        }
        else if(typeId == "::Test::MultiOptional")
        {
           return ICE_MAKE_SHARED(TestObjectReader);
        }
        else if(typeId == "::Test::B")
        {
           return ICE_MAKE_SHARED(BObjectReader);
        }
        else if(typeId == "::Test::C")
        {
           return ICE_MAKE_SHARED(CObjectReader);
        }
        else if(typeId == "::Test::D")
        {
           return ICE_MAKE_SHARED(DObjectReader);
        }
        else if(typeId == "::Test::F")
        {
           return ICE_MAKE_SHARED(FObjectReader);
        }

        return 0;
    }

    void
    setEnabled(bool enabled)
    {
        _enabled = enabled;
    }
};

#ifdef ICE_CPP11_MAPPING
using FactoryIPtr = shared_ptr<FactoryI>;
#else
typedef IceUtil::Handle<FactoryI> FactoryIPtr;
#endif

InitialPrxPtr
allTests(Test::TestHelper* helper, bool)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    FactoryIPtr factory = ICE_MAKE_SHARED(FactoryI);

#ifdef ICE_CPP11_MAPPING
    communicator->getValueFactoryManager()->add([factory](const string& typeId)
                                                {
                                                    return factory->create(typeId);
                                                },
                                                "");
#else
    communicator->getValueFactoryManager()->add(factory, "");
#endif

    cout << "testing stringToProxy... " << flush;
    string ref = "initial:" + helper->getTestEndpoint();
    Ice::ObjectPrxPtr base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    InitialPrxPtr initial = ICE_CHECKED_CAST(InitialPrx, base);
    test(initial);

#ifdef ICE_CPP11_MAPPING
    test(targetEqualTo(initial, base));
#else
    test(initial == base);
#endif

    bool supportsCppStringView = initial->supportsCppStringView();
    cout << "ok" << endl;

    cout << "testing constructor, copy constructor, and assignment operator... " << flush;

    OneOptionalPtr oo1 = ICE_MAKE_SHARED(OneOptional);
    test(!oo1->a);
    oo1->a = 15;
    test(oo1->a && *oo1->a == 15);

    OneOptionalPtr oo2 = ICE_MAKE_SHARED(OneOptional, 16);
    test(oo2->a && *oo2->a == 16);

    OneOptionalPtr oo3 = ICE_MAKE_SHARED(OneOptional, *oo2);
    test(oo3->a && *oo3->a == 16);

    *oo3 = *oo1;
    test(oo3->a && *oo3->a == 15);

    OneOptionalPtr oon = ICE_MAKE_SHARED(OneOptional, IceUtil::None);
    test(!oon->a);

    MultiOptionalPtr mo1 = ICE_MAKE_SHARED(MultiOptional);
    mo1->a = 15;
    mo1->b = true;
    mo1->c = 19;
    mo1->d = 78;
    mo1->e = 99;
    mo1->f = 5.5f;
    mo1->g = 1.0;
    mo1->h = string("test");
    mo1->i = ICE_ENUM(MyEnum, MyEnumMember);
    mo1->j = ICE_UNCHECKED_CAST(MultiOptionalPrx, communicator->stringToProxy("test"));
    mo1->k = mo1;
    mo1->bs = ByteSeq();
    (*mo1->bs).push_back(5);
    mo1->ss = StringSeq();
    mo1->ss->push_back("test");
    mo1->ss->push_back("test2");
    mo1->iid = IntIntDict();
    (*mo1->iid)[4] = 3;
    mo1->sid = StringIntDict();
    (*mo1->sid)["test"] = 10;
    FixedStruct fs;
    fs.m = 78;
    mo1->fs = fs;
    VarStruct vs;
    vs.m = "hello";
    mo1->vs = vs;

    mo1->shs = ShortSeq();
    mo1->shs->push_back(1);
    mo1->es = MyEnumSeq();
    mo1->es->push_back(ICE_ENUM(MyEnum, MyEnumMember));
    mo1->es->push_back(ICE_ENUM(MyEnum, MyEnumMember));
    mo1->fss = FixedStructSeq();
    mo1->fss->push_back(fs);
    mo1->vss = VarStructSeq();
    mo1->vss->push_back(vs);
    mo1->oos = OneOptionalSeq();
    mo1->oos->push_back(oo1);
    mo1->oops = OneOptionalPrxSeq();
    mo1->oops->push_back(ICE_UNCHECKED_CAST(OneOptionalPrx, communicator->stringToProxy("test")));

    mo1->ied = IntEnumDict();
    mo1->ied.value()[4] = ICE_ENUM(MyEnum, MyEnumMember);
    mo1->ifsd = IntFixedStructDict();
    mo1->ifsd.value()[4] = fs;
    mo1->ivsd = IntVarStructDict();
    mo1->ivsd.value()[5] = vs;
    mo1->iood = IntOneOptionalDict();
    mo1->iood.value()[5] = ICE_MAKE_SHARED(OneOptional);
    mo1->iood.value()[5]->a = 15;
    mo1->ioopd = IntOneOptionalPrxDict();
    mo1->ioopd.value()[5] = ICE_UNCHECKED_CAST(OneOptionalPrx, communicator->stringToProxy("test"));

    mo1->bos = BoolSeq();
    mo1->bos->push_back(false);
    mo1->bos->push_back(true);
    mo1->bos->push_back(false);

#ifndef ICE_CPP11_MAPPING
    mo1->ice_collectable(true);
#endif

    MultiOptionalPtr mo2 = ICE_MAKE_SHARED(MultiOptional, *mo1);

    MultiOptionalPtr mo3 = ICE_MAKE_SHARED(MultiOptional);
    *mo3 = *mo2;

    test(mo3->a == static_cast<Ice::Byte>(15));
    test(mo3->b == true);
    test(mo3->c == static_cast<short>(19));
    test(mo3->d == 78);
    test(mo3->e == static_cast<Ice::Long>(99));
    test(mo3->f == 5.5f);
    test(mo3->g == 1.0);
    test(mo3->h == string("test"));
    test(mo3->i = ICE_ENUM(MyEnum, MyEnumMember));
    test(mo3->j = ICE_UNCHECKED_CAST(MultiOptionalPrx, communicator->stringToProxy("test")));
    test(mo3->k == mo1);
    test(mo3->bs == mo1->bs);
    test(mo3->ss == mo1->ss);
    test(mo3->iid == mo1->iid);
    test(mo3->sid == mo1->sid);
    test(mo3->fs == mo1->fs);
    test(mo3->vs == mo1->vs);

    test(mo3->shs == mo1->shs);
    test(mo3->es == mo1->es);
    test(mo3->fss == mo1->fss);
    test(mo3->vss == mo1->vss);
    test(mo3->oos == mo1->oos);
    test(mo3->oops == mo1->oops);

    test(mo3->ied == mo1->ied);
    test(mo3->ifsd == mo1->ifsd);
    test(mo3->ivsd == mo1->ivsd);
    test(mo3->iood == mo1->iood);
    test(mo3->ioopd == mo1->ioopd);

    test(mo3->bos == mo1->bos);

    cout << "ok" << endl;

    cout << "testing comparison operators... " << flush;

    test(mo1->a == static_cast<Ice::Byte>(15) && static_cast<Ice::Byte>(15) == mo1->a &&
        mo1->a != static_cast<Ice::Byte>(16) && static_cast<Ice::Byte>(16) != mo1->a);
    test(mo1->a < static_cast<Ice::Byte>(16) && mo1->a > static_cast<Ice::Byte>(14) &&
        mo1->a <= static_cast<Ice::Byte>(15) && mo1->a >= static_cast<Ice::Byte>(15) &&
        mo1->a <= static_cast<Ice::Byte>(16) && mo1->a >= static_cast<Ice::Byte>(14));
    test(mo1->a > IceUtil::Optional<Ice::Byte>() && IceUtil::Optional<Ice::Byte>() < mo1->a);
    test(14 > IceUtil::Optional<int>() && IceUtil::Optional<int>() < 14);

    test(mo1->h == string("test") && string("test") == mo1->h && mo1->h != string("testa") && string("testa") != mo1->h);
    test(mo1->h < string("test1") && mo1->h > string("tesa") && mo1->h <= string("test"));
    test(mo1->h >= string("test") && mo1->h <= string("test1") && mo1->h >= string("tesa"));
    test(mo1->h > IceUtil::Optional<string>() && IceUtil::Optional<string>() < mo1->h);
    test(string("test1") > IceUtil::Optional<string>() && IceUtil::Optional<string>() < string("test1"));

    cout << "ok" << endl;

    cout << "testing marshalling... " << flush;
    OneOptionalPtr oo4 = ICE_DYNAMIC_CAST(OneOptional, initial->pingPong(ICE_MAKE_SHARED(OneOptional)));
    test(!oo4->a);

    OneOptionalPtr oo5 = ICE_DYNAMIC_CAST(OneOptional, initial->pingPong(oo1));
    test(oo1->a == oo5->a);

    MultiOptionalPtr mo4 = ICE_DYNAMIC_CAST(MultiOptional, initial->pingPong(ICE_MAKE_SHARED(MultiOptional)));
    test(!mo4->a);
    test(!mo4->b);
    test(!mo4->c);
    test(!mo4->d);
    test(!mo4->e);
    test(!mo4->f);
    test(!mo4->g);
    test(!mo4->h);
    test(!mo4->i);
    test(!mo4->j);
    test(!mo4->k);
    test(!mo4->bs);
    test(!mo4->ss);
    test(!mo4->iid);
    test(!mo4->sid);
    test(!mo4->fs);
    test(!mo4->vs);

    test(!mo4->shs);
    test(!mo4->es);
    test(!mo4->fss);
    test(!mo4->vss);
    test(!mo4->oos);
    test(!mo4->oops);

    test(!mo4->ied);
    test(!mo4->ifsd);
    test(!mo4->ivsd);
    test(!mo4->iood);
    test(!mo4->ioopd);

    test(!mo4->bos);

    mo1->k = mo1;
    MultiOptionalPtr mo5 = ICE_DYNAMIC_CAST(MultiOptional, initial->pingPong(mo1));
    test(mo5->a == mo1->a);
    test(mo5->b == mo1->b);
    test(mo5->c == mo1->c);
    test(mo5->d == mo1->d);
    test(mo5->e == mo1->e);
    test(mo5->f == mo1->f);
    test(mo5->g == mo1->g);
    test(mo5->h == mo1->h);
    test(mo5->i == mo1->i);
#ifdef ICE_CPP11_MAPPING
    test(targetEqualTo(mo5->j.value(), mo1->j.value()));
#else
    test(mo5->j == mo1->j);
#endif
    test(mo5->k == mo5->k);
    test(mo5->bs == mo1->bs);
    test(mo5->ss == mo1->ss);
    test(mo5->iid == mo1->iid);
    test(mo5->sid == mo1->sid);
    test(mo5->fs == mo1->fs);
    test(mo5->vs == mo1->vs);

    test(mo5->shs == mo1->shs);
    test(mo5->es == mo1->es);
    test(mo5->fss == mo1->fss);
    test(mo5->vss == mo1->vss);
    test(!mo5->oos->empty() && (*mo5->oos)[0]->a == oo1->a);

#ifdef ICE_CPP11_MAPPING
    test(mo5->oops.value().size() == mo1->oops.value().size());
    for(size_t i = 0; i< mo5->oops.value().size(); ++i)
    {
        test(targetEqualTo(mo5->oops.value()[i], mo1->oops.value()[i]));
    }
#else
    test(mo5->oops == mo1->oops);
#endif

    test(mo5->ied == mo1->ied);
    test(mo5->ifsd == mo1->ifsd);
    test(mo5->ivsd == mo1->ivsd);
    test(!mo5->iood->empty() && (*mo5->iood)[5]->a == 15);

#ifdef ICE_CPP11_MAPPING
    test(mo5->ioopd.value().size() == mo1->ioopd.value().size());
    for(auto& v : mo5->ioopd.value())
    {
        test(targetEqualTo(mo1->ioopd.value()[v.first], v.second));
    }
#else
    test(mo5->ioopd == mo1->ioopd);
#endif

    test(mo5->bos == mo1->bos);

    // Clear the first half of the optional parameters
    MultiOptionalPtr mo6 = ICE_MAKE_SHARED(MultiOptional, *mo5);
    mo6->a = IceUtil::None;
    mo6->c = IceUtil::None;
    mo6->e = IceUtil::None;
    mo6->g = IceUtil::None;
    mo6->i = IceUtil::None;
    mo6->k = IceUtil::None;
    mo6->ss = IceUtil::None;
    mo6->sid = IceUtil::None;
    mo6->vs = IceUtil::None;

    mo6->es = IceUtil::None;
    mo6->vss = IceUtil::None;
    mo6->oops = IceUtil::None;

    mo6->ied = IceUtil::None;
    mo6->ivsd = IceUtil::None;
    mo6->ioopd = IceUtil::None;

    MultiOptionalPtr mo7 = ICE_DYNAMIC_CAST(MultiOptional, initial->pingPong(mo6));
    test(!mo7->a);
    test(mo7->b == mo1->b);
    test(!mo7->c);
    test(mo7->d == mo1->d);
    test(!mo7->e);
    test(mo7->f == mo1->f);
    test(!mo7->g);
    test(mo7->h == mo1->h);
    test(!mo7->i);
#ifdef ICE_CPP11_MAPPING
    test(targetEqualTo(mo7->j.value(), mo1->j.value()));
#else
    test(mo7->j == mo1->j);
#endif
    test(!mo7->k);
    test(mo7->bs == mo1->bs);
    test(!mo7->ss);
    test(mo7->iid == mo1->iid);
    test(!mo7->sid);
    test(mo7->fs == mo1->fs);
    test(!mo7->vs);

    test(mo7->shs == mo1->shs);
    test(!mo7->es);
    test(mo7->fss == mo1->fss);
    test(!mo7->vss);
    test(!mo7->oos->empty() && (*mo7->oos)[0]->a == oo1->a);
    test(!mo7->oops);

    test(!mo7->ied);
    test(mo7->ifsd == mo1->ifsd);
    test(!mo7->ivsd);
    test(!mo7->iood->empty() && (*mo7->iood)[5]->a == 15);
    test(!mo7->ioopd);

    // Clear the second half of the optional parameters
    MultiOptionalPtr mo8 = ICE_MAKE_SHARED(MultiOptional, *mo5);
#ifndef ICE_CPP11_MAPPING
    mo8->ice_collectable(true);
#endif
    mo8->b = IceUtil::None;
    mo8->d = IceUtil::None;
    mo8->f = IceUtil::None;
    mo8->h = IceUtil::None;
    mo8->j = IceUtil::None;
    mo8->bs = IceUtil::None;
    mo8->iid = IceUtil::None;
    mo8->fs = IceUtil::None;

    mo8->shs = IceUtil::None;
    mo8->fss = IceUtil::None;
    mo8->oos = IceUtil::None;

    mo8->ifsd = IceUtil::None;
    mo8->iood = IceUtil::None;

    mo8->k = mo8;
    MultiOptionalPtr mo9 = ICE_DYNAMIC_CAST(MultiOptional, initial->pingPong(mo8));
    test(mo9->a == mo1->a);
    test(!mo9->b);
    test(mo9->c == mo1->c);
    test(!mo9->d);
    test(mo9->e == mo1->e);
    test(!mo9->f);
    test(mo9->g == mo1->g);
    test(!mo9->h);
    test(mo9->i == mo1->i);
    test(!mo9->j);
    test(mo9->k == mo9);
    test(!mo9->bs);
    test(mo9->ss == mo1->ss);
    test(!mo9->iid);
    test(mo9->sid == mo1->sid);
    test(!mo9->fs);
    test(mo9->vs == mo1->vs);

    test(!mo8->shs);
    test(mo8->es == mo1->es);
    test(!mo8->fss);
    test(mo8->vss == mo1->vss);
    test(!mo8->oos);

#ifdef ICE_CPP11_MAPPING
    test(mo8->oops.value().size() == mo1->oops.value().size());
    for(size_t i = 0; i< mo8->oops.value().size(); ++i)
    {
        test(targetEqualTo(mo8->oops.value()[i], mo1->oops.value()[i]));
    }
#else
    test(mo8->oops == mo1->oops);
#endif

    test(mo8->ied == mo1->ied);
    test(!mo8->ifsd);
    test(mo8->ivsd == mo1->ivsd);
    test(!mo8->iood);

    Ice::ByteSeq inEncaps;
    Ice::ByteSeq outEncaps;

    //
    // Send a request using blobjects. Upon receival, we don't read
    // any of the optional members. This ensures the optional members
    // are skipped even if the receiver knows nothing about them.
    //
    {
        factory->setEnabled(true);
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(oo1);
        out.endEncapsulation();
        out.finished(inEncaps);
        test(initial->ice_invoke("pingPong", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps));
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        Ice::ValuePtr obj;
        in.read(obj);
        in.endEncapsulation();
        test(obj && dynamic_cast<TestObjectReader*>(obj.get()));
    }

    {
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(mo1);
        out.endEncapsulation();
        out.finished(inEncaps);
        test(initial->ice_invoke("pingPong", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps));
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        Ice::ValuePtr obj;
        in.read(obj);
        in.endEncapsulation();
        test(obj && dynamic_cast<TestObjectReader*>(obj.get()));
        factory->setEnabled(false);
    }

#ifdef ICE_CPP11_MAPPING
    mo1->k = shared_ptr<MultiOptional>();
    mo2->k = shared_ptr<MultiOptional>();
    mo3->k = shared_ptr<MultiOptional>();
    mo4->k = shared_ptr<MultiOptional>();
    mo5->k = shared_ptr<MultiOptional>();
    mo6->k = shared_ptr<MultiOptional>();
    mo7->k = shared_ptr<MultiOptional>();
    mo8->k = shared_ptr<MultiOptional>();
    mo9->k = shared_ptr<MultiOptional>();
#endif

    //
    // Use the 1.0 encoding with operations whose only class parameters are optional.
    //
    IceUtil::Optional<OneOptionalPtr> oo(ICE_MAKE_SHARED(OneOptional, 53));
    initial->sendOptionalClass(true, oo);
    initial->ice_encodingVersion(Ice::Encoding_1_0)->sendOptionalClass(true, oo);

    initial->returnOptionalClass(true, oo);
    test(oo);
    initial->ice_encodingVersion(Ice::Encoding_1_0)->returnOptionalClass(true, oo);
    test(!oo);

    RecursiveSeq recursive1;
    recursive1.push_back(ICE_MAKE_SHARED(Recursive));
    RecursiveSeq recursive2;
    recursive2.push_back(ICE_MAKE_SHARED(Recursive));
    recursive1[0]->value = recursive2;
    RecursivePtr outer = ICE_MAKE_SHARED(Recursive);
    outer->value = recursive1;
    initial->pingPong(outer);

    GPtr g = ICE_MAKE_SHARED(G);
    g->gg1Opt = ICE_MAKE_SHARED(G1, "gg1Opt");
    g->gg2 = ICE_MAKE_SHARED(G2, 10);
    g->gg2Opt = ICE_MAKE_SHARED(G2, 20);
    g->gg1 = ICE_MAKE_SHARED(G1, "gg1");
    GPtr r = initial->opG(g);
    test("gg1Opt" == r->gg1Opt.value()->a);
    test(10 == r->gg2->a);
    test(20 == r->gg2Opt.value()->a);
    test("gg1" == r->gg1->a);

    initial->opVoid();

    {
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(1, IceUtil::Optional<int>(15));
        out.write(2, IceUtil::Optional<string>("test"));
        out.endEncapsulation();
        out.finished(inEncaps);
        test(initial->ice_invoke("opVoid", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps));
    }

    cout << "ok" << endl;

    cout << "testing marshalling of large containers with fixed size elements..." << flush;
    MultiOptionalPtr mc = ICE_MAKE_SHARED(MultiOptional);

    ByteSeq byteSeq;
    byteSeq.resize(1000);
    mc->bs = byteSeq;

    ShortSeq shortSeq;
    shortSeq.resize(300);
    mc->shs = shortSeq;

    FixedStructSeq fsSeq;
    fsSeq.resize(300);
    mc->fss = fsSeq;

    IntFixedStructDict ifsd;
    for(int i = 0; i < 300; ++i)
    {
        ifsd.insert(make_pair(i, FixedStruct()));
    }
    mc->ifsd = ifsd;

    mc = ICE_DYNAMIC_CAST(MultiOptional, initial->pingPong(mc));
    test(mc->bs->size() == 1000);
    test(mc->shs->size() == 300);
    test(mc->fss->size() == 300);
    test(mc->ifsd->size() == 300);

    {
        factory->setEnabled(true);
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(mc);
        out.endEncapsulation();
        out.finished(inEncaps);
        test(initial->ice_invoke("pingPong", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps));
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        Ice::ValuePtr obj;
        in.read(obj);
        in.endEncapsulation();
        test(obj && dynamic_cast<TestObjectReader*>(obj.get()));
        factory->setEnabled(false);
    }

    cout << "ok" << endl;

    cout << "testing tag marshalling... " << flush;
    {
        BPtr b = ICE_MAKE_SHARED(B);
        BPtr b2 = ICE_DYNAMIC_CAST(B, initial->pingPong(b));
        test(!b2->ma);
        test(!b2->mb);
        test(!b2->mc);

        b->ma = 10;
        b->mb = 11;
        b->mc = 12;
        b->md = 13;

        b2 = ICE_DYNAMIC_CAST(B, initial->pingPong(b));
        test(b2->ma == 10);
        test(b2->mb == 11);
        test(b2->mc == 12);
        test(b2->md == 13);
        
        factory->setEnabled(true);
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(b);
        out.endEncapsulation();
        out.finished(inEncaps);
        test(initial->ice_invoke("pingPong", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps));
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        Ice::ValuePtr obj;
        in.read(obj);
        in.endEncapsulation();
        test(obj);
        factory->setEnabled(false);
    }

    cout << "ok" << endl;

    cout << "testing marshalling of objects with optional objects..." << flush;
    {
        FPtr f = ICE_MAKE_SHARED(F);

        f->af = ICE_MAKE_SHARED(A);
        f->ae = *f->af;

        FPtr rf = ICE_DYNAMIC_CAST(F, initial->pingPong(f));
        test(rf->ae == *rf->af);

        factory->setEnabled(true);
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(f);
        out.endEncapsulation();
        out.finished(inEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), inEncaps);
        in.startEncapsulation();
        Ice::ValuePtr obj;
        in.read(obj);
        in.endEncapsulation();
        factory->setEnabled(false);

        rf = dynamic_cast<FObjectReader*>(obj.get())->getF();
        test(rf->ae && !rf->af);
    }
    cout << "ok" << endl;

    cout << "testing optional with default values... " << flush;
    WDPtr wd = ICE_DYNAMIC_CAST(WD, initial->pingPong(ICE_MAKE_SHARED(WD)));
    test(*wd->a == 5);
    test(*wd->s == "test");
    wd->a = IceUtil::None;
    wd->s = IceUtil::None;
    wd = ICE_DYNAMIC_CAST(WD, initial->pingPong(wd));
    test(!wd->a);
    test(!wd->s);
    cout << "ok" << endl;

    if(communicator->getProperties()->getPropertyAsInt("Ice.Default.SlicedFormat") > 0)
    {
        cout << "testing marshalling with unknown class slices... " << flush;
        {
            CPtr c = ICE_MAKE_SHARED(C);
            c->ss = "test";
            c->ms = string("testms");

            {
                Ice::OutputStream out(communicator);
                out.startEncapsulation();
                out.write(c);
                out.endEncapsulation();
                out.finished(inEncaps);
                factory->setEnabled(true);
                test(initial->ice_invoke("pingPong", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps));
                Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
                in.startEncapsulation();
                Ice::ValuePtr obj;
                in.read(obj);
                in.endEncapsulation();
                test(dynamic_cast<CObjectReader*>(obj.get()));
                factory->setEnabled(false);
            }

            {
                factory->setEnabled(true);
                Ice::OutputStream out(communicator);
                out.startEncapsulation();
                Ice::ValuePtr d = ICE_MAKE_SHARED(DObjectWriter);
                out.write(d);
                out.endEncapsulation();
                out.finished(inEncaps);
                test(initial->ice_invoke("pingPong", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps));
                Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
                in.startEncapsulation();
                Ice::ValuePtr obj;
                in.read(obj);
                in.endEncapsulation();
                test(obj && dynamic_cast<DObjectReader*>(obj.get()));
                dynamic_cast<DObjectReader*>(obj.get())->check();
                factory->setEnabled(false);
            }
        }
        cout << "ok" << endl;

        cout << "testing optionals with unknown classes..." << flush;
        {
            APtr a = ICE_MAKE_SHARED(A);

            Ice::OutputStream out(communicator);
            out.startEncapsulation();
            out.write(a);
#ifdef ICE_CPP11_MAPPING
            out.write(1, Ice::make_optional(make_shared<DObjectWriter>()));
#else
            out.write(1, IceUtil::makeOptional(Ice::ObjectPtr(new DObjectWriter)));
#endif
            out.endEncapsulation();
            out.finished(inEncaps);
            test(initial->ice_invoke("opClassAndUnknownOptional", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps));

            Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
            in.startEncapsulation();
            in.endEncapsulation();
        }
        cout << "ok" << endl;
    }

    cout << "testing optional parameters... " << flush;
    {
        IceUtil::Optional<Ice::Byte> p1;
        IceUtil::Optional<Ice::Byte> p3;
        IceUtil::Optional<Ice::Byte> p2 = initial->opByte(p1, p3);
        test(!p2 && !p3);

        const Ice::Byte bval = 56;

        p1 = bval;
        p2 = initial->opByte(p1, p3);
        test(p2 == bval && p3 == bval);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opByte", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);

        IceUtil::Optional<Ice::Byte> p4 = 0x08;
        in.read(89, p4);

        in.endEncapsulation();
        test(p2 == bval && p3 == bval && !p4);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<bool> p1;
        IceUtil::Optional<bool> p3;
        IceUtil::Optional<bool> p2 = initial->opBool(p1, p3);
        test(!p2 && !p3);

        p1 = true;
        p2 = initial->opBool(p1, p3);
        test(*p2 == true && *p3 == true);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opBool", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(*p2 == true && *p3 == true);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<Ice::Short> p1;
        IceUtil::Optional<Ice::Short> p3;
        IceUtil::Optional<Ice::Short> p2 = initial->opShort(p1, p3);
        test(!p2 && !p3);

        const Ice::Short sval = 56;

        p1 = sval;
        p2 = initial->opShort(p1, p3);
        test(p2 == sval && p3 == sval);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opShort", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == sval && p3 == sval);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<Ice::Int> p1;
        IceUtil::Optional<Ice::Int> p3;
        IceUtil::Optional<Ice::Int> p2 = initial->opInt(p1, p3);
        test(!p2 && !p3);

        const Ice::Int ival = 56;

        p1 = ival;
        p2 = initial->opInt(p1, p3);
        test(p2 == 56 && p3 == 56);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opInt", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == ival && p3 == ival);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<Ice::Long> p1;
        IceUtil::Optional<Ice::Long> p3;
        IceUtil::Optional<Ice::Long> p2 = initial->opLong(p1, p3);
        test(!p2 && !p3);

        const Ice::Long lval = 56;

        p1 = lval;
        p2 = initial->opLong(p1, p3);
        test(p2 == lval && p3 == lval);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(1, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opLong", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(2, p3);
        in.read(3, p2);
        in.endEncapsulation();
        test(p2 == lval && p3 == lval);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<Ice::Float> p1;
        IceUtil::Optional<Ice::Float> p3;
        IceUtil::Optional<Ice::Float> p2 = initial->opFloat(p1, p3);
        test(!p2 && !p3);

        const Ice::Float fval = 1.0f;

        p1 = fval;
        p2 = initial->opFloat(p1, p3);
        test(p2 == fval && p3 == fval);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opFloat", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == fval && p3 == fval);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<Ice::Double> p1;
        IceUtil::Optional<Ice::Double> p3;
        IceUtil::Optional<Ice::Double> p2 = initial->opDouble(p1, p3);
        test(!p2 && !p3);

        const Ice::Double dval = 1.0;

        p1 = dval;
        p2 = initial->opDouble(p1, p3);
        test(p2 == dval && p3 == dval);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opDouble", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == dval && p3 == dval);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<string> p1;
        IceUtil::Optional<string> p3;
        IceUtil::Optional<string> p2 = initial->opString(p1, p3);
        test(!p2 && !p3);

        const string sval = "test";

        p1 = sval;
        p2 = initial->opString(sval, p3);
        test(p2 == sval && p3 == sval);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opString", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == sval && p3 == sval);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        if(supportsCppStringView)
        {
            IceUtil::Optional<Util::string_view> p1;
            IceUtil::Optional<string> p3;
            IceUtil::Optional<string> p2 = initial->opCustomString(p1, p3);
            test(!p2 && !p3);

            const string sval = "test";

            p1 = Util::string_view(sval);
            p2 = initial->opString(sval, p3);
            test(p2 == sval && p3 == sval);

            Ice::OutputStream out(communicator);
            out.startEncapsulation();
            out.write(2, p1);
            out.endEncapsulation();
            out.finished(inEncaps);
            initial->ice_invoke("opCustomString", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
            Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
            in.startEncapsulation();
            in.read(1, p2);
            in.read(3, p3);
            in.endEncapsulation();
            test(p2 == sval && p3 == sval);

            Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
            in2.startEncapsulation();
            in2.endEncapsulation();
        }
    }

    {
        IceUtil::Optional<Test::MyEnum> p1;
        IceUtil::Optional<Test::MyEnum> p3;
        IceUtil::Optional<Test::MyEnum> p2 = initial->opMyEnum(p1, p3);
        test(!p2 && !p3);

        p1 = ICE_ENUM(MyEnum, MyEnumMember);
        p2 = initial->opMyEnum(p1, p3);
        test(p2 == ICE_ENUM(MyEnum, MyEnumMember) && p3 == ICE_ENUM(MyEnum, MyEnumMember));

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opMyEnum", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == ICE_ENUM(MyEnum, MyEnumMember) && p3 == ICE_ENUM(MyEnum, MyEnumMember));

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<Test::SmallStruct> p1;
        IceUtil::Optional<Test::SmallStruct> p3;
        IceUtil::Optional<Test::SmallStruct> p2 = initial->opSmallStruct(p1, p3);
        test(!p2 && !p3);

        p1 = Test::SmallStruct();
        p1->m = 56;
        p2 = initial->opSmallStruct(p1, p3);
        test(p2->m == 56 && p3->m == 56);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opSmallStruct", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2->m == 56 && p3->m == 56);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<Test::FixedStruct> p1;
        IceUtil::Optional<Test::FixedStruct> p3;
        IceUtil::Optional<Test::FixedStruct> p2 = initial->opFixedStruct(p1, p3);
        test(!p2 && !p3);

        p1 = Test::FixedStruct();
        p1->m = 56;
        p2 = initial->opFixedStruct(p1, p3);
        test(p2->m == 56 && p3->m == 56);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opFixedStruct", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2->m == 56 && p3->m == 56);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<Test::VarStruct> p1;
        IceUtil::Optional<Test::VarStruct> p3;
        IceUtil::Optional<Test::VarStruct> p2 = initial->opVarStruct(p1, p3);
        test(!p2 && !p3);

        p1 = Test::VarStruct();
        p1->m = "test";
        p2 = initial->opVarStruct(p1, p3);
        test(p2->m == "test" && p3->m == "test");

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opVarStruct", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2->m == "test" && p3->m == "test");

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<OneOptionalPtr> p1;
        IceUtil::Optional<OneOptionalPtr> p3;
        IceUtil::Optional<OneOptionalPtr> p2 = initial->opOneOptional(p1, p3);
        test(!p2 && !p3);

        if(initial->supportsNullOptional())
        {
            p2 = initial->opOneOptional(OneOptionalPtr(), p3);
            test(*p2 == ICE_NULLPTR && *p3 == ICE_NULLPTR);
        }

        p1 = ICE_MAKE_SHARED(OneOptional, 58);
        p2 = initial->opOneOptional(p1, p3);
        test((*p2)->a == 58 && (*p3)->a == 58);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opOneOptional", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test((*p2)->a == 58 && (*p3)->a == 58);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<OneOptionalPrxPtr> p1;
        IceUtil::Optional<OneOptionalPrxPtr> p3;
        IceUtil::Optional<OneOptionalPrxPtr> p2 = initial->opOneOptionalProxy(p1, p3);
        test(!p2 && !p3);

        p1 = ICE_UNCHECKED_CAST(OneOptionalPrx, communicator->stringToProxy("test"));
        p2 = initial->opOneOptionalProxy(p1, p3);

#ifdef ICE_CPP11_MAPPING
        test(targetEqualTo(p2.value(), p1.value()) && targetEqualTo(p3.value(), p1.value()));
#else
        test(p2 == p1 && p3 == p1);
#endif

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opOneOptionalProxy", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();

#ifdef ICE_CPP11_MAPPING
        test(targetEqualTo(p2.value(), p1.value()) && targetEqualTo(p3.value(), p1.value()));
#else
        test(p2 == p1 && p3 == p1);
#endif

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        FPtr f = ICE_MAKE_SHARED(F);
        f->af = ICE_MAKE_SHARED(A);
        (*f->af)->requiredA = 56;
        f->ae = *f->af;

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
#ifdef ICE_CPP11_MAPPING
        out.write(1, Ice::make_optional(f));
        out.write(2, Ice::make_optional(f->ae));
#else
        out.write(1, makeOptional(f));
        out.write(2, makeOptional(f->ae));
#endif
        out.endEncapsulation();
        out.finished(inEncaps);

        Ice::InputStream in(communicator, out.getEncoding(), inEncaps);
        in.startEncapsulation();
        IceUtil::Optional<APtr> a;
        in.read(2, a);
        in.endEncapsulation();
        test(a && *a && (*a)->requiredA == 56);
    }
    cout << "ok" << endl;

    cout << "testing optional parameters and custom sequences... " << flush;
    {
        IceUtil::Optional<std::pair<const Ice::Byte*, const Ice::Byte*> > p1;
        IceUtil::Optional<ByteSeq> p3;
        IceUtil::Optional<ByteSeq> p2 = initial->opByteSeq(p1, p3);
        test(!p2 && !p3);

        vector<Ice::Byte> bs(100);
        fill(bs.begin(), bs.end(), 56);
        p1 = toArrayRange(bs);
        p2 = initial->opByteSeq(p1, p3);
        test(p2 && p3);
        test(p2 == bs && p3 == bs);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opByteSeq", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == bs && p3 == bs);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<std::pair<const bool*, const bool*> > p1;
        IceUtil::Optional<BoolSeq> p3;
        IceUtil::Optional<BoolSeq> p2 = initial->opBoolSeq(p1, p3);
        test(!p2 && !p3);

        bool bs[100];
        fill(&bs[0], &bs[0] + 100, true);
        vector<bool> bsv(&bs[0], &bs[0] + 100);
        p1 = toArrayRange(bs, 100);
        p2 = initial->opBoolSeq(p1, p3);
        test(p2 && p3);
        test(p2 == bsv && p3 == bsv);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opBoolSeq", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == bsv && p3 == bsv);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<std::pair<const Ice::Short*, const Ice::Short*> > p1;
        IceUtil::Optional<ShortSeq> p3;
        IceUtil::Optional<ShortSeq> p2 = initial->opShortSeq(p1, p3);
        test(!p2 && !p3);

        vector<Ice::Short> bs(100);
        fill(bs.begin(), bs.end(), 56);
        p1 = toArrayRange(bs);
        p2 = initial->opShortSeq(p1, p3);
        test(p2 && p3);
        test(p2 == bs && p3 == bs);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opShortSeq", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == bs && p3 == bs);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<std::pair<const Ice::Int*, const Ice::Int*> > p1;
        IceUtil::Optional<IntSeq> p3;
        IceUtil::Optional<IntSeq> p2 = initial->opIntSeq(p1, p3);
        test(!p2 && !p3);

        vector<Ice::Int> bs(100);
        fill(bs.begin(), bs.end(), 56);
        p1 = toArrayRange(bs);
        p2 = initial->opIntSeq(p1, p3);
        test(p2 && p3);
        test(p2 == bs && p3 == bs);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opIntSeq", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == bs && p3 == bs);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<std::pair<const Ice::Long*, const Ice::Long*> > p1;
        IceUtil::Optional<LongSeq> p3;
        IceUtil::Optional<LongSeq> p2 = initial->opLongSeq(p1, p3);
        test(!p2 && !p3);

        vector<Ice::Long> bs(100);
        fill(bs.begin(), bs.end(), 56);
        p1 = toArrayRange(bs);
        p2 = initial->opLongSeq(p1, p3);
        test(p2 && p3);
        test(p2 == bs && p3 == bs);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opLongSeq", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == bs && p3 == bs);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<std::pair<const Ice::Float*, const Ice::Float*> > p1;
        IceUtil::Optional<FloatSeq> p3;
        IceUtil::Optional<FloatSeq> p2 = initial->opFloatSeq(p1, p3);
        test(!p2 && !p3);

        vector<Ice::Float> bs(100);
        fill(bs.begin(), bs.end(), 1.0f);
        p1 = toArrayRange(bs);
        p2 = initial->opFloatSeq(p1, p3);
        test(p2 && p3);
        test(p2 == bs && p3 == bs);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opFloatSeq", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == bs && p3 == bs);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<std::pair<const Ice::Double*, const Ice::Double*> > p1;
        IceUtil::Optional<DoubleSeq> p3;
        IceUtil::Optional<DoubleSeq> p2 = initial->opDoubleSeq(p1, p3);
        test(!p2 && !p3);

        vector<Ice::Double> bs(100);
        fill(bs.begin(), bs.end(), 1.0);
        p1 = toArrayRange(bs);
        p2 = initial->opDoubleSeq(p1, p3);
        test(p2 && p3);
        test(p2 == bs && p3 == bs);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opDoubleSeq", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == bs && p3 == bs);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

#ifndef ICE_CPP11_MAPPING
    {
        IceUtil::Optional<std::pair<StringSeq::const_iterator, StringSeq::const_iterator> > p1;
        IceUtil::Optional<StringSeq> p3;
        IceUtil::Optional<StringSeq> p2 = initial->opStringSeq(p1, p3);
        test(!p2 && !p3);

        StringSeq ss(10);
        fill(ss.begin(), ss.end(), "test1");
#if defined(__SUNPRO_CC) && defined(_RWSTD_NO_MEMBER_TEMPLATES)
        std::pair<StringSeq::const_iterator, StringSeq::const_iterator> cpair(ss.begin(), ss.end());
        p1 = cpair;
#else
        p1 = make_pair(ss.begin(), ss.end());
#endif
        p2 = initial->opStringSeq(p1, p3);
        test(p2 && p3);
        test(p2 == ss && p3 == ss);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opStringSeq", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == ss && p3 == ss);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }
#endif

    {
        IceUtil::Optional<std::pair<const FixedStruct*, const FixedStruct*> > p1;
        IceUtil::Optional<FixedStructSeq> p3;
        IceUtil::Optional<FixedStructSeq> p2 = initial->opFixedStructSeq(p1, p3);
        test(!p2 && !p3);

        std::pair<const FixedStruct*, const FixedStruct*> p;
        p.first = p.second = 0;
        p2 = initial->opFixedStructSeq(p, p3);
        test(p2 && p3 && p2.value().empty() && p3.value().empty());

        FixedStruct fss[10];
        fss[0].m = 1;
        fss[1].m = 2;
        fss[2].m = 3;
        fss[3].m = 4;
        fss[4].m = 5;
        fss[5].m = 6;
        fss[6].m = 7;
        fss[7].m = 8;
        fss[8].m = 9;
        fss[9].m = 10;
        vector<FixedStruct> fssv(&fss[0], &fss[0] + 10);
        p1 = toArrayRange(fss, 10);
        p2 = initial->opFixedStructSeq(p1, p3);
        test(p2 && p3);
        test(p2 == fssv && p3 == fssv);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opFixedStructSeq", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == fssv && p3 == fssv);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

#ifndef ICE_CPP11_MAPPING
    {
        IceUtil::Optional<std::pair<VarStructSeq::const_iterator, VarStructSeq::const_iterator> > p1;
        IceUtil::Optional<VarStructSeq> p3;
        IceUtil::Optional<VarStructSeq> p2 = initial->opVarStructSeq(p1, p3);
        test(!p2 && !p3);

        VarStructSeq ss(10);
#if defined(__SUNPRO_CC) && defined(_RWSTD_NO_MEMBER_TEMPLATES)
        std::pair<VarStructSeq::const_iterator, VarStructSeq::const_iterator> cpair(ss.begin(), ss.end());
        p1 = cpair;
#else
        p1 = make_pair(ss.begin(), ss.end());
#endif
        p2 = initial->opVarStructSeq(p1, p3);
        test(p2 && p3);
        test(p2 == ss && p3 == ss);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opVarStructSeq", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == ss && p3 == ss);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }
#endif
    cout << "ok" << endl;

    cout << "testing optional parameters and dictionaries... " << flush;
    {
        IceUtil::Optional<IntIntDict> p1;
        IceUtil::Optional<IntIntDict> p3;
        IceUtil::Optional<IntIntDict> p2 = initial->opIntIntDict(p1, p3);
        test(!p2 && !p3);

        IntIntDict ss;
        ss.insert(make_pair(1, 1));
        p1 = ss;
        p2 = initial->opIntIntDict(p1, p3);
        test(p2 && p3);
        test(p2 == ss && p3 == ss);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opIntIntDict", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == ss && p3 == ss);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<StringIntDict> p1;
        IceUtil::Optional<StringIntDict> p3;
        IceUtil::Optional<StringIntDict> p2 = initial->opStringIntDict(p1, p3);
        test(!p2 && !p3);

        StringIntDict ss;
        ss.insert(make_pair<string, int>("test", 1));
        p1 = ss;
        p2 = initial->opStringIntDict(p1, p3);
        test(p2 && p3);
        test(p2 == ss && p3 == ss);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opStringIntDict", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test(p2 == ss && p3 == ss);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        IceUtil::Optional<IntOneOptionalDict> p1;
        IceUtil::Optional<IntOneOptionalDict> p3;
        IceUtil::Optional<IntOneOptionalDict> p2 = initial->opIntOneOptionalDict(p1, p3);
        test(!p2 && !p3);

        IntOneOptionalDict ss;
        ss.insert(make_pair<int, OneOptionalPtr>(1, ICE_MAKE_SHARED(OneOptional, 58)));
        p1 = ss;
        p2 = initial->opIntOneOptionalDict(p1, p3);
        test(p2 && p3);
        test((*p2)[1]->a == 58 && (*p3)[1]->a == 58);

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(2, p1);
        out.endEncapsulation();
        out.finished(inEncaps);
        initial->ice_invoke("opIntOneOptionalDict", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
        in.startEncapsulation();
        in.read(1, p2);
        in.read(3, p3);
        in.endEncapsulation();
        test((*p2)[1]->a == 58 && (*p3)[1]->a == 58);

        Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
        in2.startEncapsulation();
        in2.endEncapsulation();
    }

    {
        if(supportsCppStringView)
        {
            IceUtil::Optional<std::map<int, Util::string_view> > p1;
            IceUtil::Optional<IntStringDict> p3;
            IceUtil::Optional<IntStringDict> p2 = initial->opCustomIntStringDict(p1, p3);
            test(!p2 && !p3);

            map<int, Util::string_view> ss;
            ss.insert(make_pair<int, Util::string_view>(5, "testing"));
            p1 = ss;
            p2 = initial->opCustomIntStringDict(p1, p3);
            test(p2 && p3);
            test(p2 == p3);
            test(p2->size() == p1->size());
            test((*p2)[5] == ss[5].to_string());

            Ice::OutputStream out(communicator);
            out.startEncapsulation();
            out.write(2, p1);
            out.endEncapsulation();
            out.finished(inEncaps);
            initial->ice_invoke("opCustomIntStringDict", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
            Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
            in.startEncapsulation();
            in.read(1, p2);
            in.read(3, p3);
            in.endEncapsulation();
            test(p2 && p3);
            test(p2 == p3);
            test(p2->size() == p1->size());
            test((*p2)[5] == ss[5].to_string());

            Ice::InputStream in2(communicator, out.getEncoding(), outEncaps);
            in2.startEncapsulation();
            in2.endEncapsulation();
        }
    }

    cout << "ok" << endl;

    cout << "testing exception optionals... " << flush;
    {
        try
        {
            initial->opOptionalException(IceUtil::None, IceUtil::None, IceUtil::None);
            test(false);
        }
        catch(const OptionalException& ex)
        {
            test(!ex.a);
            test(!ex.b);
            test(!ex.o);
        }

        try
        {
            initial->opOptionalException(30, string("test"), ICE_MAKE_SHARED(OneOptional, 53));
            test(false);
        }
        catch(const OptionalException& ex)
        {
            test(ex.a == 30);
            test(ex.b == string("test"));
            test((*ex.o)->a = 53);
        }

        try
        {
            //
            // Use the 1.0 encoding with an exception whose only class members are optional.
            //
            initial->ice_encodingVersion(Ice::Encoding_1_0)->
                opOptionalException(30, string("test"), ICE_MAKE_SHARED(OneOptional, 53));
            test(false);
        }
        catch(const OptionalException& ex)
        {
            test(!ex.a);
            test(!ex.b);
            test(!ex.o);
        }

        try
        {
            IceUtil::Optional<Ice::Int> a;
            IceUtil::Optional<string> b;
            IceUtil::Optional<OneOptionalPtr> o;
            initial->opDerivedException(a, b, o);
            test(false);
        }
        catch(const DerivedException& ex)
        {
            test(!ex.a);
            test(!ex.b);
            test(!ex.o);
            test(!ex.ss);
            test(!ex.o2);
        }
        catch(const OptionalException&)
        {
            test(false);
        }

        try
        {
            IceUtil::Optional<Ice::Int> a = 30;
            IceUtil::Optional<string> b = string("test2");
            IceUtil::Optional<OneOptionalPtr> o = ICE_MAKE_SHARED(OneOptional, 53);
            initial->opDerivedException(a, b, o);
            test(false);
        }
        catch(const DerivedException& ex)
        {
            test(ex.a == 30);
            test(ex.b == string("test2"));
            test((*ex.o)->a == 53);
            test(ex.ss == string("test2"));
            test((*ex.o2)->a == 53);
        }
        catch(const OptionalException&)
        {
            test(false);
        }

        try
        {
            IceUtil::Optional<Ice::Int> a;
            IceUtil::Optional<string> b;
            IceUtil::Optional<OneOptionalPtr> o;
            initial->opRequiredException(a, b, o);
            test(false);
        }
        catch(const RequiredException& ex)
        {
            test(!ex.a);
            test(!ex.b);
            test(!ex.o);
            test(ex.ss == string("test"));
            test(!ex.o2);
        }
        catch(const OptionalException&)
        {
            test(false);
        }

        try
        {
            IceUtil::Optional<Ice::Int> a = 30;
            IceUtil::Optional<string> b = string("test2");
            IceUtil::Optional<OneOptionalPtr> o = ICE_MAKE_SHARED(OneOptional, 53);
            initial->opRequiredException(a, b, o);
            test(false);
        }
        catch(const RequiredException& ex)
        {
            test(ex.a == 30);
            test(ex.b == string("test2"));
            test((*ex.o)->a == 53);
            test(ex.ss == string("test2"));
            test(ex.o2->a == 53);
        }
        catch(const OptionalException&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "testing optionals with marshaled results... " << flush;
    {
        test(initial->opMStruct1());
        test(initial->opMDict1());
        test(initial->opMSeq1());
        test(initial->opMG1());

        {
            IceUtil::Optional<Test::SmallStruct> p1, p2, p3;
            p3 = initial->opMStruct2(IceUtil::None, p2);
            test(!p2 && !p3);

            p1 = Test::SmallStruct();
            p3 = initial->opMStruct2(p1, p2);
            test(p2 == p1 && p3 == p1);
        }
        {
            IceUtil::Optional<Test::StringSeq> p1, p2, p3;
            p3 = initial->opMSeq2(IceUtil::None, p2);
            test(!p2 && !p3);

            Test::StringSeq seq;
            seq.push_back("hello");
            p1 = seq;
            p3 = initial->opMSeq2(p1, p2);
            test(p2 == p1 && p3 == p1);
        }
        {
            IceUtil::Optional<Test::StringIntDict> p1, p2, p3;
            p3 = initial->opMDict2(IceUtil::None, p2);
            test(!p2 && !p3);

            Test::StringIntDict dict;
            dict["test"] = 54;
            p1 = dict;
            p3 = initial->opMDict2(p1, p2);
            test(p2 == p1 && p3 == p1);
        }
        {
            IceUtil::Optional<Test::GPtr> p1, p2, p3;
            p3 = initial->opMG2(IceUtil::None, p2);
            test(!p2 && !p3);

            p1 = ICE_MAKE_SHARED(Test::G);
            p3 = initial->opMG2(p1, p2);
            test(p2 && p3 && *p3 == *p2);
        }
    }
    cout << "ok" << endl;
    return initial;
}
