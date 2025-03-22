// Copyright (c) ZeroC, Inc.
#include <DataStorm/DataStorm.h>

using namespace DataStorm;
using namespace std;

void
testDuplicateSymbols()
{
    // Make sure these don't cause duplicate symbols errors on link
    ostringstream os;
    os << SampleEvent::Add;
    os << SampleEventSeq();
    Sample<string, string>* sample = nullptr;
    os << *sample;
}
