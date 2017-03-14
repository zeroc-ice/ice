// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/Random.h>
#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>
#include <fstream>

using namespace std;
using namespace Test;

namespace
{

int num = 0;
::IceUtil::Mutex* numMutex = 0;

class Init
{
public:

    Init()
    {
        numMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete numMutex;
        numMutex = 0;
    }
};

Init init;

}

static void
incNum()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(numMutex);
    ++num;
}

static void
decNum()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(numMutex);
    --num;
}

static int
getNum()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(numMutex);
    return num;
}

struct N : public C
{
    N()
    {
        incNum();
    }

    ~N()
    {
        decNum();
    }
};

typedef ::IceInternal::Handle<N> NPtr;

struct N2 : public C2
{
    N2()
    {
        incNum();
    }

    ~N2()
    {
        decNum();
    }
};

typedef ::IceInternal::Handle<N2> N2Ptr;

struct NN : public Node
{
    NN()
    {
        incNum();
    }

    ~NN()
    {
        decNum();
    }
};

typedef ::IceInternal::Handle<NN> NNPtr;

struct NL : public Leaf
{
    NL()
    {
        incNum();
    }

    ~NL()
    {
        decNum();
    }
};

typedef ::IceInternal::Handle<NL> NLPtr;

class TestHaveCycles : public IceInternal::GCVisitor
{
public:

    virtual bool visit(IceInternal::GCObject* obj)
    {
        if(obj->__hasFlag(IceInternal::GCObject::Visiting))
        {
            return false;
        }
        test(obj->__hasFlag(IceInternal::GCObject::CycleMember) && obj->__hasFlag(IceInternal::GCObject::Collectable));

        obj->__setFlag(IceInternal::GCObject::Visiting);
        obj->_iceGcVisitMembers(*this);
        obj->__clearFlag(IceInternal::GCObject::Visiting);
        return false;
    }
};


class MyApplication : public Ice::Application
{
public:

    MyApplication();
    virtual int run(int, char* []);
};

MyApplication::MyApplication()
    : Ice::Application(Ice::ICE_ENUM(SignalPolicy, NoSignalHandling))
{
}

int
MyApplication::run(int argc, char* argv[])
{
    cout << "testing single instance... " << flush;
    {
        NPtr n = new N;
        test(getNum() == 1);
    }
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing single instance cycle... " << flush;
    {
        NPtr n = new N;
        n->left = n;
        test(getNum() == 1);
        n->ice_collectable(true);
        TestHaveCycles().visit(n.get());
    }
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing single instance cycle with double pointers... " << flush;
    {
        NPtr n = new N;
        n->left = n;
        n->right = n;
        n->ice_collectable(true);
        TestHaveCycles().visit(n.get());
        test(getNum() == 1);
    }
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing double instance cycle... " << flush;
    {
        NPtr n1 = new N;
        NPtr n2 = new N;
        n1->left = n2;
        n2->left = n1;
        n1->ice_collectable(true);
        TestHaveCycles().visit(n1.get());
        n1 = 0;
        test(n2->left->left == n2);
        test(getNum() == 2);
    }
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing double instance cycle with double pointers... " << flush;
    {
        NPtr n1 = new N;
        NPtr n2 = new N;
        n1->left = n2;
        n2->left = n1;
        n1->right = n2;
        n2->right = n1;
        n1->ice_collectable(true);
        TestHaveCycles().visit(n1.get());
        n1 = 0;
        test(n2->left->left == n2);
        test(getNum() == 2);
    }
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing double instance cycle with looped pointers... " << flush;
    {
        NPtr n1 = new N;
        NPtr n2 = new N;
        n1->left = n2;
        n2->left = n1;
        n1->right = n1;
        n2->right = n2;
        n1->ice_collectable(true);
        TestHaveCycles().visit(n1.get());
        test(getNum() == 2);
        n1 = 0;
        test(n2->left->left == n2);
    }
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing triple instance cycle... " << flush;
    {
        NPtr n1 = new N;
        NPtr n2 = new N;
        NPtr n3 = new N;
        n1->left = n2;
        n2->left = n3;
        n3->left = n1;
        n1->ice_collectable(true);
        TestHaveCycles().visit(n1.get());
        n1 = 0;
        test(n2->left->left->left == n2);
        test(getNum() == 3);
    }
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing triple instance cycle with double pointers... " << flush;
    {
        NPtr n1 = new N;
        NPtr n2 = new N;
        NPtr n3 = new N;
        n1->left = n2;
        n2->left = n3;
        n3->left = n1;
        n1->right = n2;
        n2->right = n3;
        n3->right = n1;
        test(getNum() == 3);
        n1->ice_collectable(true);
        TestHaveCycles().visit(n1.get());
    }
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing triple instance cycle with opposing pointers... " << flush;
    {
        NPtr n1 = new N;
        NPtr n2 = new N;
        NPtr n3 = new N;
        n1->left = n2;
        n2->left = n3;
        n3->left = n1;
        n1->right = n3;
        n2->right = n1;
        n3->right = n2;
        n1->ice_collectable(true);
        TestHaveCycles().visit(n1.get());
        test(getNum() == 3);
    }
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing cycle with trailing instances... " << flush;
    NPtr n;
    {
        NPtr n1 = new N;
        NPtr n2 = new N;
        NPtr n3 = new N;
        NPtr n4 = new N;
        n1->left = n2;
        n2->left = n1;
        n2->right = n3;
        n3->left = n4;
        n = n3;
        n1->ice_collectable(true);
        TestHaveCycles().visit(n1.get());
        test(getNum() == 4);
    }
    test(getNum() == 2);
    n = 0;
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing cycle with trailing instances and trailing cycle... " << flush;
    {
        NPtr n1 = new N;
        NPtr n2 = new N;
        NPtr n3 = new N;
        NPtr n4 = new N;
        NPtr n5 = new N;
        NPtr n6 = new N;
        n1->left = n2;
        n2->left = n1;
        n2->right = n3;
        n3->left = n4;
        n4->right = n5;
        n5->right = n6;
        n6->right = n5;
        n1->ice_collectable(true);
        TestHaveCycles().visit(n1.get());
        n = n4;
        test(getNum() == 6);
    }
    test(getNum() == 3);
    n = 0;
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing complex cycles... " << flush;
    {
        NPtr n1 = new N;
        NPtr n2 = new N;
        NPtr n3 = new N;
        NPtr n4 = new N;
        NPtr n5 = new N;
        NPtr n6 = new N;
        NPtr n7 = new N;
        NPtr n8 = new N;

        n1->left = n2;
        n2->left = n3;
        n2->right = n4;
        n3->left = n8;
        n3->right = n5;
        n4->left = n5;
        n5->left = n6;
        n6->left = n5;
        n6->right = n7;
        n7->left = n3;
        n8->left = n1;
        n1->ice_collectable(true);
        TestHaveCycles().visit(n1.get());

        n4->ice_collectable(false);

        n4->ice_collectable(true);
        TestHaveCycles().visit(n4.get());

        n = n4;
        test(getNum() == 8);
    }
    test(getNum() == 8);
    n = 0;
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing random graphs... " << flush;
    {
        for(int i = 10; i <= 150; i += 10) // random graphs with 10 to 150 nodes
        {
            {
                vector<NPtr> nodes;
                for(int j = 0; j < i; ++j)
                {
                    nodes.push_back(new N());
                }
                for(int j = 0; j < i; ++j)
                {
                    nodes[j]->left = nodes[IceUtilInternal::random(i)];
                    nodes[j]->right = nodes[IceUtilInternal::random(i)];
                }
                for(int j = 0; j < i; ++j)
                {
                    nodes[j]->ice_collectable(true);
                }
                test(getNum() == i);
            }
            test(getNum() == 0);
        }
    }
    cout << "ok" << endl;

    cout << "testing sequence element cycle... " << flush;
    {
        CSeq cs;
        cs.push_back(new N);
        cs.push_back(new N);
        cs[0]->left = cs[1];
        cs[1]->left = cs[0];
        cs[0]->ice_collectable(true);
        cs[1]->ice_collectable(true);
        test(getNum() == 2);
    }
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing dictionary element cycle... " << flush;
    {
        CDict cd;
        NPtr n1 = new N;
        NPtr n2 = new N;
        n1->left = n2;
        n2->left = n1;
        cd[0] = n1;
        cd[1] = n2;
        cd[0]->ice_collectable(true);
        cd[1]->ice_collectable(true);
        test(getNum() == 2);
    }
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing sequence of struct cycle... " << flush;
    {
        SSeq ss;
        S s;

        ss.push_back(s);
        ss.push_back(s);
        ss[0].theC = new N;
        ss[1].theC = new N;
        ss[0].theC->left = ss[1].theC;
        ss[1].theC->left = ss[0].theC;
        ss[0].theC->ice_collectable(true);
        ss[1].theC->ice_collectable(true);
        test(getNum() == 2);
    }
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing sequence of struct of dictionary cycle... " << flush;
    {
        N2Ptr n2 = new N2;
        S2 s2;
        n2->theS2Seq.push_back(s2);
        n2->theS2Seq.push_back(s2);
        n2->theS2Seq[0].theC2Dict[0] = n2;
        n2->theS2Seq[0].theC2Dict[1] = n2;
        n2->theS2Seq[1].theC2Dict[0] = n2;
        n2->theS2Seq[1].theC2Dict[1] = n2;
        n2->ice_collectable(true);
        test(getNum() == 1);
    }
    test(getNum() == 0);
    cout << "ok" << endl;

    cout << "testing leaf nodes... " << flush;

    {
        NNPtr nn = new NN;
        nn->l = new NL;
        test(getNum() == 2);
    }
    test(getNum() == 0);

    {
        NLPtr p;
        {
            NNPtr nn = new NN;
            p = new NL;
            nn->l = p;
            test(getNum() == 2);
        }
        test(getNum() == 1);
    }
    test(getNum() == 0);

    {
        NNPtr nn = new NN;
        NLPtr nl = new NL;
        nn->l = nl;
        nn->n = nn;
        nn->ice_collectable(true);
        TestHaveCycles().visit(nn.get());
        test(getNum() == 2);
    }
    test(getNum() == 0);

    {
        NNPtr nn1 = new NN;
        NNPtr nn2 = new NN;
        NLPtr nl = new NL;
        nn1->l = nl;
        nn1->n = nn2;
        nn2->l = nl;
        nn2->n = nn1;
        nn1->ice_collectable(true);
        TestHaveCycles().visit(nn1.get());
        test(getNum() == 3);
    }
    test(getNum() == 0);

    {
        NLPtr nl = new NL;
        test(getNum() == 1);
    }
    test(getNum() == 0);

    {
        NNPtr nn1 = new NN;
        nn1->n = new NN;
        test(getNum() == 2);
    }
    test(getNum() == 0);

    cout << "ok" << endl;
    return 0;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL(false);
#endif
    MyApplication app;
    return app.main(argc, argv);
}
