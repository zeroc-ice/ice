// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/Random.h"
#include "Ice/UUID.h"
#include "TestHelper.h"

#include <mutex>
#include <set>
#include <stdexcept>
#include <vector>

using namespace Ice;
using namespace std;

namespace
{
    mutex staticMutex;
}

inline void
usage(const char* myName)
{
    cerr << "Usage: " << myName << " [number of UUIDs to generate] [number of threads]" << endl;
}

template<typename T, typename GenerateFunc> class InsertThread
{
public:
    using ItemSet = set<T>;

    InsertThread(int threadId, ItemSet& itemSet, GenerateFunc func, long howMany, bool verbose)
        : _threadId(threadId),
          _itemSet(itemSet),
          _func(func),
          _howMany(howMany),
          _verbose(verbose)
    {
    }

    void run()
    {
        for (long i = 0; i < _howMany; i++)
        {
            T item = _func();

            lock_guard lock(staticMutex);
            pair<typename ItemSet::iterator, bool> ok = _itemSet.insert(item);
            if (!ok.second)
            {
                cerr << "******* iteration " << i << endl;
                cerr << "******* Duplicate item: " << *ok.first << endl;
            }

            test(ok.second);

            if (_verbose && i > 0 && (i % 100000 == 0))
            {
                cout << "Thread " << _threadId << ": generated " << i << " UUIDs." << endl;
            }
        }
    }

private:
    int _threadId;
    ItemSet& _itemSet;
    GenerateFunc _func;
    long _howMany;
    bool _verbose;
};

struct GenerateUUID
{
    string operator()() { return generateUUID(); }
};

struct GenerateRandomString
{
    string operator()()
    {
        string s;
        s.resize(21);
        char buf[21];
        IceInternal::generateRandom(buf, sizeof(buf));
        for (unsigned int i = 0; i < sizeof(buf); ++i)
        {
            s[i] =
                33 + static_cast<unsigned char>(buf[i]) % (127 - 33); // We use ASCII 33-126 (from ! to ~, w/o space).
        }
        // cerr << s << endl;
        return s;
    }
};

struct GenerateRandomInt
{
public:
    int operator()() { return static_cast<int>(IceInternal::random()); }
};

template<typename T, typename GenerateFunc>
void
runTest(int threadCount, GenerateFunc func, long howMany, bool verbose, const string& name)
{
    cout << "Generating " << howMany << " " << name << "s using " << threadCount << " thread";
    if (threadCount > 1)
    {
        cout << "s";
    }
    cout << "... ";

    if (verbose)
    {
        cout << endl;
    }
    else
    {
        cout << flush;
    }

    set<T> itemSet;

    vector<thread> threads;

    auto start = chrono::steady_clock::now();
    for (int i = 0; i < threadCount; i++)
    {
        auto inserter = make_shared<InsertThread<T, GenerateFunc>>(i, itemSet, func, howMany / threadCount, verbose);
        threads.emplace_back(thread([inserter] { inserter->run(); }));
    }
    for (auto& p : threads)
    {
        if (p.joinable())
        {
            p.join();
        }
    }
    auto finish = chrono::steady_clock::now();

    cout << "ok" << endl;

    if (verbose)
    {
        cout << "Each " << name << " took an average of "
             << static_cast<double>(chrono::duration_cast<chrono::microseconds>(finish - start).count()) /
                    static_cast<double>(howMany)
             << " microseconds to generate and insert into a set." << endl;
    }
}

class Client : public Test::TestHelper
{
public:
    void run(int argc, char* argv[]) override;
};

void
Client::run(int argc, char* argv[])
{
    long howMany = 300000;
    int threadCount = 3;
    bool verbose = false;

    if (argc > 3)
    {
        usage(argv[0]);
        throw std::invalid_argument("too many arguments");
    }

    if (argc > 1)
    {
        howMany = stol(argv[1]);
        if (howMany == 0)
        {
            usage(argv[0]);
            throw invalid_argument("argv[1] howMany is not a number");
        }
    }

    if (argc > 2)
    {
        threadCount = stoi(argv[2]);
        if (threadCount <= 0)
        {
            usage(argv[0]);
            throw invalid_argument("argv[2] threadCount is not a number");
        }
        verbose = true;
    }

    runTest<string, GenerateUUID>(threadCount, GenerateUUID(), howMany, verbose, "UUID");
    runTest<string, GenerateRandomString>(threadCount, GenerateRandomString(), howMany, verbose, "string");
}

DEFINE_TEST(Client);
