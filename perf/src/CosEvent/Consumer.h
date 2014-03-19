// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPERF_CONSUMER_H
#define ICEPERF_CONSUMER_H

#include <vector>
#include <orbsvcs/CosEventCommS.h>

class Consumer : public POA_CosEventComm::PushConsumer
{
public:
    Consumer();

    int run(int argc, char* argv[]);

    virtual void push(const CORBA::Any &event ACE_ENV_ARG_DECL_NOT_USED)
        ACE_THROW_SPEC ((CORBA::SystemException));
    virtual void disconnect_push_consumer(ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
        ACE_THROW_SPEC ((CORBA::SystemException));

private:
    
    void started();
    bool stopped();
    void add(CORBA::LongLong);
    void calc();

    CORBA::ULong _event_count;

    CORBA::ULong _nExpectedTicks;
    std::vector<long> _results;

    bool _payload;
    CORBA::LongLong _startTime;
    CORBA::LongLong _stopTime;
    int _nPublishers;
    int _nStartedPublishers;
    int _nStoppedPublishers;
    ACE_Thread_Mutex _lock;
    int _payloadSize;

    CORBA::ORB_ptr _orb;
};

#endif /* CONSUMER_H */
