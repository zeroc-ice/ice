// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    //
    // Scoped types
    //
    {
        Test::MyInterfacePrx i(communicator, "i1:" + helper->getTestEndpoint());

        Test::MyStruct s1;
        s1.v = 0;
        Test::MyStruct s2;
        Test::MyStruct s3 = i->opMyStruct(s1, s2);
        test(s1 == s2);
        test(s1 == s3);

        Test::MyStructSeq sseq1;
        sseq1.push_back(s1);
        Test::MyStructSeq sseq2;
        Test::MyStructSeq sseq3 = i->opMyStructSeq(sseq1, sseq2);
        test(sseq2 == sseq1);
        test(sseq3 == sseq1);

        Test::MyStructMap smap1;
        smap1["a"] = s1;
        Test::MyStructMap smap2;
        Test::MyStructMap smap3 = i->opMyStructMap(smap1, smap2);
        test(smap2 == smap1);
        test(smap3 == smap1);

        Test::MyClassPtr c1 = make_shared<Test::MyClass>(s1);
        Test::MyClassPtr c2;
        Test::MyClassPtr c3 = i->opMyClass(c1, c2);
        test(c2->s == c1->s);
        test(c3->s == c1->s);

        Test::MyClassSeq cseq1;
        cseq1.push_back(c1);
        Test::MyClassSeq cseq2;
        Test::MyClassSeq cseq3 = i->opMyClassSeq(cseq1, cseq2);
        test(cseq2[0]->s == c1->s);
        test(cseq3[0]->s == c1->s);

        Test::MyClassMap cmap1;
        cmap1["a"] = c1;
        Test::MyClassMap cmap2;
        Test::MyClassMap cmap3 = i->opMyClassMap(cmap1, cmap2);
        test(cmap2["a"]->s == c1->s);
        test(cmap3["a"]->s == c1->s);

        Test::MyEnum e = i->opMyEnum(Test::MyEnum::v1);
        test(e == Test::MyEnum::v1);

        Test::MyOtherStruct s;
        s.s = "MyOtherStruct";
        s = i->opMyOtherStruct(s);
        test(s.s == "MyOtherStruct");

        Test::MyOtherClassPtr c = i->opMyOtherClass(make_shared<Test::MyOtherClass>("MyOtherClass"));
        test(c->s == "MyOtherClass");
    }

    //
    // Future-Based Async Function
    //
    {
        Test::MyInterfacePrx i(communicator, "i1:" + helper->getTestEndpoint());

        Test::MyStruct s1;
        s1.v = 0;
        {
            auto result = i->opMyStructAsync(s1).get();
            test(std::get<0>(result) == s1);
            test(std::get<1>(result) == s1);
        }

        Test::MyStructSeq sseq1;
        sseq1.push_back(s1);
        {
            auto result = i->opMyStructSeqAsync(sseq1).get();
            test(std::get<0>(result) == sseq1);
            test(std::get<1>(result) == sseq1);
        }

        Test::MyStructMap smap1;
        smap1["a"] = s1;
        {
            auto result = i->opMyStructMapAsync(smap1).get();
            test(std::get<0>(result) == smap1);
            test(std::get<1>(result) == smap1);
        }

        Test::MyClassPtr c1 = make_shared<Test::MyClass>(s1);
        {
            auto result = i->opMyClassAsync(c1).get();
            test(std::get<0>(result)->ice_tuple() == c1->ice_tuple());
            test(std::get<1>(result)->ice_tuple() == c1->ice_tuple());
        }

        Test::MyClassSeq cseq1;
        cseq1.push_back(c1);
        {
            auto result = i->opMyClassSeqAsync(cseq1).get();
            test(std::get<0>(result)[0]->ice_tuple() == c1->ice_tuple());
            test(std::get<1>(result)[0]->ice_tuple() == c1->ice_tuple());
        }

        Test::MyClassMap cmap1;
        cmap1["a"] = c1;
        {
            auto result = i->opMyClassMapAsync(cmap1).get();
            test(std::get<0>(result)["a"]->ice_tuple() == c1->ice_tuple());
            test(std::get<1>(result)["a"]->ice_tuple() == c1->ice_tuple());
        }

        {
            auto result = i->opMyEnumAsync(Test::MyEnum::v1).get();
            test(result == Test::MyEnum::v1);
        }

        {
            Test::MyOtherStruct s;
            s.s = "MyOtherStruct";
            s = i->opMyOtherStructAsync(s).get();
            test(s.s == "MyOtherStruct");
        }

        {
            auto result = i->opMyOtherClassAsync(make_shared<Test::MyOtherClass>("MyOtherClass")).get();
            test(result->s == "MyOtherClass");
        }
    }

    //
    // Callback-Based Async Function
    //
    {
        Test::MyInterfacePrx i(communicator, "i1:" + helper->getTestEndpoint());

        Test::MyStruct s1;
        s1.v = 0;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyStructAsync(
                s1,
                [&p, &s1](Test::MyStruct s2, Test::MyStruct s3)
                {
                    test(s2 == s1);
                    test(s3 == s1);
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::MyStructSeq sseq1;
        sseq1.push_back(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyStructSeqAsync(
                sseq1,
                [&p, &sseq1](const Test::MyStructSeq& s2, const Test::MyStructSeq& s3)
                {
                    test(s2 == sseq1);
                    test(s3 == sseq1);
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::MyStructMap smap1;
        smap1["a"] = s1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyStructMapAsync(
                smap1,
                [&p, &smap1](const Test::MyStructMap& s2, const Test::MyStructMap& s3)
                {
                    test(s2 == smap1);
                    test(s3 == smap1);
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::MyClassPtr c1 = make_shared<Test::MyClass>(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyClassAsync(
                c1,
                [&p, &c1](const Test::MyClassPtr& c2, const Test::MyClassPtr& c3)
                {
                    test(c2->ice_tuple() == c1->ice_tuple());
                    test(c3->ice_tuple() == c1->ice_tuple());
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::MyClassSeq cseq1;
        cseq1.push_back(c1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyClassSeqAsync(
                cseq1,
                [&p, c1](Test::MyClassSeq c2, Test::MyClassSeq c3)
                {
                    test(c2[0]->ice_tuple() == c1->ice_tuple());
                    test((c3[0]->ice_tuple() == c1->ice_tuple()));
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::MyClassMap cmap1;
        cmap1["a"] = c1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyClassMapAsync(
                cmap1,
                [&p, c1](Test::MyClassMap c2, Test::MyClassMap c3)
                {
                    test(c2["a"]->ice_tuple() == c1->ice_tuple());
                    test((c3["a"]->ice_tuple() == c1->ice_tuple()));
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyEnumAsync(
                Test::MyEnum::v1,
                [&p](Test::MyEnum v)
                {
                    test(v == Test::MyEnum::v1);
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        {
            Test::MyOtherStruct s;
            s.s = "MyOtherStruct";
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyOtherStructAsync(
                s,
                [&p](const Test::MyOtherStruct& v)
                {
                    test(v.s == "MyOtherStruct");
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyOtherClassAsync(
                make_shared<Test::MyOtherClass>("MyOtherClass"),
                [&p](const Test::MyOtherClassPtr& v)
                {
                    test(v->s == "MyOtherClass");
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }
    }

    {
        Test::Inner::Inner2::MyInterfacePrx i(communicator, "i2:" + helper->getTestEndpoint());

        Test::Inner::Inner2::MyStruct s1;
        s1.v = 0;
        Test::Inner::Inner2::MyStruct s2;
        Test::Inner::Inner2::MyStruct s3 = i->opMyStruct(s1, s2);
        test(s1 == s2);
        test(s1 == s3);

        Test::Inner::Inner2::MyStructSeq sseq1;
        sseq1.push_back(s1);
        Test::Inner::Inner2::MyStructSeq sseq2;
        Test::Inner::Inner2::MyStructSeq sseq3 = i->opMyStructSeq(sseq1, sseq2);
        test(sseq2 == sseq1);
        test(sseq3 == sseq1);

        Test::Inner::Inner2::MyStructMap smap1;
        smap1["a"] = s1;
        Test::Inner::Inner2::MyStructMap smap2;
        Test::Inner::Inner2::MyStructMap smap3 = i->opMyStructMap(smap1, smap2);
        test(smap2 == smap1);
        test(smap3 == smap1);

        Test::Inner::Inner2::MyClassPtr c1 = make_shared<Test::Inner::Inner2::MyClass>(s1);
        Test::Inner::Inner2::MyClassPtr c2;
        Test::Inner::Inner2::MyClassPtr c3 = i->opMyClass(c1, c2);
        test(c2->s == c1->s);
        test(c3->s == c1->s);

        Test::Inner::Inner2::MyClassSeq cseq1;
        cseq1.push_back(c1);
        Test::Inner::Inner2::MyClassSeq cseq2;
        Test::Inner::Inner2::MyClassSeq cseq3 = i->opMyClassSeq(cseq1, cseq2);
        test(cseq2[0]->s == c1->s);
        test(cseq3[0]->s == c1->s);

        Test::Inner::Inner2::MyClassMap cmap1;
        cmap1["a"] = c1;
        Test::Inner::Inner2::MyClassMap cmap2;
        Test::Inner::Inner2::MyClassMap cmap3 = i->opMyClassMap(cmap1, cmap2);
        test(cmap2["a"]->s == c1->s);
        test(cmap3["a"]->s == c1->s);
    }

    //
    // Future-Based Async Function
    //
    {
        Test::Inner::Inner2::MyInterfacePrx i(communicator, "i2:" + helper->getTestEndpoint());

        Test::Inner::Inner2::MyStruct s1;
        s1.v = 0;
        {
            auto result = i->opMyStructAsync(s1).get();
            test(std::get<0>(result) == s1);
            test(std::get<1>(result) == s1);
        }

        Test::Inner::Inner2::MyStructSeq sseq1;
        sseq1.push_back(s1);
        {
            auto result = i->opMyStructSeqAsync(sseq1).get();
            test(std::get<0>(result) == sseq1);
            test(std::get<1>(result) == sseq1);
        }

        Test::Inner::Inner2::MyStructMap smap1;
        smap1["a"] = s1;
        {
            auto result = i->opMyStructMapAsync(smap1).get();
            test(std::get<0>(result) == smap1);
            test(std::get<1>(result) == smap1);
        }

        Test::Inner::Inner2::MyClassPtr c1 = make_shared<Test::Inner::Inner2::MyClass>(s1);
        {
            auto result = i->opMyClassAsync(c1).get();
            test(std::get<0>(result)->ice_tuple() == c1->ice_tuple());
            test(std::get<1>(result)->ice_tuple() == c1->ice_tuple());
        }

        Test::Inner::Inner2::MyClassSeq cseq1;
        cseq1.push_back(c1);
        {
            auto result = i->opMyClassSeqAsync(cseq1).get();
            test(std::get<0>(result)[0]->ice_tuple() == c1->ice_tuple());
            test(std::get<1>(result)[0]->ice_tuple() == c1->ice_tuple());
        }

        Test::Inner::Inner2::MyClassMap cmap1;
        cmap1["a"] = c1;
        {
            auto result = i->opMyClassMapAsync(cmap1).get();
            test(std::get<0>(result)["a"]->ice_tuple() == c1->ice_tuple());
            test(std::get<1>(result)["a"]->ice_tuple() == c1->ice_tuple());
        }
    }

    //
    // Callback-Based Async Function
    //
    {
        Test::Inner::Inner2::MyInterfacePrx i(communicator, "i2:" + helper->getTestEndpoint());

        Test::Inner::Inner2::MyStruct s1;
        s1.v = 0;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyStructAsync(
                s1,
                [&p, &s1](Test::Inner::Inner2::MyStruct s2, Test::Inner::Inner2::MyStruct s3)
                {
                    test(s2 == s1);
                    test(s3 == s1);
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::MyStructSeq sseq1;
        sseq1.push_back(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyStructSeqAsync(
                sseq1,
                [&p, &sseq1](const Test::Inner::Inner2::MyStructSeq& s2, const Test::Inner::Inner2::MyStructSeq& s3)
                {
                    test(s2 == sseq1);
                    test(s3 == sseq1);
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::MyStructMap smap1;
        smap1["a"] = s1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyStructMapAsync(
                smap1,
                [&p, &smap1](const Test::Inner::Inner2::MyStructMap& s2, const Test::Inner::Inner2::MyStructMap& s3)
                {
                    test(s2 == smap1);
                    test(s3 == smap1);
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        auto c1 = make_shared<Test::Inner::Inner2::MyClass>(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyClassAsync(
                c1,
                [&p, &c1](
                    const shared_ptr<Test::Inner::Inner2::MyClass>& c2,
                    const shared_ptr<Test::Inner::Inner2::MyClass>& c3)
                {
                    test(c2->ice_tuple() == c1->ice_tuple());
                    test((c3->ice_tuple() == c1->ice_tuple()));
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::MyClassSeq cseq1;
        cseq1.push_back(c1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyClassSeqAsync(
                cseq1,
                [&p, c1](Test::Inner::Inner2::MyClassSeq c2, Test::Inner::Inner2::MyClassSeq c3)
                {
                    test(c2[0]->ice_tuple() == c1->ice_tuple());
                    test((c3[0]->ice_tuple() == c1->ice_tuple()));
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::MyClassMap cmap1;
        cmap1["a"] = c1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyClassMapAsync(
                cmap1,
                [&p, c1](Test::Inner::Inner2::MyClassMap c2, Test::Inner::Inner2::MyClassMap c3)
                {
                    test(c2["a"]->ice_tuple() == c1->ice_tuple());
                    test((c3["a"]->ice_tuple() == c1->ice_tuple()));
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }
    }

    {
        Test::Inner::MyInterfacePrx i(communicator, "i3:" + helper->getTestEndpoint());

        Test::Inner::Inner2::MyStruct s1;
        s1.v = 0;
        Test::Inner::Inner2::MyStruct s2;
        Test::Inner::Inner2::MyStruct s3 = i->opMyStruct(s1, s2);
        test(s1 == s2);
        test(s1 == s3);

        Test::Inner::Inner2::MyStructSeq sseq1;
        sseq1.push_back(s1);
        Test::Inner::Inner2::MyStructSeq sseq2;
        Test::Inner::Inner2::MyStructSeq sseq3 = i->opMyStructSeq(sseq1, sseq2);
        test(sseq2 == sseq1);
        test(sseq3 == sseq1);

        Test::Inner::Inner2::MyStructMap smap1;
        smap1["a"] = s1;
        Test::Inner::Inner2::MyStructMap smap2;
        Test::Inner::Inner2::MyStructMap smap3 = i->opMyStructMap(smap1, smap2);
        test(smap2 == smap1);
        test(smap3 == smap1);

        Test::Inner::Inner2::MyClassPtr c1 = make_shared<Test::Inner::Inner2::MyClass>(s1);
        Test::Inner::Inner2::MyClassPtr c2;
        Test::Inner::Inner2::MyClassPtr c3 = i->opMyClass(c1, c2);
        test(c2->s == c1->s);
        test(c3->s == c1->s);

        Test::Inner::Inner2::MyClassSeq cseq1;
        cseq1.push_back(c1);
        Test::Inner::Inner2::MyClassSeq cseq2;
        Test::Inner::Inner2::MyClassSeq cseq3 = i->opMyClassSeq(cseq1, cseq2);
        test(cseq2[0]->s == c1->s);
        test(cseq3[0]->s == c1->s);

        Test::Inner::Inner2::MyClassMap cmap1;
        cmap1["a"] = c1;
        Test::Inner::Inner2::MyClassMap cmap2;
        Test::Inner::Inner2::MyClassMap cmap3 = i->opMyClassMap(cmap1, cmap2);
        test(cmap2["a"]->s == c1->s);
        test(cmap3["a"]->s == c1->s);
    }

    //
    // Future-Based Async Function
    //
    {
        Test::Inner::MyInterfacePrx i(communicator, "i3:" + helper->getTestEndpoint());

        Test::Inner::Inner2::MyStruct s1;
        s1.v = 0;
        {
            auto result = i->opMyStructAsync(s1).get();
            test(std::get<0>(result) == s1);
            test(std::get<1>(result) == s1);
        }

        Test::Inner::Inner2::MyStructSeq sseq1;
        sseq1.push_back(s1);
        {
            auto result = i->opMyStructSeqAsync(sseq1).get();
            test(std::get<0>(result) == sseq1);
            test(std::get<1>(result) == sseq1);
        }

        Test::Inner::Inner2::MyStructMap smap1;
        smap1["a"] = s1;
        {
            auto result = i->opMyStructMapAsync(smap1).get();
            test(std::get<0>(result) == smap1);
            test(std::get<1>(result) == smap1);
        }

        Test::Inner::Inner2::MyClassPtr c1 = make_shared<Test::Inner::Inner2::MyClass>(s1);
        {
            auto result = i->opMyClassAsync(c1).get();
            test(std::get<0>(result)->ice_tuple() == c1->ice_tuple());
            test(std::get<1>(result)->ice_tuple() == c1->ice_tuple());
        }

        Test::Inner::Inner2::MyClassSeq cseq1;
        cseq1.push_back(c1);
        {
            auto result = i->opMyClassSeqAsync(cseq1).get();
            test(std::get<0>(result)[0]->ice_tuple() == c1->ice_tuple());
            test(std::get<1>(result)[0]->ice_tuple() == c1->ice_tuple());
        }

        Test::Inner::Inner2::MyClassMap cmap1;
        cmap1["a"] = c1;
        {
            auto result = i->opMyClassMapAsync(cmap1).get();
            test(std::get<0>(result)["a"]->ice_tuple() == c1->ice_tuple());
            test(std::get<1>(result)["a"]->ice_tuple() == c1->ice_tuple());
        }
    }

    //
    // Callback-Based Async Function
    //
    {
        Test::Inner::MyInterfacePrx i(communicator, "i3:" + helper->getTestEndpoint());

        Test::Inner::Inner2::MyStruct s1;
        s1.v = 0;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyStructAsync(
                s1,
                [&p, &s1](Test::Inner::Inner2::MyStruct s2, Test::Inner::Inner2::MyStruct s3)
                {
                    test(s2 == s1);
                    test(s3 == s1);
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::MyStructSeq sseq1;
        sseq1.push_back(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyStructSeqAsync(
                sseq1,
                [&p, &sseq1](const Test::Inner::Inner2::MyStructSeq& s2, const Test::Inner::Inner2::MyStructSeq& s3)
                {
                    test(s2 == sseq1);
                    test(s3 == sseq1);
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::MyStructMap smap1;
        smap1["a"] = s1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyStructMapAsync(
                smap1,
                [&p, &smap1](const Test::Inner::Inner2::MyStructMap& s2, const Test::Inner::Inner2::MyStructMap& s3)
                {
                    test(s2 == smap1);
                    test(s3 == smap1);
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        auto c1 = make_shared<Test::Inner::Inner2::MyClass>(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyClassAsync(
                c1,
                [&p, &c1](
                    const shared_ptr<Test::Inner::Inner2::MyClass>& c2,
                    const shared_ptr<Test::Inner::Inner2::MyClass>& c3)
                {
                    test(c2->ice_tuple() == c1->ice_tuple());
                    test(c3->ice_tuple() == c1->ice_tuple());
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::MyClassSeq cseq1;
        cseq1.push_back(c1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyClassSeqAsync(
                cseq1,
                [&p, c1](Test::Inner::Inner2::MyClassSeq c2, Test::Inner::Inner2::MyClassSeq c3)
                {
                    test(c2[0]->ice_tuple() == c1->ice_tuple());
                    test(c3[0]->ice_tuple() == c1->ice_tuple());
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::MyClassMap cmap1;
        cmap1["a"] = c1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyClassMapAsync(
                cmap1,
                [&p, c1](Test::Inner::Inner2::MyClassMap c2, Test::Inner::Inner2::MyClassMap c3)
                {
                    test(c2["a"]->ice_tuple() == c1->ice_tuple());
                    test(c3["a"]->ice_tuple() == c1->ice_tuple());
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }
    }

    {
        Inner::Test::Inner2::MyInterfacePrx i(communicator, "i4:" + helper->getTestEndpoint());

        Test::MyStruct s1;
        s1.v = 0;
        Test::MyStruct s2;
        Test::MyStruct s3 = i->opMyStruct(s1, s2);
        test(s1 == s2);
        test(s1 == s3);

        Test::MyStructSeq sseq1;
        sseq1.push_back(s1);
        Test::MyStructSeq sseq2;
        Test::MyStructSeq sseq3 = i->opMyStructSeq(sseq1, sseq2);
        test(sseq2 == sseq1);
        test(sseq3 == sseq1);

        Test::MyStructMap smap1;
        smap1["a"] = s1;
        Test::MyStructMap smap2;
        Test::MyStructMap smap3 = i->opMyStructMap(smap1, smap2);
        test(smap2 == smap1);
        test(smap3 == smap1);

        Test::MyClassPtr c1 = make_shared<Test::MyClass>(s1);
        Test::MyClassPtr c2;
        Test::MyClassPtr c3 = i->opMyClass(c1, c2);
        test(c2->s == c1->s);
        test(c3->s == c1->s);

        Test::MyClassSeq cseq1;
        cseq1.push_back(c1);
        Test::MyClassSeq cseq2;
        Test::MyClassSeq cseq3 = i->opMyClassSeq(cseq1, cseq2);
        test(cseq2[0]->s == c1->s);
        test(cseq3[0]->s == c1->s);

        Test::MyClassMap cmap1;
        cmap1["a"] = c1;
        Test::MyClassMap cmap2;
        Test::MyClassMap cmap3 = i->opMyClassMap(cmap1, cmap2);
        test(cmap2["a"]->s == c1->s);
        test(cmap3["a"]->s == c1->s);
    }

    //
    // Future-Based Async Function
    //
    {
        Inner::Test::Inner2::MyInterfacePrx i(communicator, "i4:" + helper->getTestEndpoint());

        Test::MyStruct s1;
        s1.v = 0;
        {
            auto result = i->opMyStructAsync(s1).get();
            test(std::get<0>(result) == s1);
            test(std::get<1>(result) == s1);
        }

        Test::MyStructSeq sseq1;
        sseq1.push_back(s1);
        {
            auto result = i->opMyStructSeqAsync(sseq1).get();
            test(std::get<0>(result) == sseq1);
            test(std::get<1>(result) == sseq1);
        }

        Test::MyStructMap smap1;
        smap1["a"] = s1;
        {
            auto result = i->opMyStructMapAsync(smap1).get();
            test(std::get<0>(result) == smap1);
            test(std::get<1>(result) == smap1);
        }

        Test::MyClassPtr c1 = make_shared<Test::MyClass>(s1);
        {
            auto result = i->opMyClassAsync(c1).get();
            test(std::get<0>(result)->ice_tuple() == c1->ice_tuple());
            test(std::get<1>(result)->ice_tuple() == c1->ice_tuple());
        }

        Test::MyClassSeq cseq1;
        cseq1.push_back(c1);
        {
            auto result = i->opMyClassSeqAsync(cseq1).get();
            test(std::get<0>(result)[0]->ice_tuple() == c1->ice_tuple());
            test(std::get<1>(result)[0]->ice_tuple() == c1->ice_tuple());
        }

        Test::MyClassMap cmap1;
        cmap1["a"] = c1;
        {
            auto result = i->opMyClassMapAsync(cmap1).get();
            test(std::get<0>(result)["a"]->ice_tuple() == c1->ice_tuple());
            test(std::get<1>(result)["a"]->ice_tuple() == c1->ice_tuple());
        }
    }

    //
    // Callback-Based Async Function
    //
    {
        Inner::Test::Inner2::MyInterfacePrx i(communicator, "i4:" + helper->getTestEndpoint());

        Test::MyStruct s1;
        s1.v = 0;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyStructAsync(
                s1,
                [&p, &s1](Test::MyStruct s2, Test::MyStruct s3)
                {
                    test(s2 == s1);
                    test(s3 == s1);
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::MyStructSeq sseq1;
        sseq1.push_back(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyStructSeqAsync(
                sseq1,
                [&p, &sseq1](const Test::MyStructSeq& s2, const Test::MyStructSeq& s3)
                {
                    test(s2 == sseq1);
                    test(s3 == sseq1);
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::MyStructMap smap1;
        smap1["a"] = s1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyStructMapAsync(
                smap1,
                [&p, &smap1](const Test::MyStructMap& s2, const Test::MyStructMap& s3)
                {
                    test(s2 == smap1);
                    test(s3 == smap1);
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        auto c1 = make_shared<Test::MyClass>(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyClassAsync(
                c1,
                [&p, &c1](const shared_ptr<Test::MyClass>& c2, const shared_ptr<Test::MyClass>& c3)
                {
                    test(c2->ice_tuple() == c1->ice_tuple());
                    test(c3->ice_tuple() == c1->ice_tuple());
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::MyClassSeq cseq1;
        cseq1.push_back(c1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyClassSeqAsync(
                cseq1,
                [&p, c1](Test::MyClassSeq c2, Test::MyClassSeq c3)
                {
                    test(c2[0]->ice_tuple() == c1->ice_tuple());
                    test(c3[0]->ice_tuple() == c1->ice_tuple());
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::MyClassMap cmap1;
        cmap1["a"] = c1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opMyClassMapAsync(
                cmap1,
                [&p, c1](Test::MyClassMap c2, Test::MyClassMap c3)
                {
                    test(c2["a"]->ice_tuple() == c1->ice_tuple());
                    test(c3["a"]->ice_tuple() == c1->ice_tuple());
                    p.set_value();
                },
                [&p](exception_ptr e) { p.set_exception(e); });

            try
            {
                f.get();
            }
            catch (const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }
    }

    Test::MyInterfacePrx i(communicator, "i1:" + helper->getTestEndpoint());
    i->shutdown();
}
