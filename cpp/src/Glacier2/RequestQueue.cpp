// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/RequestQueue.h>
#include <set>

using namespace std;
using namespace Ice;
using namespace Glacier2;

namespace Glacier2
{

class AMI_Object_ice_invokeI : public AMI_Object_ice_invoke
{
public:

    AMI_Object_ice_invokeI(const AMD_Object_ice_invokePtr& amdCB) :
	_amdCB(amdCB)
    {
	assert(_amdCB);
    }

    virtual void
    ice_response(bool ok, const std::vector<Byte>& outParams)
    {
	_amdCB->ice_response(ok, outParams);
    }

    virtual void
    ice_exception(const Exception& ex)
    {
	_amdCB->ice_exception(ex);
    }

private:

    const AMD_Object_ice_invokePtr _amdCB;
};

}

Glacier2::Request::Request(const ObjectPrx& proxy, const ByteSeq& inParams, const Current& current,
			   bool forwardContext, const AMD_Object_ice_invokePtr& amdCB) :
    _proxy(proxy),
    _inParams(inParams),
    _current(current),
    _forwardContext(forwardContext),
    _amdCB(amdCB)
{
    //
    // If this is not a twoway call, we can finish the AMD call right
    // away.
    //
    if(!_proxy->ice_isTwoway())
    {
	bool ok = true;
	ByteSeq outParams;
	_amdCB->ice_response(ok, outParams);
    }

    Context::const_iterator p = current.ctx.find("_ovrd");
    if(p != current.ctx.end())
    {
	const_cast<string&>(_override) = p->second;
    }
}


void
Glacier2::Request::invoke()
{
    try
    {
	if(_proxy->ice_isTwoway())
	{
	    AMI_Object_ice_invokePtr cb = new AMI_Object_ice_invokeI(_amdCB);
	    if(_forwardContext)
	    {
		_proxy->ice_invoke_async(cb, _current.operation, _current.mode, _inParams, _current.ctx);
	    }
	    else
	    {
		_proxy->ice_invoke_async(cb, _current.operation, _current.mode, _inParams);
	    }
	}
	else
	{
	    ByteSeq outParams;
	    if(_forwardContext)
	    {
		_proxy->ice_invoke(_current.operation, _current.mode, _inParams, outParams, _current.ctx);
	    }
	    else
	    {
		_proxy->ice_invoke(_current.operation, _current.mode, _inParams, outParams);
	    }
	}
    }
    catch(const LocalException& ex)
    {
	if(_proxy->ice_isTwoway())
	{
	    _amdCB->ice_exception(ex);
	}
    }
}

bool
Glacier2::Request::override(const RequestPtr& other) const
{
    //
    // Both override values have to be non-empty.
    //
    if(_override.empty() || other->_override.empty())
    {
	return false;
    }

    //
    // Override does not work for twoways, because a response is
    // expected for each request.
    //
    if(_proxy->ice_isTwoway() || other->_proxy->ice_isTwoway())
    {
	return false;
    }

    //
    // We cannot override if the proxies differ.
    //
    if(_proxy != other->_proxy)
    {
	return false;
    }

    return _override == other->_override;
}

bool
Glacier2::Request::isBatch() const
{
    return _proxy->ice_isBatchOneway() || _proxy->ice_isBatchDatagram();
}

ConnectionPtr
Glacier2::Request::getConnection() const
{
    return _proxy->ice_connection();
}

Glacier2::RequestQueue::RequestQueue(const IceUtil::Time& sleepTime) :
    _sleepTime(sleepTime),
    _destroy(false)
{
}

Glacier2::RequestQueue::~RequestQueue()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    assert(_destroy);
    assert(_requests.empty());
}

void 
Glacier2::RequestQueue::destroy()
{
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
	
	assert(!_destroy);
	_destroy = true;
	notify();
	
	_requests.clear();
    }

    //
    // We don't want to wait for the RequestQueue thread, because this
    // destroy() operation is called when sessions expire or are
    // destroyed, in which case we do not want the session handler
    // thread to block here. Therefore we don't call join(), but
    // instead detach the thread right after we start it.
    //
    //getThreadControl().join();
}

bool
Glacier2::RequestQueue::addRequest(const RequestPtr& request)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    
    assert(!_destroy);

    for(vector<RequestPtr>::iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
	//
        // If the new request overrides an old one, then abort the old
        // request and replace it with the new request.
	//
        if(request->override(*p))
        {
            *p = request;
	    return true;
        }
    }

    //
    // No override, we add the new request.
    //
    _requests.push_back(request);
    notify();
    return false;
}

void
Glacier2::RequestQueue::run()
{
    while(true)
    {
	vector<RequestPtr> requests;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

	    //
	    // Wait indefinitely if there's no requests to send.
	    //
            while(!_destroy && _requests.empty())
            {		
		wait();
            }

            if(_destroy)
            {
                return;
            }

	    requests.swap(_requests);
	}
        
        //
        // Send requests, flush batch requests, and sleep outside the
        // thread synchronization, so that new messages can be added
        // while this is being done.
        //

	set<ConnectionPtr> flushSet;
	
	for(vector<RequestPtr>::const_iterator p = requests.begin(); p != requests.end(); ++p)
	{
	    if((*p)->isBatch())
	    {
		try
		{
		    flushSet.insert((*p)->getConnection());
		}
		catch(const LocalException&)
		{
		    // Ignore.
		}
	    }
	    
	    (*p)->invoke(); // Exceptions are caught within invoke().
	}

	for(set<ConnectionPtr>::const_iterator q = flushSet.begin(); q != flushSet.end(); ++q)
	{
	    try
	    {
		(*q)->flushBatchRequests();
	    }
	    catch(const LocalException&)
	    {
		// Ignore.
	    }
	}
	
	//
	// In order to avoid flooding, we add a delay, if so
	// requested.
	//
	if(_sleepTime > IceUtil::Time())
	{
	    IceUtil::ThreadControl::sleep(_sleepTime);
	}
    }
}
