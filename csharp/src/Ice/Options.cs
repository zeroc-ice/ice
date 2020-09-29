// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;

namespace ZeroC.Ice
{
    /// <summary>Provides a helper method to parse command line arguments.</summary>
    public static class Options
    {
        private enum State
        {
            Normal,
            DoubleQuote,
            SingleQuote,
            ANSIQuote
        }

        /// <summary>Splits a string into a list of command line arguments, the string is parsed using semantics similar
        /// to that of command-line arguments.</summary>
        /// <param name="line">The string to split.</param>
        /// <returns>An array of strings containing the parsed command line arguments.</returns>
        public static string[] Split(string line)
        {
            string inputFieldSeparator = " \t\n";

            string l = line.Trim();
            if (l.Length == 0)
            {
                return Array.Empty<string>();
            }

            State state = State.Normal;

            string arg = "";
            var vec = new List<string>();

            for (int i = 0; i < l.Length; ++i)
            {
                char c = l[i];
                switch (state)
                {
                    case State.Normal:
                    {
                        switch (c)
                        {
                            case '\\':
                            {
                                // Ignore a backslash at the end of the string, and strip backslash-newline pairs. If a
                                // backslash is followed by a space, single quote, double quote, or dollar sign, we
                                // drop the backslash and write the space, single quote, double quote, or dollar sign.
                                // This is necessary to allow quotes to be escaped. Dropping the backslash preceding a
                                // space deviates from bash quoting rules, but is necessary so we don't drop backslashes
                                // from Windows path names.)
                                if (i < l.Length - 1 && l[++i] != '\n')
                                {
                                    switch (l[i])
                                    {
                                        case ' ':
                                        case '$':
                                        case '\'':
                                        case '"':
                                        {
                                            arg += l[i];
                                            break;
                                        }
                                        default:
                                        {
                                            arg += '\\';
                                            arg += l[i];
                                            break;
                                        }
                                    }
                                }
                                break;
                            }
                            case '\'':
                            {
                                state = State.SingleQuote;
                                break;
                            }
                            case '"':
                            {
                                state = State.DoubleQuote;
                                break;
                            }
                            case '$':
                            {
                                if (i < l.Length - 1 && l[i + 1] == '\'')
                                {
                                    // Bash uses $'<text>' to allow ANSI escape sequences within <text>
                                    state = State.ANSIQuote;
                                    ++i;
                                }
                                else
                                {
                                    arg += '$';
                                }
                                break;
                            }
                            default:
                            {
                                if (inputFieldSeparator.IndexOf(l[i]) != -1)
                                {
                                    vec.Add(arg);
                                    arg = "";

                                    // Move to start of next argument.
                                    while (++i < l.Length && inputFieldSeparator.IndexOf(l[i]) != -1)
                                    {
                                    }
                                    --i;
                                }
                                else
                                {
                                    arg += l[i];
                                }
                                break;
                            }
                        }
                        break;
                    }
                    case State.DoubleQuote:
                    {
                        // Within double quotes, only backslash retains its special meaning, and only if followed by
                        // double quote, backslash, or newline. If not followed by one of these characters, both the
                        // backslash and the character are preserved.
                        if (c == '\\' && i < l.Length - 1)
                        {
                            switch (c = l[++i])
                            {
                                case '"':
                                case '\\':
                                case '\n':
                                {
                                    arg += c;
                                    break;
                                }
                                default:
                                {
                                    arg += '\\';
                                    arg += c;
                                    break;
                                }
                            }
                        }
                        else if (c == '"') // End of double-quote mode.
                        {
                            state = State.Normal;
                        }
                        else
                        {
                            arg += c; // Everything else is taken literally.
                        }
                        break;
                    }
                    case State.SingleQuote:
                    {
                        if (c == '\'') // End of single-quote mode.
                        {
                            state = State.Normal;
                        }
                        else
                        {
                            arg += c; // Everything else is taken literally.
                        }
                        break;
                    }
                    case State.ANSIQuote:
                    {
                        switch (c)
                        {
                            case '\\':
                            {
                                if (i == l.Length - 1)
                                {
                                    break;
                                }
                                switch (c = l[++i])
                                {
                                    // Single-letter escape sequences.
                                    case 'a':
                                    {
                                        arg += '\a';
                                        break;
                                    }
                                    case 'b':
                                    {
                                        arg += '\b';
                                        break;
                                    }
                                    case 'f':
                                    {
                                        arg += '\f';
                                        break;
                                    }
                                    case 'n':
                                    {
                                        arg += '\n';
                                        break;
                                    }
                                    case 'r':
                                    {
                                        arg += '\r';
                                        break;
                                    }
                                    case 't':
                                    {
                                        arg += '\t';
                                        break;
                                    }
                                    case 'v':
                                    {
                                        arg += '\v';
                                        break;
                                    }
                                    case '\\':
                                    {
                                        arg += '\\';
                                        break;
                                    }
                                    case '\'':
                                    {
                                        arg += '\'';
                                        break;
                                    }
                                    case 'e': // Not ANSI-C, but used by bash.
                                    {
                                        arg += '\u001B';
                                        break;
                                    }

                                    // Process up to three octal digits.
                                    case '0':
                                    case '1':
                                    case '2':
                                    case '3':
                                    case '4':
                                    case '5':
                                    case '6':
                                    case '7':
                                    {
                                        const string octalDigits = "01234567";
                                        short s = 0;
                                        int j;
                                        for (j = i; j < i + 3 && j < l.Length && octalDigits.IndexOf(c = l[j]) != -1; ++j)
                                        {
                                            s = (short)((s * 8) + c - '0');
                                        }
                                        i = j - 1;
                                        arg += (char)s;
                                        break;
                                    }

                                    // Process up to two hex digits.
                                    case 'x':
                                    {
                                        const string hexDigits = "0123456789abcdefABCDEF";
                                        if (i < l.Length - 1 && !hexDigits.Contains(l[i + 1]))
                                        {
                                            arg += '\\';
                                            arg += 'x';
                                            break;
                                        }

                                        short s = 0;
                                        int j;
                                        for (j = i + 1;
                                            j < i + 3 && j < l.Length && hexDigits.IndexOf(c = l[j]) != -1;
                                            ++j)
                                        {
                                            s *= 16;
                                            if (char.IsDigit(c))
                                            {
                                                s += (short)(c - '0');
                                            }
                                            else if (char.IsLower(c))
                                            {
                                                s += (short)(c - 'a' + 10);
                                            }
                                            else
                                            {
                                                s += (short)(c - 'A' + 10);
                                            }
                                        }
                                        i = j - 1;
                                        arg += (char)s;
                                        break;
                                    }

                                    // Process control-chars.
                                    case 'c':
                                    {
                                        c = l[++i];
                                        if ((char.ToUpper(c, CultureInfo.InvariantCulture) >= 'A' &&
                                             char.ToUpper(c, CultureInfo.InvariantCulture) <= 'Z') ||
                                            c == '@' || (c >= '[' && c <= '_'))
                                        {
                                            arg += (char)(char.ToUpper(c, CultureInfo.InvariantCulture) - '@');
                                        }
                                        else
                                        {
                                            // Bash does not define what should happen if a \c is not followed by a
                                            // recognized control character. We simply treat this case like other
                                            // unrecognized escape sequences, that is, we preserve the escape sequence
                                            // unchanged.
                                            arg += '\\';
                                            arg += 'c';
                                            arg += c;
                                        }
                                        break;
                                    }

                                    // If inside an ANSI-quoted string, a backslash isn't followed by one of the
                                    // recognized characters, both the backslash and the character are preserved.
                                    default:
                                    {
                                        arg += '\\';
                                        arg += c;
                                        break;
                                    }
                                }
                                break;
                            }
                            case '\'': // End of ANSI-quote mode.
                            {
                                state = State.Normal;
                                break;
                            }
                            default:
                            {
                                arg += c; // Everything else is taken literally.
                                break;
                            }
                        }
                        break;
                    }
                    default:
                    {
                        Debug.Assert(false);
                        break;
                    }
                }
            }

            switch (state)
            {
                case State.Normal:
                {
                    vec.Add(arg);
                    break;
                }
                case State.SingleQuote:
                {
                    throw new FormatException("missing closing single quote");
                }
                case State.DoubleQuote:
                {
                    throw new FormatException("missing closing double quote");
                }
                case State.ANSIQuote:
                {
                    throw new FormatException("unterminated $' quote");
                }
                default:
                {
                    Debug.Assert(false);
                    break;
                }
            }

            return vec.ToArray();
        }
    }
}
