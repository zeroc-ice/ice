// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_DISTINGUISHED_NAME_H
#define ICE_SSL_DISTINGUISHED_NAME_H

#include "Ice/Config.h"

#include <list>
#include <string>

namespace Ice::SSL
{
    /// This class represents a DistinguishedName, similar to the Java
    /// type X500Principal and the .NET type X500DistinguishedName.
    ///
    /// For comparison purposes, the value of a relative distinguished
    /// name (RDN) component is always unescaped before matching,
    /// therefore "ZeroC, Inc." will match ZeroC\, Inc.
    ///
    /// toString() always returns exactly the same information as was
    /// provided in the constructor (i.e., "ZeroC, Inc." will not turn
    /// into ZeroC\, Inc.).
    class DistinguishedName
    {
    public:
        /// Creates a DistinguishedName from a string encoded using the rules in RFC2253.
        /// @param name The encoded distinguished name.
        /// @throws ParseException if parsing fails.
        explicit DistinguishedName(const std::string& name);

        /// Creates a DistinguishedName from a list of RDN pairs,
        /// where each pair consists of the RDN's type and value.
        /// For example, the RDN "O=ZeroC" is represented by the
        /// pair ("O", "ZeroC").
        /// @throws ParseException if parsing fails.
        explicit DistinguishedName(const std::list<std::pair<std::string, std::string>>&);

        /// Performs an exact match. The order of the RDN components is important.
        friend bool operator==(const DistinguishedName&, const DistinguishedName&);

        /// Performs an exact match. The order of the RDN components is important.
        friend bool operator<(const DistinguishedName&, const DistinguishedName&);

        /// Performs a partial match with another DistinguishedName.
        /// @param dn The name to be matched.
        /// @return `true` if all of the RDNs in the argument are present in this
        /// DistinguishedName and they have the same values.
        [[nodiscard]] bool match(const DistinguishedName& dn) const;

        /// Performs a partial match with another DistinguishedName.
        /// @param dn The name to be matched.
        /// @return `true` if all of the RDNs in the argument are present in this
        /// DistinguishedName and they have the same values.
        [[nodiscard]] bool match(const std::string& dn) const;

        /// Encodes the DN in RFC2253 format.
        /// @return An encoded string.
        [[nodiscard]] std::string toString() const;

        /// Encodes the DN in RFC2253 format.
        /// @return An encoded string.
        operator std::string() const { return toString(); }

    protected:
        /// @cond INTERNAL
        void unescape();
        /// @endcond

    private:
        std::list<std::pair<std::string, std::string>> _rdns;
        std::list<std::pair<std::string, std::string>> _unescaped;
    };

    /// Performs an exact match. The order of the RDN components is important.
    inline bool operator>(const DistinguishedName& lhs, const DistinguishedName& rhs) { return rhs < lhs; }

    /// Performs an exact match. The order of the RDN components is important.
    inline bool operator<=(const DistinguishedName& lhs, const DistinguishedName& rhs) { return !(lhs > rhs); }

    /// Performs an exact match. The order of the RDN components is important.
    inline bool operator>=(const DistinguishedName& lhs, const DistinguishedName& rhs) { return !(lhs < rhs); }

    /// Performs an exact match. The order of the RDN components is important.
    inline bool operator!=(const DistinguishedName& lhs, const DistinguishedName& rhs) { return !(lhs == rhs); }
}

#endif
