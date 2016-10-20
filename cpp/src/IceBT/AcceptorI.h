// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BT_ACCEPTOR_I_H
#define ICE_BT_ACCEPTOR_I_H

#include <Ice/TransceiverF.h>
#include <Ice/Acceptor.h>
#include <IceBT/Config.h>
#include <IceBT/EngineF.h>
#include <IceBT/InstanceF.h>

#include <stack>

namespace IceBT
{

class AcceptorI : public IceInternal::Acceptor, public IceInternal::NativeInfo
{
public:

    virtual IceInternal::NativeInfoPtr getNativeInfo();

    virtual void close();
    virtual IceInternal::EndpointIPtr listen();
    virtual IceInternal::TransceiverPtr accept();
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;

    int effectiveChannel() const;

    void newConnection(int);

private:

    AcceptorI(const EndpointIPtr&, const InstancePtr&, const std::string&, const std::string&, const std::string&,
              const std::string&, int);
    virtual ~AcceptorI();
    friend class EndpointI;

    EndpointIPtr _endpoint;
    const InstancePtr _instance;
    const std::string _adapterName;
    const std::string _addr;
    const std::string _uuid;
    const std::string _name;
    const int _channel;
    std::string _path;

    IceUtil::Monitor<IceUtil::Mutex> _lock;
    std::stack<IceInternal::TransceiverPtr> _transceivers;
};

}

#endif
