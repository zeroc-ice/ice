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

using namespace std;

PingI::PingI(int nExpectedTicks, int nPublishers) : 
    _nPublishers(nPublishers),
    _nStartedPublishers(0),
    _nStoppedPublishers(0)
{
    _results.reserve(nExpectedTicks * _nPublishers);
}

void
PingI::tick(long long time, Perf::AEnum, int, const Perf::AStruct&, const Perf::IntfPrx&, const Ice::Current& current)
{
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
PingI::tickVoid(long long time, const Ice::Current& current)
{
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
	stopped();
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
	if(_nStartedPublishers < _nPublishers)
	{
	    // TODO: ERROR
	}
	_stopTime = IceUtil::Time::now();
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
PingI::add(long long time)
{
    _results.push_back(static_cast<int>(IceUtil::Time::now().toMicroSeconds() - time));
}

void
PingI::calc()
{
    double total = 0.0;
    for(vector<int>::const_iterator p = _results.begin(); p != _results.end(); ++p)
    {
	total += *p;
    }
    double mean = total / _results.size();
    
    double deviation;
    total = 0.0;
    for(vector<int>::const_iterator p = _results.begin(); p != _results.end(); ++p)
    {
	total = (*p - mean) * (*p - mean);
    }
    deviation = sqrt(total / (_results.size() - 1));
    
    cout << mean << " " << deviation << " " 
	 << static_cast<double>(_results.size()) / (_stopTime - _startTime).toMicroSeconds() * 1000000.0 
	 << " " << flush;

    _results.clear();    
}
