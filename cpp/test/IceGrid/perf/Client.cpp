//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/Perf.h>

using namespace std;
using namespace Ice;
using namespace IcePerf;

inline void usage(const char* programName)
{
    cerr << "Usage: " << programName << " <key-to-resolve> [<key-to-resolve> ...]" << endl; 
}


template<class T>
bool
printIf(const Ice::ObjectPtr& obj, ostream& os)
{
    T* t = dynamic_cast<T*>(obj.get());
    if(t != 0)
    {
	cout << "(" << t->value << ")";
	return true;
    }
    return false;
}

bool
printIfLoadAvg(const Ice::ObjectPtr& obj, ostream& os)
{
    LoadAvg* t = dynamic_cast<LoadAvg*>(obj.get());
    if(t != 0)
    {
	cout << "(";
	for(size_t i = 0; i < t->value.size(); ++i)
	{
	    cout << t->value[i] << " ";
	}
	cout << ")";
	return true;
    }
    return false;
}


int
main(int argc, char* argv[])
{
    if(argc < 2)
    {
	usage(argv[0]);
	return 1;
    }

    StringSeq keys;
    for(int i = 1; i < argc; ++i)
    {
	keys.push_back(argv[i]);
    }
    PerfDataSeq dataSeq = getPerfData(keys);
    
    for(size_t i = 0; i < dataSeq.size(); ++i)
    {
	const string& typeId = dataSeq[i]->ice_id();

	cout << keys[i] << ": " << typeId << " ";
	
	bool printed = 
	printIf<BoxedString>(dataSeq[i], cout)
	    || printIf<BoxedInt>(dataSeq[i], cout)
	    || printIf<BoxedLong>(dataSeq[i], cout)
	    || printIf<BoxedFloat>(dataSeq[i], cout)
	    || printIfLoadAvg(dataSeq[i], cout);
	
	cout << endl;	
    }
    return 0;
}
