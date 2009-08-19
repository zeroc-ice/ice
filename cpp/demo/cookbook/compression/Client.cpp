// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Compression.h>
#include <fstream>

using namespace std;
using namespace Compression;

//
// Implementation of the Ice::Stats interface.
//
class StatsI : public Ice::Stats
{
public:

    StatsI() : _total(0) {}

    virtual void bytesSent(const string&, Ice::Int sz)
    {
        _total += sz;
    }

    virtual void bytesReceived(const string&, Ice::Int)
    {
    }

    void reset()
    {
        _total = 0;
    }

    Ice::Int total() const
    {
        return _total;
    }

private:

    Ice::Int _total;
};
typedef IceUtil::Handle<StatsI> StatsIPtr;

class Client : public Ice::Application
{
public:

    Client() : _stats(new StatsI)
    {
    }

    Ice::StatsPtr stats() const
    {
        return _stats;
    }

    virtual int run(int, char*[]);

private:

    StatsIPtr _stats;
};

int
main(int argc, char* argv[])
{
    Client app;

    Ice::InitializationData initData;
    initData.properties = Ice::createProperties();
    initData.properties->load("config.client");
    initData.stats = app.stats();
    return app.main(argc, argv, initData);
}

int
Client::run(int argc, char* argv[])
{
    if(argc == 1)
    {
        cerr << "Usage: " << argv[0] << " file" << endl;
        return 1;
    }

    //
    // Read the input file into a byte sequence.
    //
    ifstream in;
    in.open(argv[1], ios::binary);
    if(!in.good())
    {
        cerr << argv[0] << ": unable to open file `" << argv[1] << "'" << endl;
        return 1;
    }

    in.seekg(0, ios::end);
    int len = in.tellg();
    in.seekg(0, ios::beg);

    ByteSeq source;
    source.resize(len);
    in.read(reinterpret_cast<char*>(&source[0]), len);

    //
    // Create our proxies.
    //
    Ice::PropertiesPtr properties = communicator()->getProperties();
    const char* proxyProperty = "Compression.Proxy";
    std::string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
        cerr << argv[0] << ": property `" << proxyProperty << "' not set" << endl;
        return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator()->stringToProxy(proxy);
    ReceiverPrx uncompressed = ReceiverPrx::checkedCast(base->ice_compress(false));
    if(!uncompressed)
    {
        cerr << argv[0] << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }
    ReceiverPrx compressed = ReceiverPrx::uncheckedCast(base->ice_compress(true));

    const int repetitions = 10;
    int sz = 200;

    while(true)
    {
        cout << "time for " << sz << " bytes:" << endl;

        ByteSeq seq;
        seq.resize(sz);
        copy(source.begin(), source.begin() + sz, seq.begin());

        IceUtil::Time tm;
        int i;

        //
        // Measure uncompressed latency.
        //
        _stats->reset();
        tm = IceUtil::Time::now();
        for(i = 0; i < repetitions; ++i)
        {
            uncompressed->sendData(seq);
        }
        tm = IceUtil::Time::now() - tm;
        cout << "  uncompressed = " << tm * 1000 / repetitions << "ms" << flush;
        Ice::Int uncompressedTotal = _stats->total();

        //
        // Measure compressed latency.
        //
        _stats->reset();
        tm = IceUtil::Time::now();
        for(i = 0; i < repetitions; ++i)
        {
            compressed->sendData(seq);
        }
        tm = IceUtil::Time::now() - tm;
        cout << "\tcompressed = " << tm * 1000 / repetitions << "ms" << flush;
        Ice::Int compressedTotal = _stats->total();
        cout << "\tcompression = " << compressedTotal * 100.0 / uncompressedTotal << "%" << endl;

        if(sz == len)
        {
            break;
        }
        else
        {
            sz *= 10;
            if(sz > len)
            {
                sz = len;
            }
        }
    }

    cout << "done" << endl;

    return EXIT_SUCCESS;
}
