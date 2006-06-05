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

//
// This returns a list of list of RDN pairs parsed according to the
// RFC2253 parsing rules as outlined in section 3 and section 4 of the
// RFC.
//
// Note that this parser does not unescape the elements of the RDNs. So:
//
// O=Sue\, Grabit and Runn will result in ("O","Sue\, Grabit and
// Runn") not ("O","Sue, Grabit and Runn").
//
typedef std::list< std::pair<std::string, std::string> > RDNSeq;
typedef std::list<RDNSeq> RDNSeqSeq;
//
// This method seperates DNs with the ';' character.  
//
// This method returns a list of list of DNs. Any failure in parsing
// results in a ParseException being thrown.
//
RDNSeqSeq parse(const std::string&);
//
// RDNs are seperated with ',' and ';'.
//
// This method returns a list of list of RDNs. Any failure in parsing
// results in a ParseException being thrown.
//
RDNSeq parseStrict(const std::string&);

}

#endif
