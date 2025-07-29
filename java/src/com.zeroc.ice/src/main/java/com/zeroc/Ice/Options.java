// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.ArrayList;
import java.util.List;

/**
 * @hidden Public because it's used by IceBox.
 */
public final class Options {
    public static String[] split(String line) throws ParseException {
        final String IFS = " \t\n";
        final int NormalState = 1;
        final int DoubleQuoteState = 2;
        final int SingleQuoteState = 3;
        final int ANSIQuoteState = 4;

        line = line.trim();
        if (line.isEmpty()) {
            return new String[0];
        }

        int state = NormalState;

        StringBuilder arg = new StringBuilder(128);
        List<String> vec = new ArrayList<>();

        for (int i = 0; i < line.length(); i++) {
            char c = line.charAt(i);
            switch (state) {
                case NormalState:
                {
                    switch (c) {
                        case '\\':
                        {
                            // Ignore a backslash at the end of the string, and strip
                            // backslash-newline pairs.
                            //
                            // If a backslash comes before a space, single quote, double
                            // quote, or dollar sign we drop the backslash, but still write
                            // the the space, quote, or dollar sign. This is necessary to
                            // allow quotes to be escaped. Dropping the backslash preceding
                            // a space deviates from bash quoting rules, but is necessary so
                            // we don't drop backslashes from Windows path names.
                            if (i < line.length() - 1 && line.charAt(++i) != '\n') {
                                char nextChar = line.charAt(i);
                                // TODO: comment says we should be checking single quotes
                                // here, but we aren't?
                                if (nextChar != ' '
                                    && nextChar != '$'
                                    && nextChar != '\\'
                                    && nextChar != '"') {
                                    arg.append('\\');
                                }
                                arg.append(nextChar);
                            }
                            break;
                        }
                        case '\'':
                        {
                            state = SingleQuoteState;
                            break;
                        }
                        case '"':
                        {
                            state = DoubleQuoteState;
                            break;
                        }
                        case '$':
                        {
                            if (i < line.length() - 1 && line.charAt(i + 1) == '\'') {
                                // Bash uses $'<text>' to allow ANSI escape sequences within
                                // <text>.
                                state = ANSIQuoteState;
                                ++i;
                            } else {
                                arg.append('$');
                            }
                            break;
                        }
                        default:
                        {
                            if (IFS.indexOf(line.charAt(i)) != -1) {
                                vec.add(arg.toString());
                                arg = new StringBuilder(128);

                                // Move to start of next argument.
                                while (++i < line.length()
                                    && IFS.indexOf(line.charAt(i)) != -1) {
                                    continue;
                                }
                                --i;
                            } else {
                                arg.append(line.charAt(i));
                            }
                            break;
                        }
                    }
                    break;
                }
                case DoubleQuoteState:
                {
                    // Within double quotes, only backslash retains its special meaning,
                    // and only if followed by a double quote, backslash, or newline.
                    // Both the backslash and the character are preserved for any other
                    // character.
                    if (c == '\\' && i < line.length() - 1) {
                        c = line.charAt(++i);
                        if (c != '"' && c != '\\' && c != '\n') {
                            arg.append('\\');
                        }
                        arg.append(c);
                    } else if (c == '"') {
                        // End of double-quote mode.
                        state = NormalState;
                    } else {
                        // Everything else is taken literally.
                        arg.append(c);
                    }
                    break;
                }
                case SingleQuoteState:
                {
                    if (c == '\'') {
                        // End of single-quote mode.
                        state = NormalState;
                    } else {
                        // Everything else is taken literally.
                        arg.append(c);
                    }
                    break;
                }
                case ANSIQuoteState:
                {
                    switch (c) {
                        case '\\':
                        {
                            if (i == line.length() - 1) {
                                break;
                            }
                            switch (c = line.charAt(++i)) {
                                // Single-letter escape sequences.
                                case 'a':
                                {
                                    arg.append('\007');
                                    break;
                                }
                                case 'b':
                                {
                                    arg.append('\b');
                                    break;
                                }
                                case 'f':
                                {
                                    arg.append('\f');
                                    break;
                                }
                                case 'n':
                                {
                                    arg.append('\n');
                                    break;
                                }
                                case 'r':
                                {
                                    arg.append('\r');
                                    break;
                                }
                                case 't':
                                {
                                    arg.append('\t');
                                    break;
                                }
                                case 'v':
                                {
                                    arg.append('\013');
                                    break;
                                }
                                case '\\':
                                {
                                    arg.append('\\');
                                    break;
                                }
                                case '\'':
                                {
                                    arg.append('\'');
                                    break;
                                }
                                case 'e': // Not ANSI-C, but used by bash.
                                    {
                                        arg.append('\033');
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
                                    final String octalDigits = "01234567";
                                    short us = 0;
                                    int j;
                                    for (j = i;
                                                        j < i + 3
                                                                && j < line.length()
                                                                && octalDigits.indexOf(
                                                                                c = line.charAt(j))
                                                                        != -1;
                                                        j++) {
                                        us = (short) (us * 8 + c - '0');
                                    }
                                    i = j - 1;
                                    arg.append((char) us);
                                    break;
                                }

                                // Process up to two hex digits.
                                case 'x':
                                {
                                    final String hexDigits = "0123456789abcdefABCDEF";
                                    if (i < line.length() - 1
                                        && hexDigits.indexOf(line.charAt(i + 1))
                                        == -1) {
                                        arg.append('\\');
                                        arg.append('x');
                                        break;
                                    }

                                    short s = 0;
                                    int j;
                                    for (j = i + 1;
                                                        j < i + 3
                                                                && j < line.length()
                                                                && hexDigits.indexOf(
                                                                                c = line.charAt(j))
                                                                        != -1;
                                                        j++) {
                                        s *= (short) 16;
                                        if (Character.isDigit(c)) {
                                            s += (short) (c - '0');
                                        } else if (Character.isLowerCase(c)) {
                                            s += (short) (c - 'a' + 10);
                                        } else {
                                            s += (short) (c - 'A' + 10);
                                        }
                                    }
                                    i = j - 1;
                                    arg.append((char) s);
                                    break;
                                }

                                // Process control-chars.
                                case 'c':
                                {
                                    c = line.charAt(++i);
                                    if ((Character.toUpperCase(c) >= 'A'
                                        && Character.toUpperCase(c) <= 'Z')
                                        || c == '@'
                                        || (c >= '[' && c <= '_')) {
                                        arg.append(
                                            (char)
                                                (Character.toUpperCase(c)
                                                    - '@'));
                                    } else {
                                        // Bash does not define what should happen if a
                                        // \c is not followed by a recognized control
                                        // character.
                                        // We simply treat this case like other
                                        // unrecognized escape sequences, that is, we
                                        // preserve the escape sequence unchanged.
                                        arg.append('\\');
                                        arg.append('c');
                                        arg.append(c);
                                    }
                                    break;
                                }

                                // If inside an ANSI-quoted string, a backslash isn't
                                // followed by
                                // one of the recognized characters, both the backslash and
                                // the character are preserved.
                                default:
                                {
                                    arg.append('\\');
                                    arg.append(c);
                                    break;
                                }
                            }
                            break;
                        }
                        case '\'': // End of ANSI-quote mode.
                            {
                                state = NormalState;
                                break;
                            }
                        default:
                        {
                            arg.append(c); // Everything else is taken literally.
                            break;
                        }
                    }
                    break;
                }
                default:
                    assert false;
                    break;
            }
        }

        switch (state) {
            case NormalState:
            {
                vec.add(arg.toString());
                break;
            }
            case SingleQuoteState:
            {
                throw new ParseException("missing closing single quote");
            }
            case DoubleQuoteState:
            {
                throw new ParseException("missing closing double quote");
            }
            case ANSIQuoteState:
            {
                throw new ParseException("unterminated $' quote");
            }
            default:
            {
                assert false;
                break;
            }
        }

        return vec.toArray(new String[0]);
    }

    private Options() {}
}
