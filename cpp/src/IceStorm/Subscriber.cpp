// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>

#include <IceStorm/Subscriber.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Flusher.h>

using namespace IceStorm;
using namespace std;

void IceStorm::incRef(Subscriber* p) { p->__incRef(); }
void IceStorm::decRef(Subscriber* p) { p->__decRef(); }

Subscriber::Subscriber(const Ice::LoggerPtr& logger, const TraceLevelsPtr& traceLevels, const FlusherPtr& flusher,
		       const QoS& qos, const Ice::ObjectPrx& obj)
    : _logger(logger),
      _traceLevels(traceLevels),
      _invalid(false)
{

    //
    // Determine the requested reliability characteristics
    //
    QoS::const_iterator i = qos.find("reliability");
    string reliability;
    if (i == qos.end())
    {
	reliability = "oneway";
    }
    else
    {
	reliability = i->second;
    }

    if (reliability == "batch")
    {
	_obj = obj->ice_batchOneway();
	_flusher = flusher;
	_flusher->add(this);
    }
    else // reliability == "oneway"
    {
	if (reliability != "oneway")
	{
	    if (_traceLevels->subscriber > 0)
	    {
		ostringstream s;
		s << reliability <<" mode not understood.";
		_logger->trace(_traceLevels->subscriberCat, s.str());
	    }
	}
	_obj = obj->ice_oneway();
    }
}

Subscriber::~Subscriber()
{
}

bool
Subscriber::invalid() const
{
    JTCSyncT<JTCMutex> sync(_invalidMutex);
    return _invalid;
}

void
Subscriber::unsubscribe()
{
    JTCSyncT<JTCMutex> sync(_invalidMutex);
    _invalid = true;

    if (_traceLevels->subscriber > 0)
    {
	ostringstream s;
	s << "Unsubscribe " << _obj->ice_getIdentity();
	_logger->trace(_traceLevels->subscriberCat, s.str());
    }

    //
    // If this subscriber has been registered with the flusher then
    // remove it.
    //
    if (_flusher)
    {
	_flusher->remove(this);
    }
}

void
Subscriber::flush()
{
    try
    {
	_obj->ice_flush();
    }
    catch(const Ice::LocalException& e)
    {
	JTCSyncT<JTCMutex> sync(_invalidMutex);
	//
	// It's possible that the subscriber was unsubscribed, or
	// marked invalid by another thread. Don't display a
	// diagnostic in this case.
	//
	if (!_invalid)
	{
	    if (_traceLevels->subscriber > 0)
	    {
		ostringstream s;
		s << _obj->ice_getIdentity() << ": flush failed: " << e;
		_logger->trace(_traceLevels->subscriberCat, s.str());
	    }
	    _invalid = true;
	}
    }
}

void
Subscriber::publish(const string& op, const std::vector< ::Ice::Byte>& blob)
{
    try
    {
	bool nonmutating = true;
	_obj->ice_invokeIn(op, nonmutating, blob);
    }
    catch(const Ice::LocalException& e)
    {
	JTCSyncT<JTCMutex> sync(_invalidMutex);
	//
	// It's possible that the subscriber was unsubscribed, or
	// marked invalid by another thread. Don't display a
	// diagnostic in this case.
	//
	if (!_invalid)
	{
	    if (_traceLevels->subscriber > 0)
	    {
		ostringstream s;
		s << _obj->ice_getIdentity() << ": publish failed: " << e;
		_logger->trace(_traceLevels->subscriberCat, s.str());
	    }
	    _invalid = true;
	}
    }

}

std::string
Subscriber::id() const
{
    return _obj->ice_getIdentity();
}

bool
Subscriber::operator==(const Subscriber& rhs) const
{
    return _obj->ice_getIdentity() == rhs._obj->ice_getIdentity();
}
