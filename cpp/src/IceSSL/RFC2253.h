// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_RFC_2253_H
#define ICE_SSL_RFC_2253_H

#include <IceUtil/Exception.h>
#include <list>

namespace RFC2253
{

class ParseException : public IceUtil::Exception
{
public:

    ParseException(const char*, int, const std::string&);
    virtual const std::string ice_name() const;
    virtual IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason;

private:

    static const char* _name;
};

//
// This returns a list of list of RDN pairs parsed according to the
// RFC2253 parsing rules as outlined in section 3 and section 4 of the
// RFC. The only difference is that the ';' character is treated
// as a seperator between different DNs.
//
// Note that this parser does not unescape the elements of the RDNs. So:
//
// O=Sue\, Grabit and Runn will result in ("O","Sue\, Grabit and
// Runn") not ("O","Sue, Grabit and Runn").
//
// Unique DNs can be separated by a ';' character.
//
// This method returns a list of list of RDNs. Any failure in parsing
// results in a ParseException being thrown.
//
std::list< std::list<std::pair<std::string, std::string> > > parse(const std::string&);

}

#endif
