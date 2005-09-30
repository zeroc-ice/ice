// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


//
// Linux implementation of IcePerf::getPerfData
//

#include <IceGrid/Perf.h>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <limits>

using namespace std;
using namespace Ice;
using namespace IcePerf;

namespace
{

typedef map<string, PerfDataPtr> ParsedFile;
typedef map<string, ParsedFile> ParsedFileMap;

//
// Cache for parsed files with constant values, such as osname, cpuinfo
//
ParsedFileMap globalCache;

//
// A parser object can parse the given key and populate the
// corresponding entry in the ParsedFileMap. If this entry
// already exists, it is replaced.
//
class Parser
{
public:

    virtual ~Parser() {};
    virtual ParsedFileMap::const_iterator parse(const string&, ParsedFileMap&) = 0;
};

//
// A map to quickly locate a parser given a key
//
typedef map<string, Parser*> ParserMap;

//
// We only need one such map :). Variable process-ids are represented
// by the string $pid.
//
ParserMap parserMap;


//
// The static instance od Init initialized parserMap and other
// one-off data structures.
//
class Init
{
public:
    Init();
    ~Init();
};

Init init;


//
// Specialized parsers
//

class SimpleParser : public Parser
{
public:
    virtual ParsedFileMap::const_iterator parse(const string&, ParsedFileMap&);
};

class ProcessStatParser : public Parser
{
public:
    virtual ParsedFileMap::const_iterator parse(const string&, ParsedFileMap&);
};

class CpuInfoParser : public Parser
{
public:
    virtual ParsedFileMap::const_iterator parse(const string&, ParsedFileMap&);
};

class LoadAvgParser : public Parser
{
public:
    virtual ParsedFileMap::const_iterator parse(const string&, ParsedFileMap&);
};

class MemInfoParser : public Parser
{
public:
    virtual ParsedFileMap::const_iterator parse(const string&, ParsedFileMap&);
};

class NetDevParser : public Parser
{
public:
    virtual ParsedFileMap::const_iterator parse(const string&, ParsedFileMap&);
};

class StatParser : public Parser
{
public:
    virtual ParsedFileMap::const_iterator parse(const string&, ParsedFileMap&);
};

}

//
// Implementation
//

Init::Init()
{
    //
    // Let's populate the global cache
    //
    SimpleParser simpleParser;
    CpuInfoParser().parse("/proc/cpuinfo", globalCache);
    simpleParser.parse("/proc/sys/kernel/hostname", globalCache);
    simpleParser.parse("/proc/sys/kernel/osrelease", globalCache);
    simpleParser.parse("/proc/sys/kernel/ostype", globalCache);
    simpleParser.parse("/proc/sys/kernel/version", globalCache);

    //
    // Now fill the parserMap
    //
    parserMap.insert(ParserMap::value_type("/proc/$pid/cmdline", new SimpleParser));
    parserMap.insert(ParserMap::value_type("/proc/$pid/stat", new ProcessStatParser));
    parserMap.insert(ParserMap::value_type("/proc/cpuinfo", new CpuInfoParser));
    parserMap.insert(ParserMap::value_type("/proc/loadavg", new LoadAvgParser));
    parserMap.insert(ParserMap::value_type("/proc/meminfo", new MemInfoParser));
    parserMap.insert(ParserMap::value_type("/proc/net/dev", new NetDevParser));
    parserMap.insert(ParserMap::value_type("/proc/stat", new StatParser));
}

Init::~Init()
{
    for(ParserMap::iterator p = parserMap.begin(); p != parserMap.end(); ++p)
    {
	delete p->second;
    }
}


inline PerfDataPtr
get(const ParsedFile& parsedFile, const string& entry)
{
    ParsedFile::const_iterator q = parsedFile.find(entry);
    if(q != parsedFile.end())
    {
	return q->second;
    }
    return 0;
}

inline void 
skipInt(istream& is)
{
    Ice::Int ignored;
    is >> ignored;
}

inline void 
skipLong(istream& is)
{
    Ice::Long ignored;
    is >> ignored;
}

inline void 
skipString(istream& is)
{
    string ignored;
    is >> ignored;
}

inline void
skipLine(istream& is)
{
    is.ignore(numeric_limits<int>::max(), '\n');
}

template<class T>
inline void
put(const string& entry, istream& is, ParsedFile& parsedFile)
{
    if(is.good())
    {
	::IceInternal::Handle<T> val = new T;
	is >> val->value;
	if(!is.bad())
	{
	    bool inserted = parsedFile.insert(ParsedFile::value_type(entry, val)).second;
	    assert(inserted);
	}
    }    
}

PerfDataSeq 
IcePerf::getPerfData(const StringSeq& keys)
{
    //
    // Per-call cache
    //
    ParsedFileMap cache;
    
    PerfDataSeq result;
    result.reserve(keys.size());

    for(size_t i = 0; i < keys.size(); ++i)
    {
	const string& key = keys[i];

	PerfDataPtr data;

	string filename;
	string entry;

	string::size_type index = key.find(".");
	if(index == string::npos)
	{
	    filename = key;
	}
	else
	{
	    filename = key.substr(0, index);

	    if(index + 1 <= key.size())
	    {
		entry = key.substr(index + 1);
	    }
	}
       
	//
	// In global cache?
	//
	ParsedFileMap::const_iterator p = globalCache.find(filename);
	if(p != globalCache.end())
	{
	    data = get(p->second, entry);
	}
	else
	{
	    //
	    // In local cache?
	    //
	    p = cache.find(filename);
	    if(p != cache.end())
	    {
		data = get(p->second, entry);
	    }
 	    else
	    {
		//
		// No, we need to parse the file then
		//
		
		string parserMapKey = filename;

		//
		// First, if filename = "/proc/<pid>/...", let's fixup parserMapKey:
		//
		if(parserMapKey.find("/proc/") == 0)
		{
		    string str = parserMapKey.substr(strlen("/proc/"));
		    index = str.find("/");
		    
		    if(index != string::npos)
		    {
			string pid  = str.substr(0, index);
			str = str.substr(index);
			
			if(pid.find_first_not_of("0123456789") == string::npos)
			{
			    //
			    // pid found
			    //
			    parserMapKey = string("/proc/$pid") + str; 
			}
		    }
		}
		
		ParserMap::const_iterator r = parserMap.find(parserMapKey);
		if(r != parserMap.end())
		{
		    p = r->second->parse(filename, cache);

		    //
		    // If the parsing failed, p == cache.end()
		    //
		    if(p != cache.end())
		    {
			data = get(p->second, entry);
		    }
		}
	    }
	}

	if(data == 0)
	{
	    data = new Unknown;
	}
	result.push_back(data);
    }
    return result;
} 

//
// Parser implementation
//

inline void
trim(string& s)
{
    string::size_type b = 0;
    string::size_type e = s.size();

    while(b < e && isspace(s[b]))
    {
	b++;
    }
   
    while(e > b && isspace(s[e - 1]))
    {
	e--;
    }
    
    if(b < e)
    {
	s = s.substr(b, e - b);
    }
    else
    {
	s = "";
    }
}
  
ParsedFileMap::const_iterator 
SimpleParser::parse(const string& filename, ParsedFileMap& cache)
{
    ifstream is(filename.c_str());
    if(!is.is_open() || is.eof())
    {
	return cache.end();
    }
    BoxedStringPtr str = new BoxedString;
    getline(is, str->value);

    ParsedFileMap::iterator result = cache.insert(ParsedFileMap::value_type(filename, ParsedFile())).first;
    result->second[""] = str;
    return result;
}


ParsedFileMap::const_iterator 
ProcessStatParser::parse(const string& filename, ParsedFileMap& cache)
{
    ifstream is(filename.c_str());
    if(!is.is_open() || is.eof())
    {
	return cache.end();
    }
    
    ParsedFileMap::iterator result = cache.insert(ParsedFileMap::value_type(filename, ParsedFile())).first;
    ParsedFile& parsedFile = result->second;

    skipString(is);
    put<BoxedString>("comm", is, parsedFile);
    put<BoxedString>("state", is, parsedFile);
    skipInt(is);
    skipInt(is);
    skipInt(is);
    skipInt(is);
    skipInt(is);
    put<BoxedLong>("flags", is, parsedFile);
    put<BoxedLong>("minflt", is, parsedFile);
    skipLong(is);
    put<BoxedLong>("majflt", is, parsedFile);
    skipLong(is);
    put<BoxedLong>("utime", is, parsedFile);
    put<BoxedLong>("stime", is, parsedFile);
    skipLong(is);
    skipLong(is);
    skipLong(is);
    skipLong(is);
    skipLong(is);
    skipLong(is);
    skipLong(is);
    put<BoxedLong>("vsize", is, parsedFile);
    put<BoxedLong>("rss", is, parsedFile);
    put<BoxedLong>("rlim", is, parsedFile);

    return result;

}

ParsedFileMap::const_iterator 
CpuInfoParser::parse(const string& filename, ParsedFileMap& cache)
{
    ifstream is(filename.c_str());
    if(!is.is_open() || is.eof())
    {
	return cache.end();
    }

    ParsedFileMap::iterator result = cache.insert(ParsedFileMap::value_type(filename, ParsedFile())).first;
    ParsedFile& parsedFile = result->second;

    string lhs, rhs;

    int processorCount = 0;
    string processorId; // processor not found yet
    int maxPhysicalId = -1; // not found

    while(!is.eof())
    {
	getline(is, lhs, ':');	
	trim(lhs);
	getline(is, rhs);
	trim(rhs);

	if(lhs == "processor")
	{
	    processorCount++;
	    processorId = rhs; 
	}
	else if(!processorId.empty())
	{
	    if(lhs == "physical id")
	    {
		int id = atoi(rhs.c_str());
		
		if(id > maxPhysicalId)
		{
		    maxPhysicalId = id;
		}
		BoxedIntPtr data = new BoxedInt;
		data->value = id;
		parsedFile[processorId + "." + lhs] = data;
	    }
	    else if(lhs == "cpu MHz" || lhs == "bogomips")
	    {
		BoxedFloatPtr data = new BoxedFloat;
		data->value = static_cast<float>(atof(rhs.c_str()));
		parsedFile[processorId + "." + lhs] = data;
	    }
	    else
	    {
		BoxedStringPtr data = new BoxedString;
		data->value = rhs;
		parsedFile[processorId + "." + lhs] = data;
	    }
	}
    }


    //
    // "processor count" and "physical processor count" are 
    // inferred from the data in cpuinfo
    //

    BoxedIntPtr data = new BoxedInt;
    data->value = processorCount;
    parsedFile["processor count"] = data;
    
    int physicalProcessorCount = maxPhysicalId + 1;
    if(physicalProcessorCount == 0)
    {
	physicalProcessorCount = processorCount;
    }

    data = new BoxedInt;
    data->value = physicalProcessorCount;
    parsedFile["physical processor count"] = data;

    return result;
}

ParsedFileMap::const_iterator 
LoadAvgParser::parse(const string& filename, ParsedFileMap& cache)
{
    ifstream is(filename.c_str());
    if(!is.is_open() || is.eof())
    {
	return cache.end();
    }

    LoadAvgPtr data = new LoadAvg;
    for(int i = 0; i < 3; i++)
    {
	float val = 0.0;
	if(!is.eof())
	{
	    is >> val;
	    data->value.push_back(val);
	}
    }
    
    ParsedFileMap::iterator result = cache.insert(ParsedFileMap::value_type(filename, ParsedFile())).first;
    result->second[""] = data;
    return result;
}

ParsedFileMap::const_iterator 
MemInfoParser::parse(const string& filename, ParsedFileMap& cache)
{
    ifstream is(filename.c_str());
    if(!is.is_open() || is.eof())
    {
	return cache.end();
    }

    ParsedFileMap::iterator result = cache.insert(ParsedFileMap::value_type(filename, ParsedFile())).first;
    ParsedFile& parsedFile = result->second;

    string lhs, rhs;
    while(!is.eof())
    {
	getline(is, lhs, ':');
	trim(lhs);
	getline(is, rhs);
	trim(rhs);
	
	BoxedIntPtr data = new BoxedInt;
	data->value = atoi(rhs.c_str());
	parsedFile[lhs] = data;
    }
    return result;
}

ParsedFileMap::const_iterator 
NetDevParser::parse(const string& filename, ParsedFileMap& cache)
{
    ifstream is(filename.c_str());
    if(!is.is_open() || is.eof())
    {
	return cache.end();
    }

    ParsedFileMap::iterator result = cache.insert(ParsedFileMap::value_type(filename, ParsedFile())).first;
    ParsedFile& parsedFile = result->second;

    skipLine(is);
    skipLine(is);

    while(!is.eof())
    {
	string interface;
	getline(is, interface, ':');
	trim(interface);
	cerr << interface << endl;
	put<BoxedLong>(interface + ".receive.bytes", is, parsedFile);
	put<BoxedLong>(interface + ".receive.packets", is, parsedFile);
	put<BoxedLong>(interface + ".receive.errs", is, parsedFile);
	put<BoxedLong>(interface + ".receive.drop", is, parsedFile);
	put<BoxedLong>(interface + ".receive.fifo", is, parsedFile);
	put<BoxedLong>(interface + ".receive.frame", is, parsedFile);
	put<BoxedLong>(interface + ".receive.compressed", is, parsedFile);
	put<BoxedLong>(interface + ".receive.multicast", is, parsedFile);
	
	put<BoxedLong>(interface + ".transmit.bytes", is, parsedFile);
	put<BoxedLong>(interface + ".transmit.packets", is, parsedFile);
	put<BoxedLong>(interface + ".transmit.errs", is, parsedFile);
	put<BoxedLong>(interface + ".transmit.drop", is, parsedFile);
	put<BoxedLong>(interface + ".transmit.fifo", is, parsedFile);
	put<BoxedLong>(interface + ".transmit.colls", is, parsedFile);
	put<BoxedLong>(interface + ".transmit.carrier", is, parsedFile);
	put<BoxedLong>(interface + ".transmit.compressed", is, parsedFile);
    }

    return result;

}

ParsedFileMap::const_iterator 
StatParser::parse(const string& filename, ParsedFileMap& cache)
{
    ifstream is(filename.c_str());
    if(!is.is_open() || is.eof())
    {
	return cache.end();
    }

    ParsedFileMap::iterator result = cache.insert(ParsedFileMap::value_type(filename, ParsedFile())).first;
    ParsedFile& parsedFile = result->second;

    string lhs, rhs;

    while(!is.eof())
    {
	is >> lhs;
	if(lhs == "cpu")
	{
	    put<BoxedLong>("user", is, parsedFile);
	    put<BoxedLong>("nice", is, parsedFile);
	    put<BoxedLong>("system", is, parsedFile);
	    put<BoxedLong>("idle", is, parsedFile);
	    break; // we're done
	}
	else
	{
	    //
	    // Not interesting
	    //
	    getline(is, rhs);
	}
    }
    return result;

}

