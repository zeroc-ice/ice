// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <QueueI.h>

using namespace std;

void
QueueI::get_async(const Demo::AMD_Queue_getPtr& cb, const string& id, const Ice::Current&)
{
    IceUtil::Mutex::Lock lock(*this);

    //
    // If there is already a message in the message queue, send the 
    // response  immediately. Otherwise add the callback to the 
    // request queue.
    //
    if(_messageQueue.size() != 0)
    {
        try
	{
            cb->ice_response(_messageQueue.front());
	    _messageQueue.pop_front();
	}
	catch(const Ice::Exception& ex)
	{
	    cout << ex << endl;
	}
    }
    else
    {
    	Request request;
	request.id = id;
	request.cb = cb;
        _requestQueue.push_back(request);
    }
}

void
QueueI::add(const string& message, const Ice::Current&)
{
    IceUtil::Mutex::Lock lock(*this);

    //
    // If there is an outstanding request in the request queue,
    // send a response. Otherwise add the message to the message
    // queue.
    //
    if(_requestQueue.size() != 0)
    {
        try
	{
            _requestQueue.front().cb->ice_response(message);
	}
	catch(const Ice::Exception& ex)
	{
	    _messageQueue.push_back(message);
	    cout << ex << endl;
	}
	_requestQueue.pop_front();
    }
    else
    {
        _messageQueue.push_back(message);
    }
}

void
QueueI::cancel_async(const Demo::AMD_Queue_cancelPtr& cb, const vector<string>& ids, const Ice::Current&)
{
    //
    // We send immediate response so that later call to ice_exception
    // on queued requests will not cause deadlocks.
    //
    cb->ice_response();

    IceUtil::Mutex::Lock lock(*this);

    for(vector<string>::const_iterator p = ids.begin(); p != ids.end(); ++p)
    {
        for(list<Request>::iterator q = _requestQueue.begin(); q != _requestQueue.end(); )
	{
	    if((*q).id == *p)
	    {
	        try
		{
		    (*q).cb->ice_exception(Demo::RequestCanceledException());
		}
		catch(const Ice::Exception&)
		{
		    // Ignore
		}
		list<Request>::iterator erase = q++;
	        _requestQueue.erase(erase);
		break;
	    }
	    else
	    {
	        ++q;
	    }

	}
    }
}
