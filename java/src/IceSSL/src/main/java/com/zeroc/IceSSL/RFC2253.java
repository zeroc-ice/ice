// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceSSL;

//
// See RFC 2253 and RFC 1779.
//
class RFC2253
{
    static class ParseException extends com.zeroc.Ice.LocalException
    {
        public ParseException()
        {
        }

        public ParseException(String reason)
        {
            this.reason = reason;
        }

        @Override
        public String
        ice_id()
        {
            return "::RFC2253::ParseException";
        }

        public String reason;

        public static final long serialVersionUID = 0L;
    }

    static class RDNPair
    {
        String key;
        String value;
    }

    static class RDNEntry
    {
        java.util.List<RDNPair> rdn = new java.util.LinkedList<>();
        boolean negate = false;
    }

    static private class ParseState
    {
        String data;
        int pos;
    }

    public static java.util.List<RDNEntry>
    parse(String data)
        throws ParseException
    {
        java.util.List<RDNEntry> results = new java.util.LinkedList<>();
        RDNEntry current = new RDNEntry();
        ParseState state = new ParseState();
        state.data = data;
        state.pos = 0;
        while(state.pos < state.data.length())
        {
            eatWhite(state);
            if(state.pos < state.data.length() && state.data.charAt(state.pos) == '!')
            {
                if(!current.rdn.isEmpty())
                {
                    throw new ParseException("negation symbol '!' must appear at start of list");
                }
                ++state.pos;
                current.negate = true;
            }
            current.rdn.add(parseNameComponent(state));
            eatWhite(state);
            if(state.pos < state.data.length() && state.data.charAt(state.pos) == ',')
            {
                ++state.pos;
            }
            else if(state.pos < state.data.length() && state.data.charAt(state.pos) == ';')
            {
                ++state.pos;
                results.add(current);
                current = new RDNEntry();
            }
            else if(state.pos < state.data.length())
            {
                throw new ParseException("expected ',' or ';' at `" + state.data.substring(state.pos) + "'");
            }
        }
        if(!current.rdn.isEmpty())
        {
            results.add(current);
        }

        return results;
    }

    public static java.util.List<RDNPair>
    parseStrict(String data)
        throws ParseException
    {
        java.util.List<RDNPair> results = new java.util.LinkedList<>();
        ParseState state = new ParseState();
        state.data = data;
        state.pos = 0;
        while(state.pos < state.data.length())
        {
            results.add(parseNameComponent(state));
            eatWhite(state);
            if(state.pos < state.data.length() &&
               (state.data.charAt(state.pos) == ',' || state.data.charAt(state.pos) == ';'))
            {
                ++state.pos;
            }
            else if(state.pos < state.data.length())
            {
                throw new ParseException("expected ',' or ';' at `" + state.data.substring(state.pos) + "'");
            }
        }
        return results;
    }

    private static RDNPair
    parseNameComponent(ParseState state)
        throws ParseException
    {
        RDNPair result = parseAttributeTypeAndValue(state);
        while(state.pos < state.data.length())
        {
            eatWhite(state);
            if(state.pos < state.data.length() && state.data.charAt(state.pos) == '+')
            {
                ++state.pos;
            }
            else
            {
                break;
            }
            RDNPair p = parseAttributeTypeAndValue(state);
            result.value += "+";
            result.value += p.key;
            result.value += '=';
            result.value += p.value;
        }
        return result;
    }

    private static RDNPair
    parseAttributeTypeAndValue(ParseState state)
        throws ParseException
    {
        RDNPair p = new RDNPair();
        p.key = parseAttributeType(state);
        eatWhite(state);
        if(state.pos >= state.data.length())
        {
            throw new ParseException("invalid attribute type/value pair (unexpected end of state.data)");
        }
        if(state.data.charAt(state.pos) != '=')
        {
            throw new ParseException("invalid attribute type/value pair (missing =)");
        }
        ++state.pos;
        p.value = parseAttributeValue(state);
        return p;
    }

    private static String
    parseAttributeType(ParseState state)
        throws ParseException
    {
        eatWhite(state);
        if(state.pos >= state.data.length())
        {
            throw new ParseException("invalid attribute type (expected end of state.data)");
        }

        StringBuffer result = new StringBuffer();

        //
        // RFC 1779.
        // <key> ::= 1*( <keychar> ) | "OID." <oid> | "oid." <oid>
        // <oid> ::= <digitString> | <digitstring> "." <oid>
        // RFC 2253:
        // attributeType = (ALPHA 1*keychar) | oid
        // keychar    = ALPHA | DIGIT | "-"
        // oid        = 1*DIGIT *("." 1*DIGIT)
        //
        // In section 4 of RFC 2253 the document says:
        // Implementations MUST allow an oid in the attribute type to be
        // prefixed by one of the character Strings "oid." or "OID.".
        //
        // Here we must also check for "oid." and "OID." before parsing
        // according to the ALPHA KEYCHAR* rule.
        //
        // First the OID case.
        //
        if(Character.isDigit(state.data.charAt(state.pos)) ||
           (state.data.length() - state.pos >= 4 && (state.data.substring(state.pos, state.pos + 4).equals("oid.") ||
                                                   state.data.substring(state.pos, state.pos + 4).equals("OID."))))
        {
            if(!Character.isDigit(state.data.charAt(state.pos)))
            {
                result.append(state.data.substring(state.pos, state.pos + 4));
                state.pos += 4;
            }

            while(true)
            {
                // 1*DIGIT
                while(state.pos < state.data.length() && Character.isDigit(state.data.charAt(state.pos)))
                {
                    result.append(state.data.charAt(state.pos));
                    ++state.pos;
                }
                // "." 1*DIGIT
                if(state.pos < state.data.length() && state.data.charAt(state.pos) == '.')
                {
                    result.append(state.data.charAt(state.pos));
                    ++state.pos;
                    // 1*DIGIT must follow "."
                    if(state.pos < state.data.length() && !Character.isDigit(state.data.charAt(state.pos)))
                    {
                        throw new ParseException("invalid attribute type (expected end of state.data)");
                    }
                }
                else
                {
                    break;
                }
            }
        }
        else if(Character.isUpperCase(state.data.charAt(state.pos)) ||
                Character.isLowerCase(state.data.charAt(state.pos)))
        {
            //
            // The grammar is wrong in this case. It should be ALPHA
            // KEYCHAR* otherwise it will not accept "O" as a valid
            // attribute type.
            //
            result.append(state.data.charAt(state.pos));
            ++state.pos;
            // 1* KEYCHAR
            while(state.pos < state.data.length() &&
                  (Character.isDigit(state.data.charAt(state.pos)) ||
                   Character.isUpperCase(state.data.charAt(state.pos)) ||
                   Character.isLowerCase(state.data.charAt(state.pos)) ||
                   state.data.charAt(state.pos) == '-'))
            {
                result.append(state.data.charAt(state.pos));
                ++state.pos;
            }
        }
        else
        {
            throw new ParseException("invalid attribute type");
        }
        return result.toString();
    }

    private static String
    parseAttributeValue(ParseState state)
        throws ParseException
    {
        eatWhite(state);
        if(state.pos >= state.data.length())
        {
            return "";
        }

        //
        // RFC 2253
        // # hexString
        //
        StringBuffer result = new StringBuffer();
        if(state.data.charAt(state.pos) == '#')
        {
            result.append(state.data.charAt(state.pos));
            ++state.pos;
            while(true)
            {
                String h = parseHexPair(state, true);
                if(h.length() == 0)
                {
                    break;
                }
                result.append(h);
            }
        }
        //
        // RFC 2253
        // QUOTATION *( quotechar | pair ) QUOTATION ; only from v2
        // quotechar     = <any character except "\" or QUOTATION >
        //
        else if(state.data.charAt(state.pos) == '"')
        {
            result.append(state.data.charAt(state.pos));
            ++state.pos;
            while(true)
            {
                if(state.pos >= state.data.length())
                {
                    throw new ParseException("invalid attribute value (unexpected end of state.data)");
                }
                // final terminating "
                if(state.data.charAt(state.pos) == '"')
                {
                    result.append(state.data.charAt(state.pos));
                    ++state.pos;
                    break;
                }
                // any character except '\'
                else if(state.data.charAt(state.pos) != '\\')
                {
                    result.append(state.data.charAt(state.pos));
                    ++state.pos;
                }
                // pair '\'
                else
                {
                    result.append(parsePair(state));
                }
            }
        }
        //
        // RFC 2253
        // * (Stringchar | pair)
        // Stringchar = <any character except one of special, "\" or QUOTATION >
        //
        else
        {
            while(state.pos < state.data.length())
            {
                if(state.data.charAt(state.pos) == '\\')
                {
                    result.append(parsePair(state));
                }
                else if(special.indexOf(state.data.charAt(state.pos)) == -1 && state.data.charAt(state.pos) != '"')
                {
                    result.append(state.data.charAt(state.pos));
                    ++state.pos;
                }
                else
                {
                    break;
                }
            }
        }
        return result.toString();
    }

    //
    // RFC2253:
    // pair       = "\" ( special | "\" | QUOTATION | hexpair )
    //
    private static String
    parsePair(ParseState state)
        throws ParseException
    {
        String result = "";

        assert(state.data.charAt(state.pos) == '\\');
        result += state.data.charAt(state.pos);
        ++state.pos;

        if(state.pos >= state.data.length())
        {
            throw new ParseException("invalid escape format (unexpected end of state.data)");
        }

        if(special.indexOf(state.data.charAt(state.pos)) != -1 || state.data.charAt(state.pos) != '\\' ||
           state.data.charAt(state.pos) != '"')
        {
            result += state.data.charAt(state.pos);
            ++state.pos;
            return result;
        }
        return parseHexPair(state, false);
    }

    //
    // RFC 2253
    // hexpair    = hexchar hexchar
    //
    private static String
    parseHexPair(ParseState state, boolean allowEmpty)
        throws ParseException
    {
        String result = "";
        if(state.pos < state.data.length() && hexvalid.indexOf(state.data.charAt(state.pos)) != -1)
        {
            result += state.data.charAt(state.pos);
            ++state.pos;
        }
        if(state.pos < state.data.length() && hexvalid.indexOf(state.data.charAt(state.pos)) != -1)
        {
            result += state.data.charAt(state.pos);
            ++state.pos;
        }
        if(result.length() != 2)
        {
            if(allowEmpty && result.length() == 0)
            {
                return result;
            }
            throw new ParseException("invalid hex format");
        }
        return result;
    }

    //
    // RFC 2253:
    //
    // Implementations MUST allow for space (' ' ASCII 32) characters to be
    // present between name-component and ',', between attributeTypeAndValue
    // and '+', between attributeType and '=', and between '=' and
    // attributeValue.  These space characters are ignored when parsing.
    //
    private static void
    eatWhite(ParseState state)
    {
        while(state.pos < state.data.length() && state.data.charAt(state.pos) == ' ')
        {
            ++state.pos;
        }
    }

    private final static String special = ",=+<>#;";
    private final static String hexvalid = "0123456789abcdefABCDEF";
}
