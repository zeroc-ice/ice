// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// See RFC 2253 and RFC 1779.
//
namespace IceSSL
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Text;

    class RFC2253
    {
        internal class ParseException : Exception
        {
            internal ParseException()
            {
            }

            internal ParseException(string reason)
            {
                this.reason = reason;
            }

            internal string
            ice_id()
            {
                return "::RFC2253::ParseException";
            }

            internal string reason;
        }

        internal struct RDNPair
        {
            internal string key;
            internal string value;
        }

        internal class RDNEntry
        {
            internal List<RDNPair> rdn = new List<RDNPair>();
            internal bool negate = false;
        }

        internal static List<RDNEntry> parse(string data)
        {
            List<RDNEntry> results = new List<RDNEntry>();
            RDNEntry current = new RDNEntry();
            int pos = 0;
            while(pos < data.Length)
            {
                eatWhite(data, ref pos);
                if(pos < data.Length && data[pos] == '!')
                {
                    if(current.rdn.Count > 0)
                    {
                        throw new ParseException("negation symbol '!' must appear at start of list");
                    }
                    ++pos;
                    current.negate = true;
                }
                current.rdn.Add(parseNameComponent(data, ref pos));
                eatWhite(data, ref pos);
                if(pos < data.Length && data[pos] == ',')
                {
                    ++pos;
                }
                else if(pos < data.Length && data[pos] == ';')
                {
                    ++pos;
                    results.Add(current);
                    current = new RDNEntry();
                }
                else if(pos < data.Length)
                {
                    throw new ParseException("expected ',' or ';' at `" + data.Substring(pos) + "'");
                }
            }
            if(current.rdn.Count > 0)
            {
                results.Add(current);
            }

            return results;
        }

        internal static List<RDNPair> parseStrict(string data)
        {
            List<RDNPair> results = new List<RDNPair>();
            int pos = 0;
            while(pos < data.Length)
            {
                results.Add(parseNameComponent(data, ref pos));
                eatWhite(data, ref pos);
                if(pos < data.Length && (data[pos] == ',' || data[pos] == ';'))
                {
                    ++pos;
                }
                else if(pos < data.Length)
                {
                    throw new ParseException("expected ',' or ';' at `" + data.Substring(pos) + "'");
                }
            }
            return results;
        }

        public static string unescape(string data)
        {
            if(data.Length == 0)
            {
                return data;
            }

            if(data[0] == '"')
            {
                if(data[data.Length - 1] != '"')
                {
                    throw new ParseException("unescape: missing \"");
                }
                //
                // Return the string without quotes.
                //
                return data.Substring(1, data.Length - 2);
            }

            //
            // Unescape the entire string.
            //
            StringBuilder result = new StringBuilder();
            if(data[0] == '#')
            {
                int pos = 1;
                while(pos < data.Length)
                {
                    result.Append(unescapeHex(data, pos));
                    pos += 2;
                }
            }
            else
            {
                int pos = 0;
                while (pos < data.Length)
                {
                    if(data[pos] != '\\')
                    {
                        result.Append(data[pos]);
                        ++pos;
                    }
                    else
                    {
                        ++pos;
                        if(pos >= data.Length)
                        {
                            throw new ParseException("unescape: invalid escape sequence");
                        }
                        if(special.IndexOf(data[pos]) != -1 || data[pos] != '\\' || data[pos] != '"')
                        {
                            result.Append(data[pos]);
                            ++pos;
                            }
                        else
                        {
                            result.Append(unescapeHex(data, pos));
                            pos += 2;
                        }
                    }
                }
            }
            return result.ToString();
        }

        private static int hexToInt(char v)
        {
            if(v >= '0' && v <= '9')
            {
                return v - '0';
            }
            if(v >= 'a' && v <= 'f')
            {
                return 10 + (v - 'a');
            }
            if(v >= 'A' && v <= 'F')
            {
                return 10 + (v - 'A');
            }
            throw new ParseException("unescape: invalid hex pair");
        }

        private static char unescapeHex(string data, int pos)
        {
            Debug.Assert(pos < data.Length);
            if(pos + 2 >= data.Length)
            {
                throw new ParseException("unescape: invalid hex pair");
            }
            return (char)(hexToInt(data[pos]) * 16 + hexToInt(data[pos + 1]));
        }

        private static RDNPair parseNameComponent(string data, ref int pos)
        {
            RDNPair result = parseAttributeTypeAndValue(data, ref pos);
            while(pos < data.Length)
            {
                eatWhite(data, ref pos);
                if(pos < data.Length && data[pos] == '+')
                {
                    ++pos;
                }
                else
                {
                    break;
                }
                RDNPair p = parseAttributeTypeAndValue(data, ref pos);
                result.value += "+";
                result.value += p.key;
                result.value += '=';
                result.value += p.value;
            }
            return result;
        }

        private static RDNPair parseAttributeTypeAndValue(string data, ref int pos)
        {
            RDNPair p = new RDNPair();
            p.key = parseAttributeType(data, ref pos);
            eatWhite(data, ref pos);
            if(pos >= data.Length)
            {
                throw new ParseException("invalid attribute type/value pair (unexpected end of data)");
            }
            if(data[pos] != '=')
            {
                throw new ParseException("invalid attribute type/value pair (missing =). remainder: " +
                                         data.Substring(pos));
            }
            ++pos;
            p.value = parseAttributeValue(data, ref pos);
            return p;
        }

        private static string parseAttributeType(string data, ref int pos)
        {
            eatWhite(data, ref pos);
            if(pos >= data.Length)
            {
                throw new ParseException("invalid attribute type (expected end of data)");
            }

            string result = "";

            //
            // RFC 1779.
            // <key> ::= 1*( <keychar> ) | "OID." <oid> | "oid." <oid>
            // <oid> ::= <digitstring> | <digitstring> "." <oid>
            // RFC 2253:
            // attributeType = (ALPHA 1*keychar) | oid
            // keychar    = ALPHA | DIGIT | "-"
            // oid        = 1*DIGIT *("." 1*DIGIT)
            //
            // In section 4 of RFC 2253 the document says:
            // Implementations MUST allow an oid in the attribute type to be
            // prefixed by one of the character strings "oid." or "OID.".
            //
            // Here we must also check for "oid." and "OID." before parsing
            // according to the ALPHA KEYCHAR* rule.
            //
            // First the OID case.
            //
            if(char.IsDigit(data[pos]) ||
               (data.Length - pos >= 4 && (data.Substring(pos, 4).Equals("oid.") ||
                                                       data.Substring(pos, 4).Equals("OID."))))
            {
                if(!char.IsDigit(data[pos]))
                {
                    result += data.Substring(pos, 4);
                    pos += 4;
                }

                while(true)
                {
                    // 1*DIGIT
                    while(pos < data.Length && char.IsDigit(data[pos]))
                    {
                        result += data[pos];
                        ++pos;
                    }
                    // "." 1*DIGIT
                    if(pos < data.Length && data[pos] == '.')
                    {
                        result += data[pos];
                        ++pos;
                        // 1*DIGIT must follow "."
                        if(pos < data.Length && !char.IsDigit(data[pos]))
                        {
                            throw new ParseException("invalid attribute type (expected end of data)");
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else if(char.IsUpper(data[pos]) || char.IsLower(data[pos]))
            {
                //
                // The grammar is wrong in this case. It should be ALPHA
                // KEYCHAR* otherwise it will not accept "O" as a valid
                // attribute type.
                //
                result += data[pos];
                ++pos;
                // 1* KEYCHAR
                while(pos < data.Length &&
                      (char.IsDigit(data[pos]) ||
                       char.IsUpper(data[pos]) ||
                       char.IsLower(data[pos]) ||
                       data[pos] == '-'))
                {
                    result += data[pos];
                    ++pos;
                }
            }
            else
            {
                throw new ParseException("invalid attribute type");
            }
            return result;
        }

        private static string parseAttributeValue(string data, ref int pos)
        {
            eatWhite(data, ref pos);
            if(pos >= data.Length)
            {
                return "";
            }

            //
            // RFC 2253
            // # hexstring
            //
            StringBuilder result = new StringBuilder();
            if(data[pos] == '#')
            {
                result.Append(data[pos]);
                ++pos;
                while(true)
                {
                    string h = parseHexPair(data, ref pos, true);
                    if(h.Length == 0)
                    {
                        break;
                    }
                    result.Append(h);
                }
            }
            //
            // RFC 2253
            // QUOTATION *( quotechar | pair ) QUOTATION ; only from v2
            // quotechar     = <any character except "\" or QUOTATION >
            //
            else if(data[pos] == '"')
            {
                result.Append(data[pos]);
                ++pos;
                while(true)
                {
                    if(pos >= data.Length)
                    {
                        throw new ParseException("invalid attribute value (unexpected end of data)");
                    }
                    // final terminating "
                    if(data[pos] == '"')
                    {
                        result.Append(data[pos]);
                        ++pos;
                        break;
                    }
                    // any character except '\'
                    else if(data[pos] != '\\')
                    {
                        result.Append(data[pos]);
                        ++pos;
                    }
                    // pair '\'
                    else
                    {
                        result.Append(parsePair(data, ref pos));
                    }
                }
            }
            //
            // RFC 2253
            // * (stringchar | pair)
            // stringchar = <any character except one of special, "\" or QUOTATION >
            //
            else
            {
                while(pos < data.Length)
                {
                    if(data[pos] == '\\')
                    {
                        result.Append(parsePair(data, ref pos));
                    }
                    else if(special.IndexOf(data[pos]) == -1 && data[pos] != '"')
                    {
                        result.Append(data[pos]);
                        ++pos;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            return result.ToString();
        }

        //
        // RFC2253:
        // pair       = "\" ( special | "\" | QUOTATION | hexpair )
        //
        private static string parsePair(string data, ref int pos)
        {
            string result = "";

            Debug.Assert(data[pos] == '\\');
            result += data[pos];
            ++pos;

            if(pos >= data.Length)
            {
                throw new ParseException("invalid escape format (unexpected end of data)");
            }

            if(special.IndexOf(data[pos]) != -1 || data[pos] != '\\' ||
               data[pos] != '"')
            {
                result += data[pos];
                ++pos;
                return result;
            }
            return parseHexPair(data, ref pos, false);
        }

        //
        // RFC 2253
        // hexpair    = hexchar hexchar
        //
        private static string parseHexPair(string data, ref int pos, bool allowEmpty)
        {
            string result = "";
            if(pos < data.Length && hexvalid.IndexOf(data[pos]) != -1)
            {
                result += data[pos];
                ++pos;
            }
            if(pos < data.Length && hexvalid.IndexOf(data[pos]) != -1)
            {
                result += data[pos];
                ++pos;
            }
            if(result.Length != 2)
            {
                if(allowEmpty && result.Length == 0)
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
        private static void eatWhite(string data, ref int pos)
        {
            while(pos < data.Length && data[pos] == ' ')
            {
                ++pos;
            }
        }

        private static string special = ",=+<>#;";
        private static string hexvalid = "0123456789abcdefABCDEF";
    }
}
