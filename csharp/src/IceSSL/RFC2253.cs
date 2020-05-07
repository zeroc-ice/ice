//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

//
// See RFC 2253 and RFC 1779.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace IceSSL
{
    internal class RFC2253
    {
        internal struct RDNPair
        {
            internal string Key;
            internal string Value;
        }

        internal class RDNEntry
        {
            internal List<RDNPair> Rdn = new List<RDNPair>();
            internal bool Negate = false;
        }

        internal static List<RDNEntry> Parse(string data)
        {
            var results = new List<RDNEntry>();
            var current = new RDNEntry();
            int pos = 0;
            while (pos < data.Length)
            {
                EatWhite(data, ref pos);
                if (pos < data.Length && data[pos] == '!')
                {
                    if (current.Rdn.Count > 0)
                    {
                        throw new FormatException("negation symbol '!' must appear at start of list");
                    }
                    ++pos;
                    current.Negate = true;
                }
                current.Rdn.Add(ParseNameComponent(data, ref pos));
                EatWhite(data, ref pos);
                if (pos < data.Length && data[pos] == ',')
                {
                    ++pos;
                }
                else if (pos < data.Length && data[pos] == ';')
                {
                    ++pos;
                    results.Add(current);
                    current = new RDNEntry();
                }
                else if (pos < data.Length)
                {
                    throw new FormatException("expected ',' or ';' at `" + data.Substring(pos) + "'");
                }
            }
            if (current.Rdn.Count > 0)
            {
                results.Add(current);
            }

            return results;
        }

        internal static List<RDNPair> ParseStrict(string data)
        {
            var results = new List<RDNPair>();
            int pos = 0;
            while (pos < data.Length)
            {
                results.Add(ParseNameComponent(data, ref pos));
                EatWhite(data, ref pos);
                if (pos < data.Length && (data[pos] == ',' || data[pos] == ';'))
                {
                    ++pos;
                }
                else if (pos < data.Length)
                {
                    throw new FormatException("expected ',' or ';' at `" + data.Substring(pos) + "'");
                }
            }
            return results;
        }

        public static string Unescape(string data)
        {
            if (data.Length == 0)
            {
                return data;
            }

            if (data[0] == '"')
            {
                if (data[^1] != '"')
                {
                    throw new FormatException("unescape: missing \"");
                }
                //
                // Return the string without quotes.
                //
                return data[1..^1];
            }

            //
            // Unescape the entire string.
            //
            var result = new StringBuilder();
            if (data[0] == '#')
            {
                int pos = 1;
                while (pos < data.Length)
                {
                    result.Append(UnescapeHex(data, pos));
                    pos += 2;
                }
            }
            else
            {
                int pos = 0;
                while (pos < data.Length)
                {
                    if (data[pos] != '\\')
                    {
                        result.Append(data[pos]);
                        ++pos;
                    }
                    else
                    {
                        ++pos;
                        if (pos >= data.Length)
                        {
                            throw new FormatException("unescape: invalid escape sequence");
                        }
                        if (Special.IndexOf(data[pos]) != -1 || data[pos] != '\\' || data[pos] != '"')
                        {
                            result.Append(data[pos]);
                            ++pos;
                        }
                        else
                        {
                            result.Append(UnescapeHex(data, pos));
                            pos += 2;
                        }
                    }
                }
            }
            return result.ToString();
        }

        private static int HexToInt(char v)
        {
            if (v >= '0' && v <= '9')
            {
                return v - '0';
            }
            if (v >= 'a' && v <= 'f')
            {
                return 10 + (v - 'a');
            }
            if (v >= 'A' && v <= 'F')
            {
                return 10 + (v - 'A');
            }
            throw new FormatException("unescape: invalid hex pair");
        }

        private static char UnescapeHex(string data, int pos)
        {
            Debug.Assert(pos < data.Length);
            if (pos + 2 >= data.Length)
            {
                throw new FormatException("unescape: invalid hex pair");
            }
            return (char)((HexToInt(data[pos]) * 16) + HexToInt(data[pos + 1]));
        }

        private static RDNPair ParseNameComponent(string data, ref int pos)
        {
            RDNPair result = ParseAttributeTypeAndValue(data, ref pos);
            while (pos < data.Length)
            {
                EatWhite(data, ref pos);
                if (pos < data.Length && data[pos] == '+')
                {
                    ++pos;
                }
                else
                {
                    break;
                }
                RDNPair p = ParseAttributeTypeAndValue(data, ref pos);
                result.Value += "+";
                result.Value += p.Key;
                result.Value += '=';
                result.Value += p.Value;
            }
            return result;
        }

        private static RDNPair ParseAttributeTypeAndValue(string data, ref int pos)
        {
            var p = new RDNPair();
            p.Key = ParseAttributeType(data, ref pos);
            EatWhite(data, ref pos);
            if (pos >= data.Length)
            {
                throw new FormatException("invalid attribute type/value pair (unexpected end of data)");
            }
            if (data[pos] != '=')
            {
                throw new FormatException("invalid attribute type/value pair (missing =). remainder: " +
                                         data.Substring(pos));
            }
            ++pos;
            p.Value = ParseAttributeValue(data, ref pos);
            return p;
        }

        private static string ParseAttributeType(string data, ref int pos)
        {
            EatWhite(data, ref pos);
            if (pos >= data.Length)
            {
                throw new FormatException("invalid attribute type (expected end of data)");
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
            if (char.IsDigit(data[pos]) ||
               (data.Length - pos >= 4 && (data.Substring(pos, 4).Equals("oid.") ||
                                                       data.Substring(pos, 4).Equals("OID."))))
            {
                if (!char.IsDigit(data[pos]))
                {
                    result += data.Substring(pos, 4);
                    pos += 4;
                }

                while (true)
                {
                    // 1*DIGIT
                    while (pos < data.Length && char.IsDigit(data[pos]))
                    {
                        result += data[pos];
                        ++pos;
                    }
                    // "." 1*DIGIT
                    if (pos < data.Length && data[pos] == '.')
                    {
                        result += data[pos];
                        ++pos;
                        // 1*DIGIT must follow "."
                        if (pos < data.Length && !char.IsDigit(data[pos]))
                        {
                            throw new FormatException("invalid attribute type (expected end of data)");
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else if (char.IsUpper(data[pos]) || char.IsLower(data[pos]))
            {
                //
                // The grammar is wrong in this case. It should be ALPHA
                // KEYCHAR* otherwise it will not accept "O" as a valid
                // attribute type.
                //
                result += data[pos];
                ++pos;
                // 1* KEYCHAR
                while (pos < data.Length &&
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
                throw new FormatException("invalid attribute type");
            }
            return result;
        }

        private static string ParseAttributeValue(string data, ref int pos)
        {
            EatWhite(data, ref pos);
            if (pos >= data.Length)
            {
                return "";
            }

            //
            // RFC 2253
            // # hexstring
            //
            var result = new StringBuilder();
            if (data[pos] == '#')
            {
                result.Append(data[pos]);
                ++pos;
                while (true)
                {
                    string h = ParseHexPair(data, ref pos, true);
                    if (h.Length == 0)
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
            else if (data[pos] == '"')
            {
                result.Append(data[pos]);
                ++pos;
                while (true)
                {
                    if (pos >= data.Length)
                    {
                        throw new FormatException("invalid attribute value (unexpected end of data)");
                    }
                    // final terminating "
                    if (data[pos] == '"')
                    {
                        result.Append(data[pos]);
                        ++pos;
                        break;
                    }
                    // any character except '\'
                    else if (data[pos] != '\\')
                    {
                        result.Append(data[pos]);
                        ++pos;
                    }
                    // pair '\'
                    else
                    {
                        result.Append(ParsePair(data, ref pos));
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
                while (pos < data.Length)
                {
                    if (data[pos] == '\\')
                    {
                        result.Append(ParsePair(data, ref pos));
                    }
                    else if (Special.IndexOf(data[pos]) == -1 && data[pos] != '"')
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
        private static string ParsePair(string data, ref int pos)
        {
            string result = "";

            Debug.Assert(data[pos] == '\\');
            result += data[pos];
            ++pos;

            if (pos >= data.Length)
            {
                throw new FormatException("invalid escape format (unexpected end of data)");
            }

            if (Special.IndexOf(data[pos]) != -1 || data[pos] != '\\' ||
               data[pos] != '"')
            {
                result += data[pos];
                ++pos;
                return result;
            }
            return ParseHexPair(data, ref pos, false);
        }

        //
        // RFC 2253
        // hexpair    = hexchar hexchar
        //
        private static string ParseHexPair(string data, ref int pos, bool allowEmpty)
        {
            string result = "";
            if (pos < data.Length && Hexvalid.IndexOf(data[pos]) != -1)
            {
                result += data[pos];
                ++pos;
            }
            if (pos < data.Length && Hexvalid.IndexOf(data[pos]) != -1)
            {
                result += data[pos];
                ++pos;
            }
            if (result.Length != 2)
            {
                if (allowEmpty && result.Length == 0)
                {
                    return result;
                }
                throw new FormatException("invalid hex format");
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
        private static void EatWhite(string data, ref int pos)
        {
            while (pos < data.Length && data[pos] == ' ')
            {
                ++pos;
            }
        }

        private const string Special = ",=+<>#;";
        private const string Hexvalid = "0123456789abcdefABCDEF";
    }
}
