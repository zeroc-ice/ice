// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

//
// Work-around for GCC warning bug
//
#if defined(__GNUC__)
#    pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

//
// Disable VC++ warning
// 4503: decorated name length exceeded, name was truncated
//
#if defined(_MSC_VER)
#    pragma warning(disable : 4503)
#endif

using namespace std;
using namespace Test;
using namespace Ice;

namespace
{
    class CallbackBase
    {
    public:
        CallbackBase() = default;

        virtual ~CallbackBase() = default;

        void check()
        {
            unique_lock lock(_mutex);
            _condition.wait(lock, [this] { return _called; });
            _called = false;
        }

    protected:
        void called()
        {
            lock_guard lock(_mutex);
            assert(!_called);
            _called = true;
            _condition.notify_one();
        }

    private:
        mutex _mutex;
        condition_variable _condition;
        bool _called{false};
    };

    class Callback : public CallbackBase
    {
    public:
        Callback() = default;

        Callback(CommunicatorPtr communicator) : _communicator(std::move(communicator)) {}

        void ping() { called(); }

        void isA(bool result)
        {
            test(result);
            called();
        }

        void id(const string& id)
        {
            test(id == MyDerivedClass::ice_staticId());
            called();
        }

        void ids(const StringSeq& ids)
        {
            test(ids.size() == 3);
            test(ids[0] == "::Ice::Object");
            test(ids[1] == "::Test::MyClass");
            test(ids[2] == "::Test::MyDerivedClass");
            called();
        }

        void opVoid() { called(); }

        void opContext(const Context&) { called(); }

        void opByte(uint8_t r, uint8_t b)
        {
            test(b == uint8_t(0xf0));
            test(r == uint8_t(0xff));
            called();
        }

        void opBool(bool r, bool b)
        {
            test(b);
            test(!r);
            called();
        }

        void opShortIntLong(int64_t r, int16_t s, int32_t i, int64_t l)
        {
            test(s == 10);
            test(i == 11);
            test(l == 12);
            test(r == 12);
            called();
        }

        void opFloatDouble(double r, float f, double d)
        {
            test(f == float(3.14));
            test(d == double(1.1E10));
            test(r == double(1.1E10));
            called();
        }

        void opString(const string& r, const string& s)
        {
            test(s == "world hello");
            test(r == "hello world");
            called();
        }

        void opMyEnum(MyEnum r, MyEnum e)
        {
            test(e == MyEnum::enum2);
            test(r == MyEnum::enum3);
            called();
        }

        void opMyClass(const MyClassPrx& r, const MyClassPrx& c1, const MyClassPrx& c2)
        {
            test(c1->ice_getIdentity() == stringToIdentity("test"));
            test(c2->ice_getIdentity() == stringToIdentity("noSuchIdentity"));
            test(r->ice_getIdentity() == stringToIdentity("test"));

            //
            // We can't do the callbacks below in connection serialization mode.
            //
            if (_communicator->getProperties()->getIcePropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0)
            {
                r->opVoid();
                c1->opVoid();
                try
                {
                    c2->opVoid();
                    test(false);
                }
                catch (const ObjectNotExistException&)
                {
                }
            }
            called();
        }

        void opStruct(const Structure& rso, const Structure& so)
        {
            test(rso.p == nullopt);
            test(rso.e == MyEnum::enum2);
            test(rso.s.s == "def");
            test(so.e == MyEnum::enum3);
            test(so.s.s == "a new string");

            //
            // We can't do the callbacks below in connection serialization mode.
            //
            if (_communicator->getProperties()->getIcePropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0)
            {
                so.p->opVoid();
            }
            called();
        }

        void opByteS(const ByteS& rso, const ByteS& bso)
        {
            test(bso.size() == 4);
            test(bso[0] == byte{0x22});
            test(bso[1] == byte{0x12});
            test(bso[2] == byte{0x11});
            test(bso[3] == byte{0x01});
            test(rso.size() == 8);
            test(rso[0] == byte{0x01});
            test(rso[1] == byte{0x11});
            test(rso[2] == byte{0x12});
            test(rso[3] == byte{0x22});
            test(rso[4] == byte{0xf1});
            test(rso[5] == byte{0xf2});
            test(rso[6] == byte{0xf3});
            test(rso[7] == byte{0xf4});
            called();
        }

        void opBoolS(const BoolS& rso, const BoolS& bso)
        {
            test(bso.size() == 4);
            test(bso[0]);
            test(bso[1]);
            test(!bso[2]);
            test(!bso[3]);
            test(rso.size() == 3);
            test(!rso[0]);
            test(rso[1]);
            test(rso[2]);
            called();
        }

        void opShortIntLongS(const LongS& rso, const ShortS& sso, const IntS& iso, const LongS& lso)
        {
            test(sso.size() == 3);
            test(sso[0] == 1);
            test(sso[1] == 2);
            test(sso[2] == 3);
            test(iso.size() == 4);
            test(iso[0] == 8);
            test(iso[1] == 7);
            test(iso[2] == 6);
            test(iso[3] == 5);
            test(lso.size() == 6);
            test(lso[0] == 10);
            test(lso[1] == 30);
            test(lso[2] == 20);
            test(lso[3] == 10);
            test(lso[4] == 30);
            test(lso[5] == 20);
            test(rso.size() == 3);
            test(rso[0] == 10);
            test(rso[1] == 30);
            test(rso[2] == 20);
            called();
        }

        void opFloatDoubleS(const DoubleS& rso, const FloatS& fso, const DoubleS& dso)
        {
            test(fso.size() == 2);
            test(fso[0] == float(3.14));
            test(fso[1] == float(1.11));
            test(dso.size() == 3);
            test(dso[0] == double(1.3E10));
            test(dso[1] == double(1.2E10));
            test(dso[2] == double(1.1E10));
            test(rso.size() == 5);
            test(rso[0] == double(1.1E10));
            test(rso[1] == double(1.2E10));
            test(rso[2] == double(1.3E10));
            test(float(rso[3]) == float(3.14));
            test(float(rso[4]) == float(1.11));
            called();
        }

        void opStringS(const StringS& rso, const StringS& sso)
        {
            test(sso.size() == 4);
            test(sso[0] == "abc");
            test(sso[1] == "de");
            test(sso[2] == "fghi");
            test(sso[3] == "xyz");
            test(rso.size() == 3);
            test(rso[0] == "fghi");
            test(rso[1] == "de");
            test(rso[2] == "abc");
            called();
        }

        void opByteSS(const ByteSS& rso, const ByteSS& bso)
        {
            test(bso.size() == 2);
            test(bso[0].size() == 1);
            test(bso[0][0] == byte{0xff});
            test(bso[1].size() == 3);
            test(bso[1][0] == byte{0x01});
            test(bso[1][1] == byte{0x11});
            test(bso[1][2] == byte{0x12});
            test(rso.size() == 4);
            test(rso[0].size() == 3);
            test(rso[0][0] == byte{0x01});
            test(rso[0][1] == byte{0x11});
            test(rso[0][2] == byte{0x12});
            test(rso[1].size() == 1);
            test(rso[1][0] == byte{0xff});
            test(rso[2].size() == 1);
            test(rso[2][0] == byte{0x0e});
            test(rso[3].size() == 2);
            test(rso[3][0] == byte{0xf2});
            test(rso[3][1] == byte{0xf1});
            called();
        }

        void opBoolSS(const BoolSS& rso, const BoolSS& bso)
        {
            test(bso.size() == 4);
            test(bso[0].size() == 1);
            test(bso[0][0]);
            test(bso[1].size() == 1);
            test(!bso[1][0]);
            test(bso[2].size() == 2);
            test(bso[2][0]);
            test(bso[2][1]);
            test(bso[3].size() == 3);
            test(!bso[3][0]);
            test(!bso[3][1]);
            test(bso[3][2]);
            test(rso.size() == 3);
            test(rso[0].size() == 2);
            test(rso[0][0]);
            test(rso[0][1]);
            test(rso[1].size() == 1);
            test(!rso[1][0]);
            test(rso[2].size() == 1);
            test(rso[2][0]);
            called();
        }

        void opShortIntLongSS(const LongSS& rso, const ShortSS& sso, const IntSS& iso, const LongSS& lso)
        {
            test(rso.size() == 1);
            test(rso[0].size() == 2);
            test(rso[0][0] == 496);
            test(rso[0][1] == 1729);
            test(sso.size() == 3);
            test(sso[0].size() == 3);
            test(sso[0][0] == 1);
            test(sso[0][1] == 2);
            test(sso[0][2] == 5);
            test(sso[1].size() == 1);
            test(sso[1][0] == 13);
            test(sso[2].size() == 0);
            test(iso.size() == 2);
            test(iso[0].size() == 1);
            test(iso[0][0] == 42);
            test(iso[1].size() == 2);
            test(iso[1][0] == 24);
            test(iso[1][1] == 98);
            test(lso.size() == 2);
            test(lso[0].size() == 2);
            test(lso[0][0] == 496);
            test(lso[0][1] == 1729);
            test(lso[1].size() == 2);
            test(lso[1][0] == 496);
            test(lso[1][1] == 1729);
            called();
        }

        void opFloatDoubleSS(const DoubleSS& rso, const FloatSS& fso, const DoubleSS& dso)
        {
            test(fso.size() == 3);
            test(fso[0].size() == 1);
            test(fso[0][0] == float(3.14));
            test(fso[1].size() == 1);
            test(fso[1][0] == float(1.11));
            test(fso[2].size() == 0);
            test(dso.size() == 1);
            test(dso[0].size() == 3);
            test(dso[0][0] == double(1.1E10));
            test(dso[0][1] == double(1.2E10));
            test(dso[0][2] == double(1.3E10));
            test(rso.size() == 2);
            test(rso[0].size() == 3);
            test(rso[0][0] == double(1.1E10));
            test(rso[0][1] == double(1.2E10));
            test(rso[0][2] == double(1.3E10));
            test(rso[1].size() == 3);
            test(rso[1][0] == double(1.1E10));
            test(rso[1][1] == double(1.2E10));
            test(rso[1][2] == double(1.3E10));
            called();
        }

        void opStringSS(const StringSS& rso, const StringSS& sso)
        {
            test(sso.size() == 5);
            test(sso[0].size() == 1);
            test(sso[0][0] == "abc");
            test(sso[1].size() == 2);
            test(sso[1][0] == "de");
            test(sso[1][1] == "fghi");
            test(sso[2].size() == 0);
            test(sso[3].size() == 0);
            test(sso[4].size() == 1);
            test(sso[4][0] == "xyz");
            test(rso.size() == 3);
            test(rso[0].size() == 1);
            test(rso[0][0] == "xyz");
            test(rso[1].size() == 0);
            test(rso[2].size() == 0);
            called();
        }

        void opByteBoolD(const ByteBoolD& ro, const ByteBoolD& _do)
        {
            ByteBoolD di1;
            di1[10] = true;
            di1[100] = false;
            test(_do == di1);
            test(ro.size() == 4);
            test(ro.find(10) != ro.end());
            test(ro.find(10)->second == true);
            test(ro.find(11) != ro.end());
            test(ro.find(11)->second == false);
            test(ro.find(100) != ro.end());
            test(ro.find(100)->second == false);
            test(ro.find(101) != ro.end());
            test(ro.find(101)->second == true);
            called();
        }

        void opShortIntD(const ShortIntD& ro, const ShortIntD& _do)
        {
            ShortIntD di1;
            di1[110] = -1;
            di1[1100] = 123123;
            test(_do == di1);
            test(ro.size() == 4);
            test(ro.find(110) != ro.end());
            test(ro.find(110)->second == -1);
            test(ro.find(111) != ro.end());
            test(ro.find(111)->second == -100);
            test(ro.find(1100) != ro.end());
            test(ro.find(1100)->second == 123123);
            test(ro.find(1101) != ro.end());
            test(ro.find(1101)->second == 0);
            called();
        }

        void opLongFloatD(const LongFloatD& ro, const LongFloatD& _do)
        {
            LongFloatD di1;
            di1[999999110] = float(-1.1);
            di1[999999111] = float(123123.2);
            test(_do == di1);
            test(ro.size() == 4);
            test(ro.find(999999110) != ro.end());
            test(ro.find(999999110)->second == float(-1.1));
            test(ro.find(999999120) != ro.end());
            test(ro.find(999999120)->second == float(-100.4));
            test(ro.find(999999111) != ro.end());
            test(ro.find(999999111)->second == float(123123.2));
            test(ro.find(999999130) != ro.end());
            test(ro.find(999999130)->second == float(0.5));
            called();
        }

        void opStringStringD(const StringStringD& ro, const StringStringD& _do)
        {
            StringStringD di1;
            di1["foo"] = "abc -1.1";
            di1["bar"] = "abc 123123.2";
            test(_do == di1);
            test(ro.size() == 4);
            test(ro.find("foo") != ro.end());
            test(ro.find("foo")->second == "abc -1.1");
            test(ro.find("FOO") != ro.end());
            test(ro.find("FOO")->second == "abc -100.4");
            test(ro.find("bar") != ro.end());
            test(ro.find("bar")->second == "abc 123123.2");
            test(ro.find("BAR") != ro.end());
            test(ro.find("BAR")->second == "abc 0.5");
            called();
        }

        void opStringMyEnumD(const StringMyEnumD& ro, const StringMyEnumD& _do)
        {
            StringMyEnumD di1;
            di1["abc"] = MyEnum::enum1;
            di1[""] = MyEnum::enum2;
            test(_do == di1);
            test(ro.size() == 4);
            test(ro.find("abc") != ro.end());
            test(ro.find("abc")->second == MyEnum::enum1);
            test(ro.find("qwerty") != ro.end());
            test(ro.find("qwerty")->second == MyEnum::enum3);
            test(ro.find("") != ro.end());
            test(ro.find("")->second == MyEnum::enum2);
            test(ro.find("Hello!!") != ro.end());
            test(ro.find("Hello!!")->second == MyEnum::enum2);
            called();
        }

        void opMyStructMyEnumD(const MyStructMyEnumD& ro, const MyStructMyEnumD& _do)
        {
            MyStruct ms11 = {1, 1};
            MyStruct ms12 = {1, 2};
            MyStructMyEnumD di1;
            di1[ms11] = MyEnum::enum1;
            di1[ms12] = MyEnum::enum2;
            test(_do == di1);
            MyStruct ms22 = {2, 2};
            MyStruct ms23 = {2, 3};
            test(ro.size() == 4);
            test(ro.find(ms11) != ro.end());
            test(ro.find(ms11)->second == MyEnum::enum1);
            test(ro.find(ms12) != ro.end());
            test(ro.find(ms12)->second == MyEnum::enum2);
            test(ro.find(ms22) != ro.end());
            test(ro.find(ms22)->second == MyEnum::enum3);
            test(ro.find(ms23) != ro.end());
            test(ro.find(ms23)->second == MyEnum::enum2);
            called();
        }

        void opByteBoolDS(const ByteBoolDS& ro, const ByteBoolDS& _do)
        {
            test(ro.size() == 2);
            test(ro[0].size() == 3);
            test(ro[0].find(10) != ro[0].end());
            test(ro[0].find(10)->second == true);
            test(ro[0].find(11) != ro[0].end());
            test(ro[0].find(11)->second == false);
            test(ro[0].find(101) != ro[0].end());
            test(ro[0].find(101)->second == true);
            test(ro[1].size() == 2);
            test(ro[1].find(10) != ro[1].end());
            test(ro[1].find(10)->second == true);
            test(ro[1].find(100) != ro[1].end());
            test(ro[1].find(100)->second == false);
            test(_do.size() == 3);
            test(_do[0].size() == 2);
            test(_do[0].find(100) != _do[0].end());
            test(_do[0].find(100)->second == false);
            test(_do[0].find(101) != _do[0].end());
            test(_do[0].find(101)->second == false);
            test(_do[1].size() == 2);
            test(_do[1].find(10) != _do[1].end());
            test(_do[1].find(10)->second == true);
            test(_do[1].find(100) != _do[1].end());
            test(_do[1].find(100)->second == false);
            test(_do[2].size() == 3);
            test(_do[2].find(10) != _do[2].end());
            test(_do[2].find(10)->second == true);
            test(_do[2].find(11) != _do[2].end());
            test(_do[2].find(11)->second == false);
            test(_do[2].find(101) != _do[2].end());
            test(_do[2].find(101)->second == true);
            called();
        }

        void opShortIntDS(const ShortIntDS& ro, const ShortIntDS& _do)
        {
            test(ro.size() == 2);
            test(ro[0].size() == 3);
            test(ro[0].find(110) != ro[0].end());
            test(ro[0].find(110)->second == -1);
            test(ro[0].find(111) != ro[0].end());
            test(ro[0].find(111)->second == -100);
            test(ro[0].find(1101) != ro[0].end());
            test(ro[0].find(1101)->second == 0);
            test(ro[1].size() == 2);
            test(ro[1].find(110)->second == -1);
            test(ro[1].find(1100)->second == 123123);
            test(_do.size() == 3);
            test(_do[0].size() == 1);
            test(_do[0].find(100) != _do[0].end());
            test(_do[0].find(100)->second == -1001);
            test(_do[1].size() == 2);
            test(_do[1].find(110) != _do[1].end());
            test(_do[1].find(110)->second == -1);
            test(_do[1].find(1100) != _do[1].end());
            test(_do[1].find(1100)->second == 123123);
            test(_do[2].size() == 3);
            test(_do[2].find(110) != _do[2].end());
            test(_do[2].find(110)->second == -1);
            test(_do[2].find(111) != _do[2].end());
            test(_do[2].find(111)->second == -100);
            test(_do[2].find(1101) != _do[2].end());
            test(_do[2].find(1101)->second == 0);
            called();
        }

        void opLongFloatDS(const LongFloatDS& ro, const LongFloatDS& _do)
        {
            test(ro.size() == 2);
            test(ro[0].size() == 3);
            test(ro[0].find(999999110) != ro[0].end());
            test(ro[0].find(999999110)->second == float(-1.1));
            test(ro[0].find(999999120) != ro[0].end());
            test(ro[0].find(999999120)->second == float(-100.4));
            test(ro[0].find(999999130) != ro[0].end());
            test(ro[0].find(999999130)->second == float(0.5));
            test(ro[1].size() == 2);
            test(ro[1].find(999999110) != ro[1].end());
            test(ro[1].find(999999110)->second == float(-1.1));
            test(ro[1].find(999999111) != ro[1].end());
            test(ro[1].find(999999111)->second == float(123123.2));
            test(_do.size() == 3);
            test(_do[0].size() == 1);
            test(_do[0].find(999999140) != _do[0].end());
            test(_do[0].find(999999140)->second == float(3.14));
            test(_do[1].size() == 2);
            test(_do[1].find(999999110) != _do[1].end());
            test(_do[1].find(999999110)->second == float(-1.1));
            test(_do[1].find(999999111) != _do[1].end());
            test(_do[1].find(999999111)->second == float(123123.2));
            test(_do[2].size() == 3);
            test(_do[2].find(999999110) != _do[2].end());
            test(_do[2].find(999999110)->second == float(-1.1));
            test(_do[2].find(999999120) != _do[2].end());
            test(_do[2].find(999999120)->second == float(-100.4));
            test(_do[2].find(999999130) != _do[2].end());
            test(_do[2].find(999999130)->second == float(0.5));
            called();
        }

        void opStringStringDS(const StringStringDS& ro, const StringStringDS& _do)
        {
            test(ro.size() == 2);
            test(ro[0].size() == 3);
            test(ro[0].find("foo") != ro[0].end());
            test(ro[0].find("foo")->second == "abc -1.1");
            test(ro[0].find("FOO") != ro[0].end());
            test(ro[0].find("FOO")->second == "abc -100.4");
            test(ro[0].find("BAR") != ro[0].end());
            test(ro[0].find("BAR")->second == "abc 0.5");
            test(ro[1].size() == 2);
            test(ro[1].find("foo") != ro[1].end());
            test(ro[1].find("foo")->second == "abc -1.1");
            test(ro[1].find("bar") != ro[1].end());
            test(ro[1].find("bar")->second == "abc 123123.2");
            test(_do.size() == 3);
            test(_do[0].size() == 1);
            test(_do[0].find("f00") != _do[0].end());
            test(_do[0].find("f00")->second == "ABC -3.14");
            test(_do[1].size() == 2);
            test(_do[1].find("foo") != _do[1].end());
            test(_do[1].find("foo")->second == "abc -1.1");
            test(_do[1].find("bar") != _do[1].end());
            test(_do[1].find("bar")->second == "abc 123123.2");
            test(_do[2].size() == 3);
            test(_do[2].find("foo") != _do[2].end());
            test(_do[2].find("foo")->second == "abc -1.1");
            test(_do[2].find("FOO") != _do[2].end());
            test(_do[2].find("FOO")->second == "abc -100.4");
            test(_do[2].find("BAR") != _do[2].end());
            test(_do[2].find("BAR")->second == "abc 0.5");
            called();
        }

        void opStringMyEnumDS(const StringMyEnumDS& ro, const StringMyEnumDS& _do)
        {
            test(ro.size() == 2);
            test(ro[0].size() == 3);
            test(ro[0].find("abc") != ro[0].end());
            test(ro[0].find("abc")->second == MyEnum::enum1);
            test(ro[0].find("qwerty") != ro[0].end());
            test(ro[0].find("qwerty")->second == MyEnum::enum3);
            test(ro[0].find("Hello!!") != ro[0].end());
            test(ro[0].find("Hello!!")->second == MyEnum::enum2);
            test(ro[1].size() == 2);
            test(ro[1].find("abc") != ro[1].end());
            test(ro[1].find("abc")->second == MyEnum::enum1);
            test(ro[1].find("") != ro[1].end());
            test(ro[1].find("")->second == MyEnum::enum2);
            test(_do.size() == 3);
            test(_do[0].size() == 1);
            test(_do[0].find("Goodbye") != _do[0].end());
            test(_do[0].find("Goodbye")->second == MyEnum::enum1);
            test(_do[1].size() == 2);
            test(_do[1].find("abc") != _do[1].end());
            test(_do[1].find("abc")->second == MyEnum::enum1);
            test(_do[1].find("") != _do[1].end());
            test(_do[1].find("")->second == MyEnum::enum2);
            test(_do[2].size() == 3);
            test(_do[2].find("abc") != _do[2].end());
            test(_do[2].find("abc")->second == MyEnum::enum1);
            test(_do[2].find("qwerty") != _do[2].end());
            test(_do[2].find("qwerty")->second == MyEnum::enum3);
            test(_do[2].find("Hello!!") != _do[2].end());
            test(_do[2].find("Hello!!")->second == MyEnum::enum2);
            called();
        }

        void opMyEnumStringDS(const MyEnumStringDS& ro, const MyEnumStringDS& _do)
        {
            test(ro.size() == 2);
            test(ro[0].size() == 2);
            test(ro[0].find(MyEnum::enum2) != ro[0].end());
            test(ro[0].find(MyEnum::enum2)->second == "Hello!!");
            test(ro[0].find(MyEnum::enum3) != ro[0].end());
            test(ro[0].find(MyEnum::enum3)->second == "qwerty");
            test(ro[1].size() == 1);
            test(ro[1].find(MyEnum::enum1) != ro[1].end());
            test(ro[1].find(MyEnum::enum1)->second == "abc");
            test(_do.size() == 3);
            test(_do[0].size() == 1);
            test(_do[0].find(MyEnum::enum1) != _do[0].end());
            test(_do[0].find(MyEnum::enum1)->second == "Goodbye");
            test(_do[1].size() == 1);
            test(_do[1].find(MyEnum::enum1) != _do[1].end());
            test(_do[1].find(MyEnum::enum1)->second == "abc");
            test(_do[2].size() == 2);
            test(_do[2].find(MyEnum::enum2) != _do[2].end());
            test(_do[2].find(MyEnum::enum2)->second == "Hello!!");
            test(_do[2].find(MyEnum::enum3) != _do[2].end());
            test(_do[2].find(MyEnum::enum3)->second == "qwerty");
            called();
        }

        void opMyStructMyEnumDS(const MyStructMyEnumDS& ro, const MyStructMyEnumDS& _do)
        {
            MyStruct ms11 = {1, 1};
            MyStruct ms12 = {1, 2};
            MyStruct ms22 = {2, 2};
            MyStruct ms23 = {2, 3};

            test(ro.size() == 2);
            test(ro[0].size() == 3);
            test(ro[0].find(ms11) != ro[0].end());
            test(ro[0].find(ms11)->second == MyEnum::enum1);
            test(ro[0].find(ms22) != ro[0].end());
            test(ro[0].find(ms22)->second == MyEnum::enum3);
            test(ro[0].find(ms23) != ro[0].end());
            test(ro[0].find(ms23)->second == MyEnum::enum2);
            test(ro[1].size() == 2);
            test(ro[1].find(ms11) != ro[1].end());
            test(ro[1].find(ms11)->second == MyEnum::enum1);
            test(ro[1].find(ms12) != ro[1].end());
            test(ro[1].find(ms12)->second == MyEnum::enum2);
            test(_do.size() == 3);
            test(_do[0].size() == 1);
            test(_do[0].find(ms23) != _do[0].end());
            test(_do[0].find(ms23)->second == MyEnum::enum3);
            test(_do[1].size() == 2);
            test(_do[1].find(ms11) != _do[1].end());
            test(_do[1].find(ms11)->second == MyEnum::enum1);
            test(_do[1].find(ms12) != _do[1].end());
            test(_do[1].find(ms12)->second == MyEnum::enum2);
            test(_do[2].size() == 3);
            test(_do[2].find(ms11) != _do[2].end());
            test(_do[2].find(ms11)->second == MyEnum::enum1);
            test(_do[2].find(ms22) != _do[2].end());
            test(_do[2].find(ms22)->second == MyEnum::enum3);
            test(_do[2].find(ms23) != _do[2].end());
            test(_do[2].find(ms23)->second == MyEnum::enum2);
            called();
        }

        void opByteByteSD(const ByteByteSD& ro, const ByteByteSD& _do)
        {
            test(_do.size() == 1);
            test(_do.find(uint8_t{0xf1}) != _do.end());
            test(_do.find(uint8_t{0xf1})->second.size() == 2);
            test(_do.find(uint8_t{0xf1})->second[0] == byte{0xf2});
            test(_do.find(uint8_t{0xf1})->second[1] == byte{0xf3});
            test(ro.size() == 3);
            test(ro.find(uint8_t{0x01}) != ro.end());
            test(ro.find(uint8_t{0x01})->second.size() == 2);
            test(ro.find(uint8_t{0x01})->second[0] == byte{0x01});
            test(ro.find(uint8_t{0x01})->second[1] == byte{0x11});
            test(ro.find(uint8_t{0x22}) != ro.end());
            test(ro.find(uint8_t{0x22})->second.size() == 1);
            test(ro.find(uint8_t{0x22})->second[0] == byte{0x12});
            test(ro.find(uint8_t{0xf1}) != ro.end());
            test(ro.find(uint8_t{0xf1})->second.size() == 2);
            test(ro.find(uint8_t{0xf1})->second[0] == byte{0xf2});
            test(ro.find(uint8_t{0xf1})->second[1] == byte{0xf3});
            called();
        }

        void opBoolBoolSD(const BoolBoolSD& ro, const BoolBoolSD& _do)
        {
            test(_do.size() == 1);
            test(_do.find(false) != _do.end());
            test(_do.find(false)->second.size() == 2);
            test(_do.find(false)->second[0] == true);
            test(_do.find(false)->second[1] == false);
            test(ro.size() == 2);
            test(ro.find(false) != ro.end());
            test(ro.find(false)->second.size() == 2);
            test(ro.find(false)->second[0] == true);
            test(ro.find(false)->second[1] == false);
            test(ro.find(true) != ro.end());
            test(ro.find(true)->second.size() == 3);
            test(ro.find(true)->second[0] == false);
            test(ro.find(true)->second[1] == true);
            test(ro.find(true)->second[2] == true);
            called();
        }

        void opShortShortSD(const ShortShortSD& ro, const ShortShortSD& _do)
        {
            test(_do.size() == 1);
            test(_do.find(4) != _do.end());
            test(_do.find(4)->second.size() == 2);
            test(_do.find(4)->second[0] == 6);
            test(_do.find(4)->second[1] == 7);
            test(ro.size() == 3);
            test(ro.find(1) != ro.end());
            test(ro.find(1)->second.size() == 3);
            test(ro.find(1)->second[0] == 1);
            test(ro.find(1)->second[1] == 2);
            test(ro.find(1)->second[2] == 3);
            test(ro.find(2) != ro.end());
            test(ro.find(2)->second.size() == 2);
            test(ro.find(2)->second[0] == 4);
            test(ro.find(2)->second[1] == 5);
            test(ro.find(4) != ro.end());
            test(ro.find(4)->second.size() == 2);
            test(ro.find(4)->second[0] == 6);
            test(ro.find(4)->second[1] == 7);
            called();
        }

        void opIntIntSD(const IntIntSD& ro, const IntIntSD& _do)
        {
            test(_do.size() == 1);
            test(_do.find(400) != _do.end());
            test(_do.find(400)->second.size() == 2);
            test(_do.find(400)->second[0] == 600);
            test(_do.find(400)->second[1] == 700);
            test(ro.size() == 3);
            test(ro.find(100) != ro.end());
            test(ro.find(100)->second.size() == 3);
            test(ro.find(100)->second[0] == 100);
            test(ro.find(100)->second[1] == 200);
            test(ro.find(100)->second[2] == 300);
            test(ro.find(200) != ro.end());
            test(ro.find(200)->second.size() == 2);
            test(ro.find(200)->second[0] == 400);
            test(ro.find(200)->second[1] == 500);
            test(ro.find(400) != ro.end());
            test(ro.find(400)->second.size() == 2);
            test(ro.find(400)->second[0] == 600);
            test(ro.find(400)->second[1] == 700);
            called();
        }

        void opLongLongSD(const LongLongSD& ro, const LongLongSD& _do)
        {
            test(_do.size() == 1);
            test(_do.find(999999992) != _do.end());
            test(_do.find(999999992)->second.size() == 2);
            test(_do.find(999999992)->second[0] == 999999110);
            test(_do.find(999999992)->second[1] == 999999120);
            test(ro.size() == 3);
            test(ro.find(999999990) != ro.end());
            test(ro.find(999999990)->second.size() == 3);
            test(ro.find(999999990)->second[0] == 999999110);
            test(ro.find(999999990)->second[1] == 999999111);
            test(ro.find(999999990)->second[2] == 999999110);
            test(ro.find(999999991) != ro.end());
            test(ro.find(999999991)->second.size() == 2);
            test(ro.find(999999991)->second[0] == 999999120);
            test(ro.find(999999991)->second[1] == 999999130);
            test(ro.find(999999992) != ro.end());
            test(ro.find(999999992)->second.size() == 2);
            test(ro.find(999999992)->second[0] == 999999110);
            test(ro.find(999999992)->second[1] == 999999120);
            called();
        }

        void opStringFloatSD(const StringFloatSD& ro, const StringFloatSD& _do)
        {
            test(_do.size() == 1);
            test(_do.find("aBc") != _do.end());
            test(_do.find("aBc")->second.size() == 2);
            test(_do.find("aBc")->second[0] == float(-3.14));
            test(_do.find("aBc")->second[1] == float(3.14));
            test(ro.size() == 3);
            test(ro.find("abc") != ro.end());
            test(ro.find("abc")->second.size() == 3);
            test(ro.find("abc")->second[0] == float(-1.1));
            test(ro.find("abc")->second[1] == float(123123.2));
            test(ro.find("abc")->second[2] == float(100.0));
            test(ro.find("ABC") != ro.end());
            test(ro.find("ABC")->second.size() == 2);
            test(ro.find("ABC")->second[0] == float(42.24));
            test(ro.find("ABC")->second[1] == float(-1.61));
            test(ro.find("aBc") != ro.end());
            test(ro.find("aBc")->second.size() == 2);
            test(ro.find("aBc")->second[0] == float(-3.14));
            test(ro.find("aBc")->second[1] == float(3.14));
            called();
        }

        void opStringDoubleSD(const StringDoubleSD& ro, const StringDoubleSD& _do)
        {
            test(_do.size() == 1);
            test(_do.find("") != _do.end());
            test(_do.find("")->second.size() == 2);
            test(_do.find("")->second[0] == double(1.6E10));
            test(_do.find("")->second[1] == double(1.7E10));
            test(ro.size() == 3);
            test(ro.find("Hello!!") != ro.end());
            test(ro.find("Hello!!")->second.size() == 3);
            test(ro.find("Hello!!")->second[0] == double(1.1E10));
            test(ro.find("Hello!!")->second[1] == double(1.2E10));
            test(ro.find("Hello!!")->second[2] == double(1.3E10));
            test(ro.find("Goodbye") != ro.end());
            test(ro.find("Goodbye")->second.size() == 2);
            test(ro.find("Goodbye")->second[0] == double(1.4E10));
            test(ro.find("Goodbye")->second[1] == double(1.5E10));
            test(ro.find("") != ro.end());
            test(ro.find("")->second.size() == 2);
            test(ro.find("")->second[0] == double(1.6E10));
            test(ro.find("")->second[1] == double(1.7E10));
            called();
        }

        void opStringStringSD(const StringStringSD& ro, const StringStringSD& _do)
        {
            test(_do.size() == 1);
            test(_do.find("ghi") != _do.end());
            test(_do.find("ghi")->second.size() == 2);
            test(_do.find("ghi")->second[0] == "and");
            test(_do.find("ghi")->second[1] == "xor");
            test(ro.size() == 3);
            test(ro.find("abc") != ro.end());
            test(ro.find("abc")->second.size() == 3);
            test(ro.find("abc")->second[0] == "abc");
            test(ro.find("abc")->second[1] == "de");
            test(ro.find("abc")->second[2] == "fghi");
            test(ro.find("def") != ro.end());
            test(ro.find("def")->second.size() == 2);
            test(ro.find("def")->second[0] == "xyz");
            test(ro.find("def")->second[1] == "or");
            test(ro.find("ghi") != ro.end());
            test(ro.find("ghi")->second.size() == 2);
            test(ro.find("ghi")->second[0] == "and");
            test(ro.find("ghi")->second[1] == "xor");
            called();
        }

        void opMyEnumMyEnumSD(const MyEnumMyEnumSD& ro, const MyEnumMyEnumSD& _do)
        {
            test(_do.size() == 1);
            test(_do.find(MyEnum::enum1) != _do.end());
            test(_do.find(MyEnum::enum1)->second.size() == 2);
            test(_do.find(MyEnum::enum1)->second[0] == MyEnum::enum3);
            test(_do.find(MyEnum::enum1)->second[1] == MyEnum::enum3);
            test(ro.size() == 3);
            test(ro.find(MyEnum::enum3) != ro.end());
            test(ro.find(MyEnum::enum3)->second.size() == 3);
            test(ro.find(MyEnum::enum3)->second[0] == MyEnum::enum1);
            test(ro.find(MyEnum::enum3)->second[1] == MyEnum::enum1);
            test(ro.find(MyEnum::enum3)->second[2] == MyEnum::enum2);
            test(ro.find(MyEnum::enum2) != ro.end());
            test(ro.find(MyEnum::enum2)->second.size() == 2);
            test(ro.find(MyEnum::enum2)->second[0] == MyEnum::enum1);
            test(ro.find(MyEnum::enum2)->second[1] == MyEnum::enum2);
            test(ro.find(MyEnum::enum1) != ro.end());
            test(ro.find(MyEnum::enum1)->second.size() == 2);
            test(ro.find(MyEnum::enum1)->second[0] == MyEnum::enum3);
            test(ro.find(MyEnum::enum1)->second[1] == MyEnum::enum3);
            called();
        }

        void opIntS(const IntS& r)
        {
            for (int j = 0; j < static_cast<int>(r.size()); ++j)
            {
                test(r[static_cast<size_t>(j)] == -j);
            }
            called();
        }

        void opDoubleMarshaling() { called(); }

        void opIdempotent() { called(); }

        void opDerived() { called(); }

        void exCB(std::exception_ptr ex)
        {
            try
            {
                rethrow_exception(ex);
            }
            catch (const OperationNotExistException&)
            {
                called();
            }
            catch (const std::exception& e)
            {
                cerr << e.what() << endl;
                test(false);
            }
            catch (...)
            {
                test(false);
            }
        }

    private:
        CommunicatorPtr _communicator;
    };
    using CallbackPtr = shared_ptr<Callback>;
}

function<void(exception_ptr)>
makeExceptionClosure(CallbackPtr& cb)
{
    return [&](exception_ptr e) { cb->exCB(e); };
}

void
twowaysAMI(const CommunicatorPtr& communicator, const MyClassPrx& p)
{
    {
        CallbackPtr cb = make_shared<Callback>();
        p->ice_pingAsync([&]() { cb->ping(); }, makeExceptionClosure(cb));
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        p->ice_isAAsync(MyClass::ice_staticId(), [&](bool v) { cb->isA(v); }, makeExceptionClosure(cb));
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        p->ice_idAsync([&](const string& id) { cb->id(id); }, makeExceptionClosure(cb));
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        p->ice_idsAsync([&](const vector<string>& ids) { cb->ids(ids); }, makeExceptionClosure(cb));
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        p->opVoidAsync([&]() { cb->opVoid(); }, makeExceptionClosure(cb));
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        p->opByteAsync(
            uint8_t(0xff),
            uint8_t(0x0f),
            [&](uint8_t b1, uint8_t b2) { cb->opByte(b1, b2); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        p->opBoolAsync(true, false, [&](bool b1, bool b2) { cb->opBool(b1, b2); }, makeExceptionClosure(cb));
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        p->opShortIntLongAsync(
            10,
            11,
            12,
            [&](int64_t l1, short s1P, int i1, int64_t l2) { cb->opShortIntLong(l1, s1P, i1, l2); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        p->opFloatDoubleAsync(
            3.14f,
            1.1E10,
            [&](double d1, float f1, double d2) { cb->opFloatDouble(d1, f1, d2); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        p->opStringAsync(
            "hello",
            "world",
            [&](const string& s1P, const string& s2P) { cb->opString(s1P, s2P); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        p->opMyEnumAsync(MyEnum::enum2, [&](MyEnum e1, MyEnum e2) { cb->opMyEnum(e1, e2); }, makeExceptionClosure(cb));
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>(communicator);
        p->opMyClassAsync(
            p,
            [&](optional<MyClassPrx> c1, optional<MyClassPrx> c2, optional<MyClassPrx> c3)
            { cb->opMyClass(*c1, *c2, *c3); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        Structure si1;
        si1.p = p;
        si1.e = MyEnum::enum3;
        si1.s.s = "abc";
        Structure si2;
        si2.p = nullopt;
        si2.e = MyEnum::enum2;
        si2.s.s = "def";

        CallbackPtr cb = make_shared<Callback>(communicator);
        p->opStructAsync(
            si1,
            si2,
            [&](const Structure& si3, const Structure& si4) { cb->opStruct(si3, si4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        ByteS bsi1;
        ByteS bsi2;

        bsi1.push_back(byte{0x01});
        bsi1.push_back(byte{0x11});
        bsi1.push_back(byte{0x12});
        bsi1.push_back(byte{0x22});

        bsi2.push_back(byte{0xf1});
        bsi2.push_back(byte{0xf2});
        bsi2.push_back(byte{0xf3});
        bsi2.push_back(byte{0xf4});

        CallbackPtr cb = make_shared<Callback>();
        p->opByteSAsync(
            bsi1,
            bsi2,
            [&](const ByteS& bsi3, const ByteS& bsi4) { cb->opByteS(bsi3, bsi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        BoolS bsi1;
        BoolS bsi2;

        bsi1.push_back(true);
        bsi1.push_back(true);
        bsi1.push_back(false);

        bsi2.push_back(false);

        CallbackPtr cb = make_shared<Callback>();
        p->opBoolSAsync(
            bsi1,
            bsi2,
            [&](const BoolS& bsi3, const BoolS& bsi4) { cb->opBoolS(bsi3, bsi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        ShortS ssi;
        IntS isi;
        LongS lsi;

        ssi.push_back(1);
        ssi.push_back(2);
        ssi.push_back(3);

        isi.push_back(5);
        isi.push_back(6);
        isi.push_back(7);
        isi.push_back(8);

        lsi.push_back(10);
        lsi.push_back(30);
        lsi.push_back(20);

        CallbackPtr cb = make_shared<Callback>();
        p->opShortIntLongSAsync(
            ssi,
            isi,
            lsi,
            [&](const LongS& lsi1, const ShortS& ssi1, const IntS& isi1, const LongS& lsi2)
            { cb->opShortIntLongS(lsi1, ssi1, isi1, lsi2); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        FloatS fsi;
        DoubleS dsi;

        fsi.push_back(float(3.14));
        fsi.push_back(float(1.11));

        dsi.push_back(double(1.1E10));
        dsi.push_back(double(1.2E10));
        dsi.push_back(double(1.3E10));

        CallbackPtr cb = make_shared<Callback>();
        p->opFloatDoubleSAsync(
            fsi,
            dsi,
            [&](const DoubleS& dsi1, const FloatS& fsi1, const DoubleS& dsi2) { cb->opFloatDoubleS(dsi1, fsi1, dsi2); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        StringS ssi1;
        StringS ssi2;

        ssi1.emplace_back("abc");
        ssi1.emplace_back("de");
        ssi1.emplace_back("fghi");

        ssi2.emplace_back("xyz");

        CallbackPtr cb = make_shared<Callback>();
        p->opStringSAsync(
            ssi1,
            ssi2,
            [&](const StringS& ssi3, const StringS& ssi4) { cb->opStringS(ssi3, ssi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        ByteSS bsi1;
        bsi1.resize(2);
        ByteSS bsi2;
        bsi2.resize(2);

        bsi1[0].push_back(byte{0x01});
        bsi1[0].push_back(byte{0x11});
        bsi1[0].push_back(byte{0x12});
        bsi1[1].push_back(byte{0xff});

        bsi2[0].push_back(byte{0x0e});
        bsi2[1].push_back(byte{0xf2});
        bsi2[1].push_back(byte{0xf1});

        CallbackPtr cb = make_shared<Callback>();
        p->opByteSSAsync(
            bsi1,
            bsi2,
            [&](const ByteSS& bsi3, const ByteSS& bsi4) { cb->opByteSS(bsi3, bsi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        BoolSS bsi1;
        bsi1.resize(3);
        BoolSS bsi2;
        bsi2.resize(1);

        bsi1[0].push_back(true);
        bsi1[1].push_back(false);
        bsi1[2].push_back(true);
        bsi1[2].push_back(true);

        bsi2[0].push_back(false);
        bsi2[0].push_back(false);
        bsi2[0].push_back(true);

        CallbackPtr cb = make_shared<Callback>();
        p->opBoolSSAsync(
            bsi1,
            bsi2,
            [&](const BoolSS& bsi3, const BoolSS& bsi4) { cb->opBoolSS(bsi3, bsi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        ShortSS ssi;
        ssi.resize(3);
        IntSS isi;
        isi.resize(2);
        LongSS lsi;
        lsi.resize(1);
        ssi[0].push_back(1);
        ssi[0].push_back(2);
        ssi[0].push_back(5);
        ssi[1].push_back(13);
        isi[0].push_back(24);
        isi[0].push_back(98);
        isi[1].push_back(42);
        lsi[0].push_back(496);
        lsi[0].push_back(1729);

        CallbackPtr cb = make_shared<Callback>();
        p->opShortIntLongSSAsync(
            ssi,
            isi,
            lsi,
            [&](const LongSS& lsi1, const ShortSS& ssi1, const IntSS& isi1, const LongSS& lsi2)
            { cb->opShortIntLongSS(lsi1, ssi1, isi1, lsi2); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        FloatSS fsi;
        fsi.resize(3);
        DoubleSS dsi;
        dsi.resize(1);

        fsi[0].push_back(float(3.14));
        fsi[1].push_back(float(1.11));

        dsi[0].push_back(double(1.1E10));
        dsi[0].push_back(double(1.2E10));
        dsi[0].push_back(double(1.3E10));

        CallbackPtr cb = make_shared<Callback>();
        p->opFloatDoubleSSAsync(
            fsi,
            dsi,
            [&](const DoubleSS& dsi1, const FloatSS& fsi1, const DoubleSS& dsi2)
            { cb->opFloatDoubleSS(dsi1, fsi1, dsi2); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        StringSS ssi1;
        ssi1.resize(2);
        StringSS ssi2;
        ssi2.resize(3);

        ssi1[0].emplace_back("abc");
        ssi1[1].emplace_back("de");
        ssi1[1].emplace_back("fghi");

        ssi2[2].emplace_back("xyz");

        CallbackPtr cb = make_shared<Callback>();
        p->opStringSSAsync(
            ssi1,
            ssi2,
            [&](const StringSS& ssi3, const StringSS& ssi4) { cb->opStringSS(ssi3, ssi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        ByteBoolD di1;
        di1[10] = true;
        di1[100] = false;
        ByteBoolD di2;
        di2[10] = true;
        di2[11] = false;
        di2[101] = true;

        CallbackPtr cb = make_shared<Callback>();
        p->opByteBoolDAsync(
            di1,
            di2,
            [&](const ByteBoolD& di3, const ByteBoolD& di4) { cb->opByteBoolD(di3, di4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        ShortIntD di1;
        di1[110] = -1;
        di1[1100] = 123123;
        ShortIntD di2;
        di2[110] = -1;
        di2[111] = -100;
        di2[1101] = 0;

        CallbackPtr cb = make_shared<Callback>();
        p->opShortIntDAsync(
            di1,
            di2,
            [&](const ShortIntD& di3, const ShortIntD& di4) { cb->opShortIntD(di3, di4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        LongFloatD di1;
        di1[999999110] = float(-1.1);
        di1[999999111] = float(123123.2);
        LongFloatD di2;
        di2[999999110] = float(-1.1);
        di2[999999120] = float(-100.4);
        di2[999999130] = float(0.5);

        CallbackPtr cb = make_shared<Callback>();
        p->opLongFloatDAsync(
            di1,
            di2,
            [&](const LongFloatD& di3, const LongFloatD& di4) { cb->opLongFloatD(di3, di4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        StringStringD di1;
        di1["foo"] = "abc -1.1";
        di1["bar"] = "abc 123123.2";
        StringStringD di2;
        di2["foo"] = "abc -1.1";
        di2["FOO"] = "abc -100.4";
        di2["BAR"] = "abc 0.5";

        CallbackPtr cb = make_shared<Callback>();
        p->opStringStringDAsync(
            di1,
            di2,
            [&](const StringStringD& di3, const StringStringD& di4) { cb->opStringStringD(di3, di4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        StringMyEnumD di1;
        di1["abc"] = MyEnum::enum1;
        di1[""] = MyEnum::enum2;
        StringMyEnumD di2;
        di2["abc"] = MyEnum::enum1;
        di2["qwerty"] = MyEnum::enum3;
        di2["Hello!!"] = MyEnum::enum2;

        CallbackPtr cb = make_shared<Callback>();
        p->opStringMyEnumDAsync(
            di1,
            di2,
            [&](const StringMyEnumD& di3, const StringMyEnumD& di4) { cb->opStringMyEnumD(di3, di4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        MyStruct ms11 = {1, 1};
        MyStruct ms12 = {1, 2};
        MyStructMyEnumD di1;
        di1[ms11] = MyEnum::enum1;
        di1[ms12] = MyEnum::enum2;

        MyStruct ms22 = {2, 2};
        MyStruct ms23 = {2, 3};
        MyStructMyEnumD di2;
        di2[ms11] = MyEnum::enum1;
        di2[ms22] = MyEnum::enum3;
        di2[ms23] = MyEnum::enum2;

        CallbackPtr cb = make_shared<Callback>();
        p->opMyStructMyEnumDAsync(
            di1,
            di2,
            [&](const MyStructMyEnumD& di3, const MyStructMyEnumD& di4) { cb->opMyStructMyEnumD(di3, di4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        ByteBoolDS dsi1;
        dsi1.resize(2);
        ByteBoolDS dsi2;
        dsi2.resize(1);

        ByteBoolD di1;
        di1[10] = true;
        di1[100] = false;
        ByteBoolD di2;
        di2[10] = true;
        di2[11] = false;
        di2[101] = true;
        ByteBoolD di3;
        di3[100] = false;
        di3[101] = false;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = make_shared<Callback>();
        p->opByteBoolDSAsync(
            dsi1,
            dsi2,
            [&](const ByteBoolDS& dsi3, const ByteBoolDS& dsi4) { cb->opByteBoolDS(dsi3, dsi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        ShortIntDS dsi1;
        dsi1.resize(2);
        ShortIntDS dsi2;
        dsi2.resize(1);

        ShortIntD di1;
        di1[110] = -1;
        di1[1100] = 123123;
        ShortIntD di2;
        di2[110] = -1;
        di2[111] = -100;
        di2[1101] = 0;
        ShortIntD di3;
        di3[100] = -1001;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = make_shared<Callback>();
        p->opShortIntDSAsync(
            dsi1,
            dsi2,
            [&](const ShortIntDS& dsi3, const ShortIntDS& dsi4) { cb->opShortIntDS(dsi3, dsi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        LongFloatDS dsi1;
        dsi1.resize(2);
        LongFloatDS dsi2;
        dsi2.resize(1);

        LongFloatD di1;
        di1[999999110] = float(-1.1);
        di1[999999111] = float(123123.2);
        LongFloatD di2;
        di2[999999110] = float(-1.1);
        di2[999999120] = float(-100.4);
        di2[999999130] = float(0.5);
        LongFloatD di3;
        di3[999999140] = float(3.14);

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = make_shared<Callback>();
        p->opLongFloatDSAsync(
            dsi1,
            dsi2,
            [&](const LongFloatDS& dsi3, const LongFloatDS& dsi4) { cb->opLongFloatDS(dsi3, dsi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        StringStringDS dsi1;
        dsi1.resize(2);
        StringStringDS dsi2;
        dsi2.resize(1);

        StringStringD di1;
        di1["foo"] = "abc -1.1";
        di1["bar"] = "abc 123123.2";
        StringStringD di2;
        di2["foo"] = "abc -1.1";
        di2["FOO"] = "abc -100.4";
        di2["BAR"] = "abc 0.5";
        StringStringD di3;
        di3["f00"] = "ABC -3.14";

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = make_shared<Callback>();
        p->opStringStringDSAsync(
            dsi1,
            dsi2,
            [&](const StringStringDS& dsi3, const StringStringDS& dsi4) { cb->opStringStringDS(dsi3, dsi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        StringMyEnumDS dsi1;
        dsi1.resize(2);
        StringMyEnumDS dsi2;
        dsi2.resize(1);

        StringMyEnumD di1;
        di1["abc"] = MyEnum::enum1;
        di1[""] = MyEnum::enum2;
        StringMyEnumD di2;
        di2["abc"] = MyEnum::enum1;
        di2["qwerty"] = MyEnum::enum3;
        di2["Hello!!"] = MyEnum::enum2;
        StringMyEnumD di3;
        di3["Goodbye"] = MyEnum::enum1;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = make_shared<Callback>();
        p->opStringMyEnumDSAsync(
            dsi1,
            dsi2,
            [&](const StringMyEnumDS& dsi3, const StringMyEnumDS& dsi4) { cb->opStringMyEnumDS(dsi3, dsi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        MyEnumStringDS dsi1;
        dsi1.resize(2);
        MyEnumStringDS dsi2;
        dsi2.resize(1);

        MyEnumStringD di1;
        di1[MyEnum::enum1] = "abc";
        MyEnumStringD di2;
        di2[MyEnum::enum2] = "Hello!!";
        di2[MyEnum::enum3] = "qwerty";
        MyEnumStringD di3;
        di3[MyEnum::enum1] = "Goodbye";

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = make_shared<Callback>();
        p->opMyEnumStringDSAsync(
            dsi1,
            dsi2,
            [&](const MyEnumStringDS& dsi3, const MyEnumStringDS& dsi4) { cb->opMyEnumStringDS(dsi3, dsi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        MyStructMyEnumDS dsi1;
        dsi1.resize(2);
        MyStructMyEnumDS dsi2;
        dsi2.resize(1);

        MyStruct ms11 = {1, 1};
        MyStruct ms12 = {1, 2};
        MyStructMyEnumD di1;
        di1[ms11] = MyEnum::enum1;
        di1[ms12] = MyEnum::enum2;

        MyStruct ms22 = {2, 2};
        MyStruct ms23 = {2, 3};
        MyStructMyEnumD di2;
        di2[ms11] = MyEnum::enum1;
        di2[ms22] = MyEnum::enum3;
        di2[ms23] = MyEnum::enum2;

        MyStructMyEnumD di3;
        di3[ms23] = MyEnum::enum3;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = make_shared<Callback>();
        p->opMyStructMyEnumDSAsync(
            dsi1,
            dsi2,
            [&](const MyStructMyEnumDS& dsi3, const MyStructMyEnumDS& dsi4) { cb->opMyStructMyEnumDS(dsi3, dsi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        ByteByteSD sdi1;
        ByteByteSD sdi2;

        ByteS si1;
        ByteS si2;
        ByteS si3;

        si1.push_back(byte{0x01});
        si1.push_back(byte{0x11});
        si2.push_back(byte{0x12});
        si3.push_back(byte{0xf2});
        si3.push_back(byte{0xf3});

        sdi1[uint8_t{0x01}] = si1;
        sdi1[uint8_t{0x22}] = si2;
        sdi2[uint8_t{0xf1}] = si3;

        CallbackPtr cb = make_shared<Callback>();
        p->opByteByteSDAsync(
            sdi1,
            sdi2,
            [&](const ByteByteSD& sdi3, const ByteByteSD& sdi4) { cb->opByteByteSD(sdi3, sdi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        BoolBoolSD sdi1;
        BoolBoolSD sdi2;

        BoolS si1;
        BoolS si2;

        si1.push_back(true);
        si1.push_back(false);
        si2.push_back(false);
        si2.push_back(true);
        si2.push_back(true);

        sdi1[false] = si1;
        sdi1[true] = si2;
        sdi2[false] = si1;

        CallbackPtr cb = make_shared<Callback>();
        p->opBoolBoolSDAsync(
            sdi1,
            sdi2,
            [&](const BoolBoolSD& sdi3, const BoolBoolSD& sdi4) { cb->opBoolBoolSD(sdi3, sdi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        ShortShortSD sdi1;
        ShortShortSD sdi2;

        ShortS si1;
        ShortS si2;
        ShortS si3;

        si1.push_back(1);
        si1.push_back(2);
        si1.push_back(3);
        si2.push_back(4);
        si2.push_back(5);
        si3.push_back(6);
        si3.push_back(7);

        sdi1[1] = si1;
        sdi1[2] = si2;
        sdi2[4] = si3;

        CallbackPtr cb = make_shared<Callback>();
        p->opShortShortSDAsync(
            sdi1,
            sdi2,
            [&](const ShortShortSD& sdi3, const ShortShortSD& sdi4) { cb->opShortShortSD(sdi3, sdi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        IntIntSD sdi1;
        IntIntSD sdi2;

        IntS si1;
        IntS si2;
        IntS si3;

        si1.push_back(100);
        si1.push_back(200);
        si1.push_back(300);
        si2.push_back(400);
        si2.push_back(500);
        si3.push_back(600);
        si3.push_back(700);

        sdi1[100] = si1;
        sdi1[200] = si2;
        sdi2[400] = si3;

        CallbackPtr cb = make_shared<Callback>();
        p->opIntIntSDAsync(
            sdi1,
            sdi2,
            [&](const IntIntSD& sdi3, const IntIntSD& sdi4) { cb->opIntIntSD(sdi3, sdi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        LongLongSD sdi1;
        LongLongSD sdi2;

        LongS si1;
        LongS si2;
        LongS si3;

        si1.push_back(999999110);
        si1.push_back(999999111);
        si1.push_back(999999110);
        si2.push_back(999999120);
        si2.push_back(999999130);
        si3.push_back(999999110);
        si3.push_back(999999120);

        sdi1[999999990] = si1;
        sdi1[999999991] = si2;
        sdi2[999999992] = si3;

        CallbackPtr cb = make_shared<Callback>();
        p->opLongLongSDAsync(
            sdi1,
            sdi2,
            [&](const LongLongSD& sdi3, const LongLongSD& sdi4) { cb->opLongLongSD(sdi3, sdi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        StringFloatSD sdi1;
        StringFloatSD sdi2;

        FloatS si1;
        FloatS si2;
        FloatS si3;

        si1.push_back(float(-1.1));
        si1.push_back(float(123123.2));
        si1.push_back(float(100.0));
        si2.push_back(float(42.24));
        si2.push_back(float(-1.61));
        si3.push_back(float(-3.14));
        si3.push_back(float(3.14));

        sdi1["abc"] = si1;
        sdi1["ABC"] = si2;
        sdi2["aBc"] = si3;

        CallbackPtr cb = make_shared<Callback>();
        p->opStringFloatSDAsync(
            sdi1,
            sdi2,
            [&](const StringFloatSD& sdi3, const StringFloatSD& sdi4) { cb->opStringFloatSD(sdi3, sdi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        StringDoubleSD sdi1;
        StringDoubleSD sdi2;

        DoubleS si1;
        DoubleS si2;
        DoubleS si3;

        si1.push_back(double(1.1E10));
        si1.push_back(double(1.2E10));
        si1.push_back(double(1.3E10));
        si2.push_back(double(1.4E10));
        si2.push_back(double(1.5E10));
        si3.push_back(double(1.6E10));
        si3.push_back(double(1.7E10));

        sdi1["Hello!!"] = si1;
        sdi1["Goodbye"] = si2;
        sdi2[""] = si3;

        CallbackPtr cb = make_shared<Callback>();
        p->opStringDoubleSDAsync(
            sdi1,
            sdi2,
            [&](const StringDoubleSD& sdi3, const StringDoubleSD& sdi4) { cb->opStringDoubleSD(sdi3, sdi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        StringStringSD sdi1;
        StringStringSD sdi2;

        StringS si1;
        StringS si2;
        StringS si3;

        si1.emplace_back("abc");
        si1.emplace_back("de");
        si1.emplace_back("fghi");

        si2.emplace_back("xyz");
        si2.emplace_back("or");

        si3.emplace_back("and");
        si3.emplace_back("xor");

        sdi1["abc"] = si1;
        sdi1["def"] = si2;
        sdi2["ghi"] = si3;

        CallbackPtr cb = make_shared<Callback>();
        p->opStringStringSDAsync(
            sdi1,
            sdi2,
            [&](const StringStringSD& sdi3, const StringStringSD& sdi4) { cb->opStringStringSD(sdi3, sdi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        MyEnumMyEnumSD sdi1;
        MyEnumMyEnumSD sdi2;

        MyEnumS si1;
        MyEnumS si2;
        MyEnumS si3;

        si1.push_back(MyEnum::enum1);
        si1.push_back(MyEnum::enum1);
        si1.push_back(MyEnum::enum2);
        si2.push_back(MyEnum::enum1);
        si2.push_back(MyEnum::enum2);
        si3.push_back(MyEnum::enum3);
        si3.push_back(MyEnum::enum3);

        sdi1[MyEnum::enum3] = si1;
        sdi1[MyEnum::enum2] = si2;
        sdi2[MyEnum::enum1] = si3;

        CallbackPtr cb = make_shared<Callback>();
        p->opMyEnumMyEnumSDAsync(
            sdi1,
            sdi2,
            [&](const MyEnumMyEnumSD& sdi3, const MyEnumMyEnumSD& sdi4) { cb->opMyEnumMyEnumSD(sdi3, sdi4); },
            makeExceptionClosure(cb));
        cb->check();
    }

    {
        const int lengths[] = {0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000};

        for (int length : lengths)
        {
            IntS s;
            for (int i = 0; i < length; ++i)
            {
                s.push_back(i);
            }
            CallbackPtr cb = make_shared<Callback>();
            p->opIntSAsync(s, [&](const IntS& s1P) { cb->opIntS(s1P); }, makeExceptionClosure(cb));
            cb->check();
        }
    }

    {
        {
            Context ctx;
            ctx["one"] = "ONE";
            ctx["two"] = "TWO";
            ctx["three"] = "THREE";
            {
                test(p->ice_getContext().empty());
                promise<void> prom;
                p->opContextAsync(
                    [&](const Context& c)
                    {
                        test(c != ctx);
                        prom.set_value();
                    },
                    [](exception_ptr) { test(false); });
                prom.get_future().get();
            }
            {
                test(p->ice_getContext().empty());
                promise<void> prom;
                p->opContextAsync(
                    [&](const Context& c)
                    {
                        test(c == ctx);
                        prom.set_value();
                    },
                    [](exception_ptr) { test(false); },
                    nullptr,
                    ctx);
                prom.get_future().get();
            }
            {
                MyClassPrx p2 = p->ice_context(ctx);
                test(p2->ice_getContext() == ctx);
                promise<void> prom;
                p2->opContextAsync(
                    [&](const Context& c)
                    {
                        test(c == ctx);
                        prom.set_value();
                    },
                    [](exception_ptr) { test(false); });
                prom.get_future().get();
            }
            {
                MyClassPrx p2 = p->ice_context(ctx);
                promise<void> prom;
                p2->opContextAsync(
                    [&](const Context& c)
                    {
                        test(c == ctx);
                        prom.set_value();
                    },
                    [](exception_ptr) { test(false); },
                    nullptr,
                    ctx);
                prom.get_future().get();
            }
        }

        if (p->ice_getConnection() && communicator->getProperties()->getIceProperty("Ice.Default.Protocol") != "bt")
        {
            //
            // Test implicit context propagation
            //

            string impls[] = {"Shared", "PerThread"};
            for (const auto& impl : impls)
            {
                InitializationData initData;
                initData.properties = communicator->getProperties()->clone();
                initData.properties->setProperty("Ice.ImplicitContext", impl);
                installTransport(initData);

                CommunicatorPtr ic = initialize(initData);

                Context ctx;
                ctx["one"] = "ONE";
                ctx["two"] = "TWO";
                ctx["three"] = "THREE";

                PropertiesPtr properties = ic->getProperties();
                MyClassPrx q(ic, "test:" + TestHelper::getTestEndpoint(properties));
                ic->getImplicitContext()->setContext(ctx);
                test(ic->getImplicitContext()->getContext() == ctx);
                {
                    promise<void> prom;
                    q->opContextAsync(
                        [&](const Context& c)
                        {
                            test(c == ctx);
                            prom.set_value();
                        },
                        [](exception_ptr) { test(false); });
                    prom.get_future().get();
                }

                ic->getImplicitContext()->put("zero", "ZERO");

                ctx = ic->getImplicitContext()->getContext();
                {
                    promise<void> prom;
                    q->opContextAsync(
                        [&](const Context& c)
                        {
                            test(c == ctx);
                            prom.set_value();
                        },
                        [](exception_ptr ex)
                        {
                            try
                            {
                                rethrow_exception(ex);
                            }
                            catch (const Exception& e)
                            {
                                cerr << e << endl;
                            }
                            test(false);
                        });
                    prom.get_future().get();
                }

                Context prxContext;
                prxContext["one"] = "UN";
                prxContext["four"] = "QUATRE";

                Context combined = prxContext;
                combined.insert(ctx.begin(), ctx.end());
                test(combined["one"] == "UN");

                q = q->ice_context(prxContext);

                ic->getImplicitContext()->setContext(Context());
                {
                    promise<void> prom;
                    q->opContextAsync(
                        [&](const Context& c)
                        {
                            test(c == prxContext);
                            prom.set_value();
                        },
                        [](exception_ptr) { test(false); });
                    prom.get_future().get();
                }

                ic->getImplicitContext()->setContext(ctx);
                {
                    promise<void> prom;
                    q->opContextAsync(
                        [&](const Context& c)
                        {
                            test(c == combined);
                            prom.set_value();
                        },
                        [](exception_ptr) { test(false); });
                    prom.get_future().get();
                }

                ic->getImplicitContext()->setContext(Context());
                ic->destroy();
            }
        }
    }

    {
        double d = 1278312346.0 / 13.0;
        DoubleS ds(5, d);
        CallbackPtr cb = make_shared<Callback>();
        p->opDoubleMarshalingAsync(d, ds, [&]() { cb->opDoubleMarshaling(); }, makeExceptionClosure(cb));
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        p->opIdempotentAsync([&]() { cb->opIdempotent(); }, makeExceptionClosure(cb));
        cb->check();
    }

    {
        auto derived = uncheckedCast<MyDerivedClassPrx>(p);
        CallbackPtr cb = make_shared<Callback>();
        derived->opDerivedAsync([&]() { cb->opDerived(); }, makeExceptionClosure(cb));
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        auto f = p->ice_pingAsync();
        try
        {
            f.get();
            cb->ping();
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        auto f = p->ice_isAAsync(MyClass::ice_staticId());
        try
        {
            cb->isA(f.get());
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        auto f = p->ice_idAsync();
        try
        {
            cb->id(f.get());
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        auto f = p->ice_idsAsync();
        try
        {
            cb->ids(f.get());
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opVoidAsync();
        try
        {
            f.get();
            cb->opVoid();
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opByteAsync(uint8_t(0xff), uint8_t(0x0f));
        try
        {
            auto r = f.get();
            cb->opByte(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opBoolAsync(true, false);
        try
        {
            auto r = f.get();
            cb->opBool(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opShortIntLongAsync(10, 11, 12);
        try
        {
            auto r = f.get();
            cb->opShortIntLong(get<0>(r), get<1>(r), get<2>(r), get<3>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opFloatDoubleAsync(3.14f, 1.1E10);
        try
        {
            auto r = f.get();
            cb->opFloatDouble(get<0>(r), get<1>(r), get<2>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opStringAsync("hello", "world");
        try
        {
            auto r = f.get();
            cb->opString(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opMyEnumAsync(MyEnum::enum2);
        try
        {
            auto r = f.get();
            cb->opMyEnum(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>(communicator);
        auto f = p->opMyClassAsync(p);
        try
        {
            auto r = f.get();
            cb->opMyClass(*get<0>(r), *get<1>(r), *get<2>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        Structure si1;
        si1.p = p;
        si1.e = MyEnum::enum3;
        si1.s.s = "abc";
        Structure si2;
        si2.p = nullopt;
        si2.e = MyEnum::enum2;
        si2.s.s = "def";

        CallbackPtr cb = make_shared<Callback>(communicator);
        auto f = p->opStructAsync(si1, si2);
        try
        {
            auto r = f.get();
            cb->opStruct(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        ByteS bsi1;
        ByteS bsi2;

        bsi1.push_back(byte{0x01});
        bsi1.push_back(byte{0x11});
        bsi1.push_back(byte{0x12});
        bsi1.push_back(byte{0x22});

        bsi2.push_back(byte{0xf1});
        bsi2.push_back(byte{0xf2});
        bsi2.push_back(byte{0xf3});
        bsi2.push_back(byte{0xf4});

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opByteSAsync(bsi1, bsi2);
        try
        {
            auto r = f.get();
            cb->opByteS(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        BoolS bsi1;
        BoolS bsi2;

        bsi1.push_back(true);
        bsi1.push_back(true);
        bsi1.push_back(false);

        bsi2.push_back(false);

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opBoolSAsync(bsi1, bsi2);
        try
        {
            auto r = f.get();
            cb->opBoolS(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        ShortS ssi;
        IntS isi;
        LongS lsi;

        ssi.push_back(1);
        ssi.push_back(2);
        ssi.push_back(3);

        isi.push_back(5);
        isi.push_back(6);
        isi.push_back(7);
        isi.push_back(8);

        lsi.push_back(10);
        lsi.push_back(30);
        lsi.push_back(20);

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opShortIntLongSAsync(ssi, isi, lsi);
        try
        {
            auto r = f.get();
            cb->opShortIntLongS(get<0>(r), get<1>(r), get<2>(r), get<3>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        FloatS fsi;
        DoubleS dsi;

        fsi.push_back(float(3.14));
        fsi.push_back(float(1.11));

        dsi.push_back(double(1.1E10));
        dsi.push_back(double(1.2E10));
        dsi.push_back(double(1.3E10));

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opFloatDoubleSAsync(fsi, dsi);
        try
        {
            auto r = f.get();
            cb->opFloatDoubleS(get<0>(r), get<1>(r), get<2>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        StringS ssi1;
        StringS ssi2;

        ssi1.emplace_back("abc");
        ssi1.emplace_back("de");
        ssi1.emplace_back("fghi");

        ssi2.emplace_back("xyz");

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opStringSAsync(ssi1, ssi2);
        try
        {
            auto r = f.get();
            cb->opStringS(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        ByteSS bsi1;
        bsi1.resize(2);
        ByteSS bsi2;
        bsi2.resize(2);

        bsi1[0].push_back(byte{0x01});
        bsi1[0].push_back(byte{0x11});
        bsi1[0].push_back(byte{0x12});
        bsi1[1].push_back(byte{0xff});

        bsi2[0].push_back(byte{0x0e});
        bsi2[1].push_back(byte{0xf2});
        bsi2[1].push_back(byte{0xf1});

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opByteSSAsync(bsi1, bsi2);
        try
        {
            auto r = f.get();
            cb->opByteSS(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        FloatSS fsi;
        fsi.resize(3);
        DoubleSS dsi;
        dsi.resize(1);

        fsi[0].push_back(float(3.14));
        fsi[1].push_back(float(1.11));

        dsi[0].push_back(double(1.1E10));
        dsi[0].push_back(double(1.2E10));
        dsi[0].push_back(double(1.3E10));

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opFloatDoubleSSAsync(fsi, dsi);
        try
        {
            auto r = f.get();
            cb->opFloatDoubleSS(get<0>(r), get<1>(r), get<2>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        StringSS ssi1;
        ssi1.resize(2);
        StringSS ssi2;
        ssi2.resize(3);

        ssi1[0].emplace_back("abc");
        ssi1[1].emplace_back("de");
        ssi1[1].emplace_back("fghi");

        ssi2[2].emplace_back("xyz");

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opStringSSAsync(ssi1, ssi2);
        try
        {
            auto r = f.get();
            cb->opStringSS(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        ByteBoolD di1;
        di1[10] = true;
        di1[100] = false;
        ByteBoolD di2;
        di2[10] = true;
        di2[11] = false;
        di2[101] = true;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opByteBoolDAsync(di1, di2);
        try
        {
            auto r = f.get();
            cb->opByteBoolD(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        ShortIntD di1;
        di1[110] = -1;
        di1[1100] = 123123;
        ShortIntD di2;
        di2[110] = -1;
        di2[111] = -100;
        di2[1101] = 0;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opShortIntDAsync(di1, di2);
        try
        {
            auto r = f.get();
            cb->opShortIntD(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        LongFloatD di1;
        di1[999999110] = float(-1.1);
        di1[999999111] = float(123123.2);
        LongFloatD di2;
        di2[999999110] = float(-1.1);
        di2[999999120] = float(-100.4);
        di2[999999130] = float(0.5);

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opLongFloatDAsync(di1, di2);
        try
        {
            auto r = f.get();
            cb->opLongFloatD(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        StringStringD di1;
        di1["foo"] = "abc -1.1";
        di1["bar"] = "abc 123123.2";
        StringStringD di2;
        di2["foo"] = "abc -1.1";
        di2["FOO"] = "abc -100.4";
        di2["BAR"] = "abc 0.5";

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opStringStringDAsync(di1, di2);
        try
        {
            auto r = f.get();
            cb->opStringStringD(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        StringMyEnumD di1;
        di1["abc"] = MyEnum::enum1;
        di1[""] = MyEnum::enum2;
        StringMyEnumD di2;
        di2["abc"] = MyEnum::enum1;
        di2["qwerty"] = MyEnum::enum3;
        di2["Hello!!"] = MyEnum::enum2;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opStringMyEnumDAsync(di1, di2);
        try
        {
            auto r = f.get();
            cb->opStringMyEnumD(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        MyStruct ms11 = {1, 1};
        MyStruct ms12 = {1, 2};
        MyStructMyEnumD di1;
        di1[ms11] = MyEnum::enum1;
        di1[ms12] = MyEnum::enum2;

        MyStruct ms22 = {2, 2};
        MyStruct ms23 = {2, 3};
        MyStructMyEnumD di2;
        di2[ms11] = MyEnum::enum1;
        di2[ms22] = MyEnum::enum3;
        di2[ms23] = MyEnum::enum2;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opMyStructMyEnumDAsync(di1, di2);
        try
        {
            auto r = f.get();
            cb->opMyStructMyEnumD(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        ByteBoolDS dsi1;
        dsi1.resize(2);
        ByteBoolDS dsi2;
        dsi2.resize(1);

        ByteBoolD di1;
        di1[10] = true;
        di1[100] = false;
        ByteBoolD di2;
        di2[10] = true;
        di2[11] = false;
        di2[101] = true;
        ByteBoolD di3;
        di3[100] = false;
        di3[101] = false;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opByteBoolDSAsync(dsi1, dsi2);
        try
        {
            auto r = f.get();
            cb->opByteBoolDS(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        ShortIntDS dsi1;
        dsi1.resize(2);
        ShortIntDS dsi2;
        dsi2.resize(1);

        ShortIntD di1;
        di1[110] = -1;
        di1[1100] = 123123;
        ShortIntD di2;
        di2[110] = -1;
        di2[111] = -100;
        di2[1101] = 0;
        ShortIntD di3;
        di3[100] = -1001;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opShortIntDSAsync(dsi1, dsi2);
        try
        {
            auto r = f.get();
            cb->opShortIntDS(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        LongFloatDS dsi1;
        dsi1.resize(2);
        LongFloatDS dsi2;
        dsi2.resize(1);

        LongFloatD di1;
        di1[999999110] = float(-1.1);
        di1[999999111] = float(123123.2);
        LongFloatD di2;
        di2[999999110] = float(-1.1);
        di2[999999120] = float(-100.4);
        di2[999999130] = float(0.5);
        LongFloatD di3;
        di3[999999140] = float(3.14);

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opLongFloatDSAsync(dsi1, dsi2);
        try
        {
            auto r = f.get();
            cb->opLongFloatDS(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        StringStringDS dsi1;
        dsi1.resize(2);
        StringStringDS dsi2;
        dsi2.resize(1);

        StringStringD di1;
        di1["foo"] = "abc -1.1";
        di1["bar"] = "abc 123123.2";
        StringStringD di2;
        di2["foo"] = "abc -1.1";
        di2["FOO"] = "abc -100.4";
        di2["BAR"] = "abc 0.5";
        StringStringD di3;
        di3["f00"] = "ABC -3.14";

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opStringStringDSAsync(dsi1, dsi2);
        try
        {
            auto r = f.get();
            cb->opStringStringDS(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        StringMyEnumDS dsi1;
        dsi1.resize(2);
        StringMyEnumDS dsi2;
        dsi2.resize(1);

        StringMyEnumD di1;
        di1["abc"] = MyEnum::enum1;
        di1[""] = MyEnum::enum2;
        StringMyEnumD di2;
        di2["abc"] = MyEnum::enum1;
        di2["qwerty"] = MyEnum::enum3;
        di2["Hello!!"] = MyEnum::enum2;
        StringMyEnumD di3;
        di3["Goodbye"] = MyEnum::enum1;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opStringMyEnumDSAsync(dsi1, dsi2);
        try
        {
            auto r = f.get();
            cb->opStringMyEnumDS(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        MyEnumStringDS dsi1;
        dsi1.resize(2);
        MyEnumStringDS dsi2;
        dsi2.resize(1);

        MyEnumStringD di1;
        di1[MyEnum::enum1] = "abc";
        MyEnumStringD di2;
        di2[MyEnum::enum2] = "Hello!!";
        di2[MyEnum::enum3] = "qwerty";
        MyEnumStringD di3;
        di3[MyEnum::enum1] = "Goodbye";

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opMyEnumStringDSAsync(dsi1, dsi2);
        try
        {
            auto r = f.get();
            cb->opMyEnumStringDS(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        MyStructMyEnumDS dsi1;
        dsi1.resize(2);
        MyStructMyEnumDS dsi2;
        dsi2.resize(1);

        MyStruct ms11 = {1, 1};
        MyStruct ms12 = {1, 2};
        MyStructMyEnumD di1;
        di1[ms11] = MyEnum::enum1;
        di1[ms12] = MyEnum::enum2;

        MyStruct ms22 = {2, 2};
        MyStruct ms23 = {2, 3};
        MyStructMyEnumD di2;
        di2[ms11] = MyEnum::enum1;
        di2[ms22] = MyEnum::enum3;
        di2[ms23] = MyEnum::enum2;

        MyStructMyEnumD di3;
        di3[ms23] = MyEnum::enum3;

        dsi1[0] = di1;
        dsi1[1] = di2;
        dsi2[0] = di3;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opMyStructMyEnumDSAsync(dsi1, dsi2);
        try
        {
            auto r = f.get();
            cb->opMyStructMyEnumDS(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        ByteByteSD sdi1;
        ByteByteSD sdi2;

        ByteS si1;
        ByteS si2;
        ByteS si3;

        si1.push_back(byte{0x01});
        si1.push_back(byte{0x11});
        si2.push_back(byte{0x12});
        si3.push_back(byte{0xf2});
        si3.push_back(byte{0xf3});

        sdi1[uint8_t{0x01}] = si1;
        sdi1[uint8_t{0x22}] = si2;
        sdi2[uint8_t{0xf1}] = si3;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opByteByteSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opByteByteSD(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        BoolBoolSD sdi1;
        BoolBoolSD sdi2;

        BoolS si1;
        BoolS si2;

        si1.push_back(true);
        si1.push_back(false);
        si2.push_back(false);
        si2.push_back(true);
        si2.push_back(true);

        sdi1[false] = si1;
        sdi1[true] = si2;
        sdi2[false] = si1;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opBoolBoolSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opBoolBoolSD(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        ShortShortSD sdi1;
        ShortShortSD sdi2;

        ShortS si1;
        ShortS si2;
        ShortS si3;

        si1.push_back(1);
        si1.push_back(2);
        si1.push_back(3);
        si2.push_back(4);
        si2.push_back(5);
        si3.push_back(6);
        si3.push_back(7);

        sdi1[1] = si1;
        sdi1[2] = si2;
        sdi2[4] = si3;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opShortShortSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opShortShortSD(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        IntIntSD sdi1;
        IntIntSD sdi2;

        IntS si1;
        IntS si2;
        IntS si3;

        si1.push_back(100);
        si1.push_back(200);
        si1.push_back(300);
        si2.push_back(400);
        si2.push_back(500);
        si3.push_back(600);
        si3.push_back(700);

        sdi1[100] = si1;
        sdi1[200] = si2;
        sdi2[400] = si3;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opIntIntSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opIntIntSD(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        LongLongSD sdi1;
        LongLongSD sdi2;

        LongS si1;
        LongS si2;
        LongS si3;

        si1.push_back(999999110);
        si1.push_back(999999111);
        si1.push_back(999999110);
        si2.push_back(999999120);
        si2.push_back(999999130);
        si3.push_back(999999110);
        si3.push_back(999999120);

        sdi1[999999990] = si1;
        sdi1[999999991] = si2;
        sdi2[999999992] = si3;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opLongLongSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opLongLongSD(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        StringFloatSD sdi1;
        StringFloatSD sdi2;

        FloatS si1;
        FloatS si2;
        FloatS si3;

        si1.push_back(float(-1.1));
        si1.push_back(float(123123.2));
        si1.push_back(float(100.0));
        si2.push_back(float(42.24));
        si2.push_back(float(-1.61));
        si3.push_back(float(-3.14));
        si3.push_back(float(3.14));

        sdi1["abc"] = si1;
        sdi1["ABC"] = si2;
        sdi2["aBc"] = si3;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opStringFloatSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opStringFloatSD(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        StringDoubleSD sdi1;
        StringDoubleSD sdi2;

        DoubleS si1;
        DoubleS si2;
        DoubleS si3;

        si1.push_back(double(1.1E10));
        si1.push_back(double(1.2E10));
        si1.push_back(double(1.3E10));
        si2.push_back(double(1.4E10));
        si2.push_back(double(1.5E10));
        si3.push_back(double(1.6E10));
        si3.push_back(double(1.7E10));

        sdi1["Hello!!"] = si1;
        sdi1["Goodbye"] = si2;
        sdi2[""] = si3;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opStringDoubleSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opStringDoubleSD(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        StringStringSD sdi1;
        StringStringSD sdi2;

        StringS si1;
        StringS si2;
        StringS si3;

        si1.emplace_back("abc");
        si1.emplace_back("de");
        si1.emplace_back("fghi");

        si2.emplace_back("xyz");
        si2.emplace_back("or");

        si3.emplace_back("and");
        si3.emplace_back("xor");

        sdi1["abc"] = si1;
        sdi1["def"] = si2;
        sdi2["ghi"] = si3;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opStringStringSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opStringStringSD(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        MyEnumMyEnumSD sdi1;
        MyEnumMyEnumSD sdi2;

        MyEnumS si1;
        MyEnumS si2;
        MyEnumS si3;

        si1.push_back(MyEnum::enum1);
        si1.push_back(MyEnum::enum1);
        si1.push_back(MyEnum::enum2);
        si2.push_back(MyEnum::enum1);
        si2.push_back(MyEnum::enum2);
        si3.push_back(MyEnum::enum3);
        si3.push_back(MyEnum::enum3);

        sdi1[MyEnum::enum3] = si1;
        sdi1[MyEnum::enum2] = si2;
        sdi2[MyEnum::enum1] = si3;

        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opMyEnumMyEnumSDAsync(sdi1, sdi2);
        try
        {
            auto r = f.get();
            cb->opMyEnumMyEnumSD(get<0>(r), get<1>(r));
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        const int lengths[] = {0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000};

        for (int length : lengths)
        {
            IntS s;
            for (int i = 0; i < length; ++i)
            {
                s.push_back(i);
            }
            CallbackPtr cb = make_shared<Callback>();
            auto f = p->opIntSAsync(s);
            try
            {
                cb->opIntS(f.get());
            }
            catch (...)
            {
                cb->exCB(current_exception());
            }
            cb->check();
        }
    }

    {
        double d = 1278312346.0 / 13.0;
        DoubleS ds(5, d);
        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opDoubleMarshalingAsync(d, ds);
        try
        {
            f.get();
            cb->opDoubleMarshaling();
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        CallbackPtr cb = make_shared<Callback>();
        auto f = p->opIdempotentAsync();
        try
        {
            f.get();
            cb->opIdempotent();
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }

    {
        auto derived = uncheckedCast<MyDerivedClassPrx>(p);
        CallbackPtr cb = make_shared<Callback>();
        auto f = derived->opDerivedAsync();
        try
        {
            f.get();
            cb->opDerived();
        }
        catch (...)
        {
            cb->exCB(current_exception());
        }
        cb->check();
    }
}
