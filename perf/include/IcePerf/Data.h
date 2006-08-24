// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPERF_DATA_H
#define ICEPERF_DATA_H

#ifdef _ICEE
#   include <IceE/IceE.h>
#else
#   include <IceUtil/Time.h>
#   include <Ice/Config.h>
#endif

#include <ostream>
#include <string>

namespace IcePerf
{
class TestPrinter
{
public:

    virtual ~TestPrinter() {}

    //
    // Write the test data to an output stream in a format that is
    // interpretable by Python.
    //
    virtual void fmt(std::ostream&, const std::string& product, const std::string& name, const IceUtil::Time& duration, 
		     const Ice::Int& repetitions, const Ice::Int& payloadSize, const int argc, char* argv[]);
};

//
// Ideally this code would be in a library, but it doesn't work that way
// because there are name clashes and mix and match problems with Ice and
// IceE. In short, you cannot have an IceE application that uses Ice
// libraries. So we end up with a cut-n-paste kind of code reuse.
//
void
IcePerf::TestPrinter::fmt(std::ostream& output, const std::string& product, const std::string& name, const IceUtil::Time& duration, 
			  const Ice::Int& repetitions, const Ice::Int& payloadSize, const int argc, char* argv[])
{
    output << "{";
    output << "'product' : '" << product << "', ";
    output << "'name' : '" << name << "', ";
    output << "'duration' : " << duration.toMilliSeconds() << ", ";
    output << "'repetitions' : "  << repetitions << ", ";
    output << "'payload' : " << payloadSize  << ", ";
    output << "'latency' : " << duration.toMilliSecondsDouble()/repetitions << ", ";
    output << "'throughput' : " << ((double)payloadSize * repetitions / (1024 * 1024)) / duration.toMilliSecondsDouble() * 1000.0 << ", ";
    output << "'args' : '" ;
    int i;
    for(i = 0; i < argc; ++i)
    {
	if(i > 0)
	    output << ' ';
	output << argv[i];
    }
    output << "'";
    output << "}";
}

}

#endif /* ICEPERF_DATA_H */
