// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CONTACT_I_H
#define CONTACT_I_H

#include <Contact.h>

class ContactDBI : public Demo::ContactDB
{
    std::map<std::string, Demo::ContactPtr> _contacts;

public:

    virtual void addContact(const std::string&, const IceUtil::Optional< Demo::NumberType>&,
    	const IceUtil::Optional< std::string>&, const IceUtil::Optional< Ice::Int>&, const Ice::Current&);

    virtual void updateContact(const std::string&, const IceUtil::Optional< Demo::NumberType>&,
    	const IceUtil::Optional< std::string>&, const IceUtil::Optional< Ice::Int>&, const Ice::Current&);

    virtual Demo::ContactPtr query(const std::string&, const Ice::Current&);

    virtual IceUtil::Optional<std::string> queryNumber(const std::string&, const Ice::Current&);

    virtual void queryDialgroup(const std::string&, IceUtil::Optional< Ice::Int>&, const Ice::Current&);

    virtual void shutdown(const Ice::Current&);
};

#endif
