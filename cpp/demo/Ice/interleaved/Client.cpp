// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Throughput.h>

#include <iomanip>

#include <list>

using namespace std;
using namespace Demo;

class Callback : public IceUtil::Shared
{
public:

    Callback()
    {
    }

    void response(const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&)
    {
    }

    void responseSS(const StringSeq&)
    {
    }

    void responseTS(const StringDoubleSeq&)
    {
    }

    void responseFS(const FixedSeq&)
    {
    }

    void exception(const ::Ice::Exception& ex)
    {
        cerr << ex << endl;
    }
};

typedef IceUtil::Handle<Callback> CallbackPtr;

class ThroughputClient : public Ice::Application
{
public:

    ThroughputClient();
    virtual int run(int, char*[]);

private:

    void menu();
};

int
main(int argc, char* argv[])
{
    ThroughputClient app;
    return app.main(argc, argv, "config.client");
}

ThroughputClient::ThroughputClient() :
    //
    // Since this is an interactive demo we don't want any signal
    // handling.
    //
    Ice::Application(Ice::NoSignalHandling)
{
}

int
ThroughputClient::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    ThroughputPrx throughput = ThroughputPrx::checkedCast(communicator()->propertyToProxy("Throughput.Proxy"));
    if(!throughput)
    {
        cerr << argv[0] << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    ThroughputPrx throughputOneway = ThroughputPrx::uncheckedCast(throughput->ice_oneway());

    ByteSeq byteSeq(ByteSeqSize);
    pair<const Ice::Byte*, const Ice::Byte*> byteArr;
    byteArr.first = &byteSeq[0];
    byteArr.second = byteArr.first + byteSeq.size();

    StringSeq stringSeq(StringSeqSize, "hello");

    StringDoubleSeq structSeq(StringDoubleSeqSize);
    int i;
    for(i = 0; i < StringDoubleSeqSize; ++i)
    {
        structSeq[i].s = "hello";
        structSeq[i].d = 3.14;
    }

    FixedSeq fixedSeq(FixedSeqSize);
    for(i = 0; i < FixedSeqSize; ++i)
    {
        fixedSeq[i].i = 0;
        fixedSeq[i].j = 0;
        fixedSeq[i].d = 0;
    }

    throughput->ice_ping(); // Initial ping to setup the connection.

    menu();

    //
    // By default use byte sequence.
    //
    char currentType = '1';
    int seqSize = ByteSeqSize;
    // The maximum number of outstanding requests. -1 means
    // unlimited. If the number of oustanding requests is unlimited
    // and the server is slower than the client in processing the
    // requests, high memory consumption will result.
    int maxOutstanding = 2;

    CallbackPtr cb = new Callback;

    Callback_Throughput_echoByteSeqPtr byteSeqCB = newCallback_Throughput_echoByteSeq(
        cb, &Callback::response, &Callback::exception);
    Callback_Throughput_echoStringSeqPtr stringSeqCB = newCallback_Throughput_echoStringSeq(
        cb, &Callback::responseSS, &Callback::exception);
    Callback_Throughput_echoStructSeqPtr structSeqCB = newCallback_Throughput_echoStructSeq(
        cb, &Callback::responseTS, &Callback::exception);
    Callback_Throughput_echoFixedSeqPtr fixedSeqCB = newCallback_Throughput_echoFixedSeq(
        cb, &Callback::responseFS, &Callback::exception);

    char c;
    do
    {
        try
        {
            cout << "==> ";
            cin >> c;

            IceUtil::Time tm = IceUtil::Time::now(IceUtil::Time::Monotonic);
            const int repetitions = 1000;

            if(c == 'o')
            {
                if(maxOutstanding == -1)
                {
                    maxOutstanding = 2;
                    cout << "outstanding requests are now limited to 2." << endl;
                }
                else
                {
                    maxOutstanding = -1;
                    cout << "outstanding requests are now unlimited." << endl;
                }
            }
            else if(c == '1' || c == '2' || c == '3' || c == '4')
            {
                currentType = c;
                switch(c)
                {
                    case '1':
                    {
                        cout << "using byte sequences" << endl;
                        seqSize = ByteSeqSize;
                        break;
                    }

                    case '2':
                    {
                        cout << "using string sequences" << endl;
                        seqSize = StringSeqSize;
                        break;
                    }

                    case '3':
                    {
                        cout << "using variable-length struct sequences" << endl;
                        seqSize = StringDoubleSeqSize;
                        break;
                    }

                    case '4':
                    {
                        cout << "using fixed-length struct sequences" << endl;
                        seqSize = FixedSeqSize;
                        break;
                    }
                }
            }
            else if(c == 'e')
            {
                cout << "sending and receiving";

                cout << ' ' << repetitions;
                switch(currentType)
                {
                    case '1':
                    {
                        cout << " byte";
                        break;
                    }

                    case '2':
                    {
                        cout << " string";
                        break;
                    }

                    case '3':
                    {
                        cout << " variable-length struct";
                        break;
                    }

                    case '4':
                    {
                        cout << " fixed-length struct";
                        break;
                    }
                }
                cout << " sequences of size " << seqSize;

                cout << "..." << endl;
                
                list<Ice::AsyncResultPtr> results;
                for(int i = 0; i < repetitions; ++i)
                {
                    // Start the AMI request.
                    Ice::AsyncResultPtr r;
                    switch(currentType)
                    {
                    case '1':
                        r = throughput->begin_echoByteSeq(byteArr, byteSeqCB);
                        break;

                    case '2':
                        r = throughput->begin_echoStringSeq(stringSeq, stringSeqCB);
                        break;

                    case '3':
                        r = throughput->begin_echoStructSeq(structSeq, structSeqCB);
                        break;

                    case '4':
                        r = throughput->begin_echoFixedSeq(fixedSeq, fixedSeqCB);
                        break;
                    }
                    // For flow control purposes we want to restrict
                    // the number of sending messages to one. This
                    // avoids unnecessarily accumulating buffers in
                    // the Ice runtime, keeping memory usage low.
                    r->waitForSent();
                    results.push_back(r);

                    // Remove any completed requests from the list of
                    // pending results.
                    list<Ice::AsyncResultPtr>::iterator p = results.begin();
                    while(p != results.end())
                    {
                        if((*p)->isCompleted())
                        {
                            p = results.erase(p);
                        }
                        else
                        {
                            ++p;
                        }
                    }

                    // This avoids too many outstanding requests. This
                    // is desirable if the server doesn't limit the
                    // number of threads, or the server process
                    // requests slower than then client can send them.
                    while(maxOutstanding != -1 && static_cast<int>(results.size()) > maxOutstanding)
                    {
                        r = results.front();
                        results.pop_front();
                        r->waitForCompleted();
                    }
                }
                // Before we're complete we must wait for all
                // responses to be received.
                while(!results.empty())
                {
                    Ice::AsyncResultPtr r = results.front();
                    results.pop_front();
                    r->waitForCompleted();
                }

                tm = IceUtil::Time::now(IceUtil::Time::Monotonic) - tm;
                cout << "time for " << repetitions << " sequences: " << tm * 1000 << "ms" << endl;
                cout << "time per sequence: " << tm * 1000 / repetitions << "ms" << endl;
                int wireSize = 0;
                switch(currentType)
                {
                    case '1':
                    {
                        wireSize = 1;
                        break;
                    }
                    case '2':
                    {
                        wireSize = static_cast<int>(stringSeq[0].size());
                        break;
                    }
                    case '3':
                    {
                        wireSize = static_cast<int>(structSeq[0].s.size());
                        wireSize += 8; // Size of double on the wire.
                        break;
                    }
                    case '4':
                    {
                        wireSize = 16; // Size of two ints and a double on the wire.
                        break;
                    }
                }
                double mbit = repetitions * seqSize * wireSize * 8.0 / tm.toMicroSeconds();
                if(c == 'e')
                {
                    mbit *= 2;
                }
                cout << "throughput: " << setprecision(5) << mbit << "Mbps" << endl;
            }
            else if(c == 's')
            {
                throughput->shutdown();
            }
            else if(c == 'x')
            {
                // Nothing to do
            }
            else if(c == '?')
            {
                menu();
            }
            else
            {
                cout << "unknown command `" << c << "'" << endl;
                menu();
            }
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
        }
    }
    while(cin.good() && c != 'x');

    return EXIT_SUCCESS;
}

void
ThroughputClient::menu()
{
    cout <<
        "usage:\n"
        "\n"
        "toggle type of data to send:\n"
        "1: sequence of bytes (default)\n"
        "2: sequence of strings (\"hello\")\n"
        "3: sequence of structs with a string (\"hello\") and a double\n"
        "4: sequence of structs with two ints and a double\n"
        "o: toggle the limit of outstanding requests\n"
        "\n"
        "select test to run:\n"
        "e: Echo (send and receive) sequence\n"
        "\n"
        "other commands:\n"
        "s: shutdown server\n"
        "x: exit\n"
        "?: help\n";
}
