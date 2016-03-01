// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_RFC_2253_H
#define ICE_SSL_RFC_2253_H

#include <IceUtil/Config.h>
#include <list>

//
// The methods in the IceSSL::RFC2253 namespace implement a parser
// for relative distinguished name (RDN) pairs using the parsing
// rules outlined in sections 3 and 4 of RFC 2253.
//
// Note that this parser does not unescape the elements of the RDNs.
// For example, parsing the following RDN
//
// O=Sue\, Grabit and Runn
//
// results in the pair ("O","Sue\, Grabit and Runn") and not
// ("O","Sue, Grabit and Runn").
//
namespace IceSSL
{
namespace RFC2253
{

typedef std::list< std::pair<std::string, std::string> > RDNSeq;

struct RDNEntry
{
    RDNSeq rdn;
    bool negate;
};
typedef std::list<RDNEntry> RDNEntrySeq;

//
// This function separates DNs with the ';' character. A list of RDN
// pairs may optionally be prefixed with '!' to indicate a negation.
// The function returns a list of RDNEntry structures. Any failure in
// parsing results in a ParseException being thrown.
//
RDNEntrySeq parse(const std::string&);

//
// RDNs are separated with ',' and ';'.
//
// This function returns a list of RDN pairs. Any failure in parsing
// results in a ParseException being thrown.
//
RDNSeq parseStrict(const std::string&);

//
// Unescape the string.
//
std::string unescape(const std::string&);

}
}

#endif
