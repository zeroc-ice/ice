// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/SubscriberFactory.h>
#include <IceStorm/LinkSubscriber.h>
#include <IceStorm/OnewaySubscriber.h>
#include <IceStorm/OnewayBatchSubscriber.h>
#include <IceStorm/Flusher.h>
#include <IceStorm/TraceLevels.h>

using namespace std;
using namespace IceStorm;

SubscriberFactory::SubscriberFactory(const TraceLevelsPtr& traceLevels, const FlusherPtr& flusher) :
    _traceLevels(traceLevels),
    _flusher(flusher)
{
}

SubscriberPtr
SubscriberFactory::createSubscriber(const QoS& qos, const Ice::ObjectPrx& obj)
{
    //
    // Determine the requested reliability characteristics
    //
    Ice::ObjectPrx newObj;

    QoS::const_iterator i = qos.find("reliability");
    string reliability;
    if(i == qos.end())
    {
	reliability = "oneway";
    }
    else
    {
	reliability = i->second;
    }

    if(reliability == "batch")
    {
	return new OnewayBatchSubscriber(_traceLevels, _flusher, obj->ice_batchOneway());
    }
    else // reliability == "oneway"
    {
	if(reliability != "oneway")
	{
	    if(_traceLevels->subscriber > 0)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
		out << reliability <<" mode not understood.";
	    }
	}
	return new OnewaySubscriber(_traceLevels, obj->ice_oneway());
    }

    assert(false);
}

SubscriberPtr
SubscriberFactory::createLinkSubscriber(const TopicLinkPrx& obj, Ice::Int cost)
{
    return new LinkSubscriber(_traceLevels, obj, cost);
}
