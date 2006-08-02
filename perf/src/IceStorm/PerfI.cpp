// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/Time.h>
#include <IceUtil/Thread.h>

#include <PerfI.h>

#include <io.h>

#include <algorithm>
#include <math.h>

using namespace std;

PingI::PingI(int nExpectedTicks, int nPublishers) : 
    _nPublishers(nPublishers),
    _nStartedPublishers(0),
    _nStoppedPublishers(0),
    _nExpectedTicks(nExpectedTicks * _nPublishers),
    _nReceived(0)
{
    _results.reserve(nExpectedTicks * _nPublishers);
}

void
PingI::tick(Ice::Long time, Perf::AEnum, int, const Perf::AStruct&, const Ice::Current& current)
{
    Lock sync(*this);
    if(time > 0)
    {
	add(time);
    }
    else if(time == 0)
    {
	started();
    }
    else if(time < 0)
    {
	if(stopped())
	{
	    current.adapter->getCommunicator()->shutdown();
	}
    }
}

void
PingI::tickVoid(Ice::Long time, const Ice::Current& current)
{
    Lock sync(*this);
    if(time > 0)
    {
	add(time);
    }
    else if(time == 0)
    {
	started();
    }
    else if(time == -1)
    {
	if(stopped())
	{
	    current.adapter->getCommunicator()->shutdown();
	}
    }
    else if(time < 0)
    {
	cerr << "time < 0: " << time << endl;
    }
}

void
PingI::started()
{
    if(++_nStartedPublishers == _nPublishers)
    {
	_startTime = IceUtil::Time::now();
    }
}

bool
PingI::stopped()
{
    if(_nStoppedPublishers == 0)
    {
	_stopTime = IceUtil::Time::now();
    }
    if(_nStartedPublishers < _nPublishers)
    {
	cerr << "Some publishers are already finished while others aren't even started" << endl;
	cerr << _nPublishers << " " << _nStartedPublishers << " " << _nStoppedPublishers << endl;
	cerr << _startTime - _stopTime << " " << _results.size() << " " << _nReceived << endl;
    }
    if(++_nStoppedPublishers == _nPublishers)
    {
	calc();
	return true;
    }
    else
    {
	return false;
    }
}

void
PingI::add(Ice::Long time)
{
    ++_nReceived;
    if(_nStartedPublishers == _nPublishers && _nStoppedPublishers == 0)
    {
#ifdef WIN32
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	Ice::Long interval = t.QuadPart - time;
	QueryPerformanceFrequency(&t);
	interval /= (t.QuadPart / 100000);
	_results.push_back(interval);
#else
	IceUtil::Time t = IceUtil::Time::microSeconds(time);
	IceUtil::Time interval = IceUtil::Time::now() - t;
	_results.push_back(static_cast<Ice::Long>(interval.toMicroSeconds()));
#endif
    }
}

void
PingI::calc()
{
    double originalSize = _results.size();
    //
    // Only keep the N/2 best results
    //
    sort(_results.begin(), _results.end());
    _results.resize(_results.size() / 2);

    double total = 0.0;
    {
	for(vector<Ice::Long>::const_iterator p = _results.begin(); p != _results.end(); ++p)
	{
	    total += (*p) * 1.0;
	}
    }
    double newSize = _results.size();
    double mean = total / newSize; 
    
    double deviation;
    double x = 0.0;
    {
	for(vector<Ice::Long>::const_iterator p = _results.begin(); p != _results.end(); ++p)
	{
	    x += (*p - mean) * (*p - mean);
	}
    }
    deviation = sqrt(x / (_results.size() - 1));

    if(originalSize < (_nExpectedTicks * 0.90))
    {
	cerr << "less than 90% of the expected ticks were used for the test " << originalSize << endl;
    }

    cout << mean << " " << deviation << " " << 
	originalSize / (_stopTime - _startTime).toMicroSeconds() * 1000000.0 << " ";
    cout << flush;

    _results.clear();
    _nStartedPublishers = 0;
    _nStoppedPublishers = 0;
}
