// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <QueueI.h>

using namespace std;

void
QueueI::get_async(const Demo::AMD_Queue_getPtr& getCB, const Ice::Current&)
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
            getCB->ice_response(_messageQueue.front());
	    _messageQueue.pop_front();
	}
	catch(const Ice::Exception& ex)
	{
	    cout << ex << endl;
	}
    }
    else
    {
        _requestQueue.push_back(getCB);
    }
}

void
QueueI::add(const ::std::string& message, const Ice::Current&)
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
            _requestQueue.front()->ice_response(message);
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
