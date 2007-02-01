// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

//
// AMI callback class for twoway requests
//
// NOTE: the received response isn't sent back directly with the AMD
// callback. Instead it's queued and the request queue thread is
// responsible for sending back the response. It's necessary because
// sending back the response might block.
//
class AMI_Array_Object_ice_invokeI : public AMI_Array_Object_ice_invoke
{
public:

    AMI_Array_Object_ice_invokeI(const RequestQueuePtr& requestQueue, const AMD_Array_Object_ice_invokePtr& amdCB) :
        _requestQueue(requestQueue),
        _amdCB(amdCB)
    {
        assert(_amdCB);
    }

    virtual void
    ice_response(bool ok, const pair<const Byte*, const Byte*>& outParams)
    {
        _requestQueue->addResponse(new Response(_amdCB, ok, outParams));
    }

    virtual void
    ice_exception(const Exception& ex)
    {
        _requestQueue->addResponse(new Response(_amdCB, ex));
    }

private:

    const RequestQueuePtr _requestQueue;
    const AMD_Array_Object_ice_invokePtr _amdCB;
};

}

Glacier2::Request::Request(const ObjectPrx& proxy, const std::pair<const Byte*, const Byte*>& inParams,
                           const Current& current, bool forwardContext, const Ice::Context& sslContext,
                           const AMD_Array_Object_ice_invokePtr& amdCB) :
    _proxy(proxy),
    _inParams(inParams.first, inParams.second),
    _current(current),
    _forwardContext(forwardContext),
    _sslContext(sslContext),
    _amdCB(amdCB)
{
    //
    // If this is not a twoway call, we can finish the AMD call right
    // away.
    //
    if(!_proxy->ice_isTwoway())
    {
        bool ok = true;
        pair<const Byte*, const Byte*> outParams(0, 0);
        _amdCB->ice_response(ok, outParams);
    }

    Context::const_iterator p = current.ctx.find("_ovrd");
    if(p != current.ctx.end())
    {
        const_cast<string&>(_override) = p->second;
    }
}


bool
Glacier2::Request::invoke(const RequestQueuePtr& requestQueue)
{
    pair<const Byte*, const Byte*> inPair;
    if(_inParams.size() == 0)
    {
        inPair.first = inPair.second = 0;
    }
    else
    {
        inPair.first = &_inParams[0];
        inPair.second = inPair.first + _inParams.size();
    }
    if(_proxy->ice_isTwoway())
    {
        AMI_Array_Object_ice_invokePtr cb = new AMI_Array_Object_ice_invokeI(requestQueue, _amdCB);
        if(_forwardContext)
        {
            if(_sslContext.size() > 0)
            {
                Ice::Context ctx = _current.ctx;
                ctx.insert(_sslContext.begin(), _sslContext.end());
                _proxy->ice_invoke_async(cb, _current.operation, _current.mode, inPair, ctx);
            }
            else
            {
                _proxy->ice_invoke_async(cb, _current.operation, _current.mode, inPair, _current.ctx);
            }
        }
        else
        {
            if(_sslContext.size() > 0)
            {
                _proxy->ice_invoke_async(cb, _current.operation, _current.mode, inPair, _sslContext);
            }
            else
            {
                _proxy->ice_invoke_async(cb, _current.operation, _current.mode, inPair);
            }
        }
        return true; // A twoway method is being dispatched.
    }
    else
    {
        try
        {
            ByteSeq outParams;
            if(_forwardContext)
            {
                if(_sslContext.size() > 0)
                {
                    Ice::Context ctx = _current.ctx;
                    ctx.insert(_sslContext.begin(), _sslContext.end());
                    _proxy->ice_invoke(_current.operation, _current.mode, inPair, outParams, ctx);
                }
                else
                {
                    _proxy->ice_invoke(_current.operation, _current.mode, inPair, outParams, _current.ctx);
                }
            }
            else
            {
                if(_sslContext.size() > 0)
                {
                    _proxy->ice_invoke(_current.operation, _current.mode, inPair, outParams, _sslContext);
                }
                else
                {
                    _proxy->ice_invoke(_current.operation, _current.mode, inPair, outParams);
                }
            }
        }
        catch(const LocalException&)
        {
        }
        return false;
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
    return _proxy->ice_getConnection();
}

Glacier2::Response::Response(const AMD_Array_Object_ice_invokePtr& amdCB, bool ok, 
                             const pair<const Byte*, const Byte*>& outParams) : 
    _amdCB(amdCB),
    _ok(ok),
    _outParams(outParams.first, outParams.second)
{
}

Glacier2::Response::Response(const AMD_Array_Object_ice_invokePtr& amdCB, const Exception& ex) : 
    _amdCB(amdCB),
    _ok(false),
    _exception(ex.ice_clone())
{
}

void
Glacier2::Response::invoke()
{
    if(_exception.get())
    {
        _amdCB->ice_exception(*_exception.get());
    }
    else
    {
        pair<const Byte*, const Byte*> outPair;
        if(_outParams.size() == 0)
        {
            outPair.first = outPair.second = 0;
        }
        else
        {
            outPair.first = &_outParams[0];
            outPair.second = outPair.first + _outParams.size();
        }
        _amdCB->ice_response(_ok, outPair);
    }
}

Glacier2::RequestQueue::RequestQueue(const IceUtil::Time& sleepTime) :
    _sleepTime(sleepTime),
    _destroy(false),
    _sleep(false)
{
}

Glacier2::RequestQueue::~RequestQueue()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    assert(_destroy);
    assert(_requests.empty());
    assert(_responses.empty());
}

void 
Glacier2::RequestQueue::destroy()
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
        
        assert(!_destroy);
        _destroy = true;
        _sleep = false;
        notify();
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
    
    if(_destroy)
    {
        throw ObjectNotExistException(__FILE__, __LINE__);
    }

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
    if(!_sleep)
    {
        //
        // No need to notify if the request queue thread is sleeping,
        // once it wakes up it will check if there's requests to send.
        //
        notify();
    }
    return false;
}

void
Glacier2::RequestQueue::addResponse(const ResponsePtr& response)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    _responses.push_back(response);
    notify();
}

void
Glacier2::RequestQueue::run()
{
    RequestQueuePtr self = this; // This is to avoid creating a temporary Ptr for each call to Request::invoke()
    ptrdiff_t dispatchCount = 0; // The dispatch count keeps track of the number of outstanding twoway requests.
    while(true)
    {
        vector<RequestPtr> requests;
        vector<ResponsePtr> responses;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

            //
            // Wait indefinitely if there's no requests/responses to
            // send. If the queue is being destroyed we still need to
            // wait until all the responses for twoway requests are
            // received.
            //
            while((!_destroy || dispatchCount != 0) && _responses.empty() && (_requests.empty() || _sleep))
            {
                if(_sleep)
                {
                    IceUtil::Time now = IceUtil::Time::now();
                    if(!timedWait(_sleepDuration))
                    {
                        _sleepDuration = IceUtil::Time();
                    }
                    else
                    {
                        _sleepDuration -= IceUtil::Time::now() - now;
                    }
                    if(_sleepDuration <= IceUtil::Time())
                    {
                        _sleep = false;
                    }
                }
                else
                {
                    wait();
                }
            }

            //
            // If the queue is being destroyed and there's no requests
            // or responses to send, we're done.
            //
            if(_destroy && _requests.empty() && _responses.empty())
            {
                assert(dispatchCount == 0); // We would have blocked in the wait() above otherwise.
                return;
            }

            //
            // If there's requests to sent and we're not sleeping,
            // send the requests. If a sleep time is configured, we
            // set the sleep duration and set the sleep flag to make
            // sure we'll sleep again once we're done sending requests
            // and responses.
            //
            if(!_requests.empty() && !_sleep)
            {
                requests.swap(_requests);
                if(_sleepTime > IceUtil::Time())
                {
                    _sleep = true;
                    _sleepDuration = _sleepTime;
                }
            }
            if(!_responses.empty())
            {
                responses.swap(_responses);
            }
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
            
            //
            // Invoke returns true if the request expects a response.
            // If that's the case we increment the dispatch count to
            // ensure that the thread won't be destroyed before the
            // response is received.
            //
            if((*p)->invoke(self)) // Exceptions are caught within invoke().
            {
                ++dispatchCount;
            }
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
        // Send the responses and decrement the dispatch count.
        //
        for(vector<ResponsePtr>::const_iterator r = responses.begin(); r != responses.end(); ++r)
        {
            (*r)->invoke();
        }
        dispatchCount -= responses.size();
    }
}
