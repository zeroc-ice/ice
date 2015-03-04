// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Throughput.h>

#include <iomanip>

using namespace std;
using namespace Demo;

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


    //
    // To allow cross-language tests we may need to "warm up" the
    // server. The warm up is to ensure that any JIT compiler will
    // have converted any hotspots to native code. This ensures an
    // accurate throughput measurement.
    //
    if(throughput->needsWarmup())
    {
        throughput->startWarmup();

        ByteSeq emptyBytesBuf(1);
        emptyBytesBuf.resize(1);
        pair<const Ice::Byte*, const Ice::Byte*> emptyBytes;
        emptyBytes.first = &emptyBytesBuf[0];
        emptyBytes.second = emptyBytes.first + emptyBytesBuf.size();

        StringSeq emptyStrings(1);
        emptyStrings.resize(1);

        StringDoubleSeq emptyStructs(1);
        emptyStructs.resize(1);

        FixedSeq emptyFixed(1);
        emptyFixed.resize(1);

        cout << "warming up the server... " << flush;
        for(int i = 0; i < 10000; i++)
        {
            throughput->sendByteSeq(emptyBytes);
            throughput->sendStringSeq(emptyStrings);
            throughput->sendStructSeq(emptyStructs);
            throughput->sendFixedSeq(emptyFixed);

            throughput->recvByteSeq();
            throughput->recvStringSeq();
            throughput->recvStructSeq();
            throughput->recvFixedSeq();

            throughput->echoByteSeq(emptyBytes);
            throughput->echoStringSeq(emptyStrings);
            throughput->echoStructSeq(emptyStructs);
            throughput->echoFixedSeq(emptyFixed);
        }

        throughput->endWarmup();

        cout << " ok" << endl;
    }
    else
    {
        throughput->ice_ping(); // Initial ping to setup the connection.
    }

    menu();

    //
    // By default use byte sequence.
    //
    char currentType = '1';
    int seqSize = ByteSeqSize;

    char c;
    do
    {
        try
        {
            cout << "==> ";
            cin >> c;

            IceUtil::Time tm = IceUtil::Time::now(IceUtil::Time::Monotonic);
            const int repetitions = 1000;

            if(c == '1' || c == '2' || c == '3' || c == '4')
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
            else if(c == 't' || c == 'o' || c == 'r' || c == 'e')
            {
                switch(c)
                {
                    case 't':
                    case 'o':
                    {
                        cout << "sending";
                        break;
                    }

                    case 'r':
                    {
                        cout << "receiving";
                        break;
                    }

                    case 'e':
                    {
                        cout << "sending and receiving";
                        break;
                    }
                }

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

                if(c == 'o')
                {
                    cout << " as oneway";
                }
                
                cout << "..." << endl;
                
                for(int i = 0; i < repetitions; ++i)
                {
                    switch(currentType)
                    {
                        case '1':
                        {
                            switch(c)
                            {
                                case 't':
                                {
                                    throughput->sendByteSeq(byteArr);
                                    break;
                                }
                        
                                case 'o':
                                {
                                    throughputOneway->sendByteSeq(byteArr);
                                    break;
                                }
                        
                                case 'r':
                                {
                                    throughput->recvByteSeq();
                                    break;
                                }
                        
                                case 'e':
                                {
                                    throughput->echoByteSeq(byteArr);
                                    break;
                                }
                            }
                            break;
                        }

                        case '2':
                        {
                            switch(c)
                            {
                                case 't':
                                {
                                    throughput->sendStringSeq(stringSeq);
                                    break;
                                }
                        
                                case 'o':
                                {
                                    throughputOneway->sendStringSeq(stringSeq);
                                    break;
                                }
                        
                                case 'r':
                                {
                                    throughput->recvStringSeq();
                                    break;
                                }
                        
                                case 'e':
                                {
                                    throughput->echoStringSeq(stringSeq);
                                    break;
                                }
                            }
                            break;
                        }

                        case '3':
                        {
                            switch(c)
                            {
                                case 't':
                                {
                                    throughput->sendStructSeq(structSeq);
                                    break;
                                }
                        
                                case 'o':
                                {
                                    throughputOneway->sendStructSeq(structSeq);
                                    break;
                                }
                        
                                case 'r':
                                {
                                    throughput->recvStructSeq();
                                    break;
                                }
                        
                                case 'e':
                                {
                                    throughput->echoStructSeq(structSeq);
                                    break;
                                }
                            }
                            break;
                        }

                        case '4':
                        {
                            switch(c)
                            {
                                case 't':
                                {
                                    throughput->sendFixedSeq(fixedSeq);
                                    break;
                                }
                        
                                case 'o':
                                {
                                    throughputOneway->sendFixedSeq(fixedSeq);
                                    break;
                                }
                        
                                case 'r':
                                {
                                    throughput->recvFixedSeq();
                                    break;
                                }
                        
                                case 'e':
                                {
                                    throughput->echoFixedSeq(fixedSeq);
                                    break;
                                }
                            }
                            break;
                        }
                    }
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
        "\n"
        "select test to run:\n"
        "t: Send sequence as twoway\n"
        "o: Send sequence as oneway\n"
        "r: Receive sequence\n"
        "e: Echo (send and receive) sequence\n"
        "\n"
        "other commands:\n"
        "s: shutdown server\n"
        "x: exit\n"
        "?: help\n";
}
