// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;

/**
 * @hidden Public because it's used by IceGridGUI.
 */
public final class StringUtil {
    /**
     * Returns the index of the first character in str to appear in match, starting from 0.
     *
     * @param str the string to search in
     * @param match the characters to search for
     * @return the index of the first matching character, or -1 if none is found
     */
    public static int findFirstOf(String str, String match) {
        return findFirstOf(str, match, 0);
    }

    /**
     * Returns the index of the first character in str to appear in match, starting from start.
     *
     * @param str the string to search in
     * @param match the characters to search for
     * @param start the starting index
     * @return the index of the first matching character, or -1 if none is found
     */
    public static int findFirstOf(String str, String match, int start) {
        final int len = str.length();
        for (int i = start; i < len; i++) {
            char ch = str.charAt(i);
            if (match.indexOf(ch) != -1) {
                return i;
            }
        }

        return -1;
    }

    /**
     * Returns the index of the first character in str which does not appear in match, starting from start.
     *
     * @param str the string to search in
     * @param match the characters to exclude
     * @param start the starting index
     * @return the index of the first non-matching character, or -1 if none is found
     */
    public static int findFirstNotOf(String str, String match, int start) {
        final int len = str.length();
        for (int i = start; i < len; i++) {
            char ch = str.charAt(i);
            if (match.indexOf(ch) == -1) {
                return i;
            }
        }

        return -1;
    }

    private static void encodeChar(
            char c, StringBuilder sb, String special, ToStringMode toStringMode) {
        switch (c) {
            case '\\':
            {
                sb.append("\\\\");
                break;
            }
            case '\'':
            {
                sb.append("\\'");
                break;
            }
            case '"':
            {
                sb.append("\\\"");
                break;
            }
            case '\007':
            {
                if (toStringMode == ToStringMode.Compat) {
                    // Octal escape for compatibility with 3.6 and earlier
                    sb.append("\\007");
                } else {
                    sb.append("\\a");
                }
                break;
            }
            case '\b':
            {
                sb.append("\\b");
                break;
            }
            case '\f':
            {
                sb.append("\\f");
                break;
            }
            case '\n':
            {
                sb.append("\\n");
                break;
            }
            case '\r':
            {
                sb.append("\\r");
                break;
            }
            case '\t':
            {
                sb.append("\\t");
                break;
            }
            case '\013':
            {
                if (toStringMode == ToStringMode.Compat) {
                    // Octal escape for compatibility with 3.6 and earlier
                    sb.append("\\013");
                } else {
                    sb.append("\\v");
                }
                break;
            }
            default:
            {
                if (special != null && special.indexOf(c) != -1) {
                    sb.append('\\');
                    sb.append(c);
                } else {
                    if (c < 32 || c > 126) {
                        if (toStringMode == ToStringMode.Compat) {
                            // When ToStringMode=Compat, c is a UTF-8 byte
                            assert (c < 256);

                            sb.append('\\');
                            String octal = Integer.toOctalString(c);

                            // Add leading zeroes so that we avoid problems during
                            // decoding. For example, consider the encoded string
                            // \0013 (i.e., a character with value 1 followed by
                            // the character '3'). If the leading zeroes were omitted,
                            // the result would be incorrectly interpreted by the
                            // decoder as a single character with value 11.
                            for (int j = octal.length(); j < 3; j++) {
                                sb.append('0');
                            }
                            sb.append(octal);
                        } else if (c < 32
                            || c == 127
                            || toStringMode == ToStringMode.ASCII) {
                            // append \\unnnn
                            sb.append("\\u");
                            String hex = Integer.toHexString(c);
                            for (int j = hex.length(); j < 4; j++) {
                                sb.append('0');
                            }
                            sb.append(hex);
                        } else {
                            // keep as is
                            sb.append(c);
                        }
                    } else {
                        // printable ASCII character
                        sb.append(c);
                    }
                }
                break;
            }
        }
    }

    /**
     * Adds escape sequences (like "\n") to the input string.
     *
     * @param s the string to escape
     * @param special additional characters to escape, or null
     * @param toStringMode the string mode for escaping
     * @return the escaped string
     */
    public static String escapeString(
            String s, String special, ToStringMode toStringMode) {
        if (special != null) {
            for (int i = 0; i < special.length(); i++) {
                if (special.charAt(i) < 32 || special.charAt(i) > 126) {
                    throw new IllegalArgumentException(
                        "special characters must be in ASCII range 32-126");
                }
            }
        }

        if (toStringMode == ToStringMode.Compat) {
            // Encode UTF-8 bytes

            byte[] bytes = null;
            try {
                bytes = s.getBytes("UTF8");
            } catch (UnsupportedEncodingException ex) {
                assert false;
                return null;
            }

            StringBuilder result = new StringBuilder(bytes.length);
            for (int i = 0; i < bytes.length; i++) {
                encodeChar((char) (bytes[i] & 0xFF), result, special, toStringMode);
            }

            return result.toString();
        } else {
            StringBuilder result = new StringBuilder(s.length());

            for (int i = 0; i < s.length(); i++) {
                char c = s.charAt(i);
                if (toStringMode == ToStringMode.Unicode
                    || !Character.isSurrogate(c)) {
                    encodeChar(c, result, special, toStringMode);
                } else {
                    assert (toStringMode == ToStringMode.ASCII
                        && Character.isSurrogate(c));
                    if (i + 1 == s.length()) {
                        throw new IllegalArgumentException("High surrogate without low surrogate");
                    } else {
                        i++;
                        int codePoint = Character.toCodePoint(c, s.charAt(i));
                        // append \Unnnnnnnn
                        result.append("\\U");
                        String hex = Integer.toHexString(codePoint);
                        for (int j = hex.length(); j < 8; j++) {
                            result.append('0');
                        }
                        result.append(hex);
                    }
                }
            }

            return result.toString();
        }
    }

    private static char checkChar(String s, int pos) {
        char c = s.charAt(pos);
        if (c < 32 || c == 127) {
            String msg;
            if (pos > 0) {
                msg = "character after `" + s.substring(0, pos) + "'";
            } else {
                msg = "first character";
            }
            msg += " has invalid ordinal value " + (int) c;
            throw new IllegalArgumentException(msg);
        }
        return c;
    }

    /**
     * Decode the character or escape sequence starting at start and appends it to result.
     *
     * @return the index of the first character following the decoded character or escape sequence.
     */
    private static int decodeChar(
            String s, int start, int end, String special, StringBuilder result) {
        assert (start >= 0);
        assert (start < end);
        assert (end <= s.length());

        if (s.charAt(start) != '\\') {
            result.append(checkChar(s, start++));
        } else if (start + 1 == end) {
            ++start;
            result.append('\\');
        } else {
            char c = s.charAt(++start);

            switch (c) {
                case '\\':
                case '\'':
                case '"':
                case '?':
                {
                    ++start;
                    result.append(c);
                    break;
                }
                case 'a':
                {
                    ++start;
                    result.append('\u0007');
                    break;
                }
                case 'b':
                {
                    ++start;
                    result.append('\b');
                    break;
                }
                case 'f':
                {
                    ++start;
                    result.append('\f');
                    break;
                }
                case 'n':
                {
                    ++start;
                    result.append('\n');
                    break;
                }
                case 'r':
                {
                    ++start;
                    result.append('\r');
                    break;
                }
                case 't':
                {
                    ++start;
                    result.append('\t');
                    break;
                }
                case 'v':
                {
                    ++start;
                    result.append('\u000b');
                    break;
                }
                case 'u':
                case 'U':
                {
                    int codePoint = 0;
                    boolean inBMP = c == 'u';
                    int size = inBMP ? 4 : 8;
                    ++start;
                    while (size > 0 && start < end) {
                        c = s.charAt(start++);
                        int charVal = 0;
                        if (c >= '0' && c <= '9') {
                            charVal = c - '0';
                        } else if (c >= 'a' && c <= 'f') {
                            charVal = 10 + (c - 'a');
                        } else if (c >= 'A' && c <= 'F') {
                            charVal = 10 + (c - 'A');
                        } else {
                            break; // while
                        }
                        codePoint = codePoint * 16 + charVal;
                        --size;
                    }
                    if (size > 0) {
                        throw new IllegalArgumentException(
                            "Invalid universal character name: too few hex digits");
                    }
                    if (codePoint >= 0xD800 && codePoint <= 0xDFFF) {
                        throw new IllegalArgumentException(
                            "A universal character name cannot designate a surrogate");
                    }
                    if (inBMP || Character.isBmpCodePoint(codePoint)) {
                        result.append((char) codePoint);
                    } else {
                        result.append(Character.toChars(codePoint));
                    }
                    break;
                }

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case 'x':
                {
                    // UTF-8 byte sequence encoded with octal or hex escapes

                    byte[] arr = new byte[end - start];
                    int i = 0;
                    boolean more = true;
                    while (more) {
                        int val = 0;
                        if (c == 'x') {
                            int size = 2;
                            ++start;
                            while (size > 0 && start < end) {
                                c = s.charAt(start++);
                                int charVal = 0;
                                if (c >= '0' && c <= '9') {
                                    charVal = c - '0';
                                } else if (c >= 'a' && c <= 'f') {
                                    charVal = 10 + (c - 'a');
                                } else if (c >= 'A' && c <= 'F') {
                                    charVal = 10 + (c - 'A');
                                } else {
                                    --start; // move back
                                    break; // while
                                }
                                val = val * 16 + charVal;
                                --size;
                            }
                            if (size == 2) {
                                throw new IllegalArgumentException(
                                    "Invalid \\x escape sequence: no hex digit");
                            }
                        } else {
                            for (int j = 0; j < 3 && start < end; j++) {
                                int charVal = s.charAt(start++) - '0';
                                if (charVal < 0 || charVal > 7) {
                                    --start; // move back
                                    assert (j != 0); // must be at least one digit
                                    break; // for
                                }
                                val = val * 8 + charVal;
                            }
                            if (val > 255) {
                                String msg =
                                    "octal value \\"
                                        + Integer.toOctalString(val)
                                        + " ("
                                        + val
                                        + ") is out of range";
                                throw new IllegalArgumentException(msg);
                            }
                        }

                        arr[i++] = (byte) val;

                        more = false;

                        if ((start + 1 < end) && s.charAt(start) == '\\') {
                            c = s.charAt(start + 1);
                            if (c == 'x' || (c >= '0' && c <= '9')) {
                                start++;
                                more = true;
                            }
                        }
                    }

                    try {
                        result.append(new String(arr, 0, i, "UTF8"));
                    } catch (UnsupportedEncodingException ex) {
                        throw new IllegalArgumentException("unsupported encoding", ex);
                    }
                    break;
                }
                default:
                {
                    if (special == null || special.isEmpty() || special.indexOf(c) == -1) {
                        result.append('\\'); // not in special, so we keep the backslash
                    }
                    result.append(checkChar(s, start++));
                    break;
                }
            }
        }

        return start;
    }

    /**
     * Removes escape sequences added by escapeString.
     *
     * @param s the string to unescape
     * @param start the starting index
     * @param end the ending index
     * @param special additional escaped characters to handle, or null
     * @return the unescaped string
     * @throws IllegalArgumentException for an invalid input string
     */
    public static String unescapeString(String s, int start, int end, String special) {
        assert (start >= 0 && start <= end && end <= s.length());

        if (special != null) {
            for (int i = 0; i < special.length(); i++) {
                if (special.charAt(i) < 32 || special.charAt(i) > 126) {
                    throw new IllegalArgumentException(
                        "special characters must be in ASCII range 32-126");
                }
            }
        }

        // Optimization for strings without escapes
        int p = s.indexOf('\\', start);
        if (p == -1 || p >= end) {
            p = start;
            while (p < end) {
                checkChar(s, p++);
            }
            return s.substring(start, end);
        } else {
            StringBuilder sb = new StringBuilder(end - start);
            while (start < end) {
                start = decodeChar(s, start, end, special, sb);
            }
            return sb.toString();
        }
    }

    /**
     * Splits a string using the specified delimiters, handling quoted sections.
     *
     * @param str the string to split
     * @param delim the delimiter characters
     * @return an array of split strings, or null for unmatched quotes
     */
    public static String[] splitString(String str, String delim) {
        List<String> l = new ArrayList<>();
        char[] arr = new char[str.length()];
        int pos = 0;

        int n = 0;
        char quoteChar = '\0';
        while (pos < str.length()) {
            if (quoteChar == '\0' && (str.charAt(pos) == '"' || str.charAt(pos) == '\'')) {
                quoteChar = str.charAt(pos++);
                continue; // Skip the quote.
            } else if (quoteChar == '\0'
                && str.charAt(pos) == '\\'
                && pos + 1 < str.length()
                && (str.charAt(pos + 1) == '"' || str.charAt(pos + 1) == '\'')) {
                ++pos; // Skip the backslash
            } else if (quoteChar != '\0'
                && str.charAt(pos) == '\\'
                && pos + 1 < str.length()
                && str.charAt(pos + 1) == quoteChar) {
                ++pos; // Skip the backslash
            } else if (quoteChar != '\0' && str.charAt(pos) == quoteChar) {
                ++pos;
                quoteChar = '\0';
                continue; // Skip the quote.
            } else if (delim.indexOf(str.charAt(pos)) != -1) {
                if (quoteChar == '\0') {
                    ++pos;
                    if (n > 0) {
                        l.add(new String(arr, 0, n));
                        n = 0;
                    }
                    continue;
                }
            }

            if (pos < str.length()) {
                arr[n++] = str.charAt(pos++);
            }
        }

        if (n > 0) {
            l.add(new String(arr, 0, n));
        }
        if (quoteChar != '\0') {
            return null; // Unmatched quote.
        }
        return l.toArray(new String[0]);
    }

    /**
     * Checks for a quotation mark at the start position and finds the matching closing quote.
     *
     * @param s the string to check
     * @param start the starting position
     * @return the position of the matching closing quote, 0 if no quote at start, or -1 if unmatched
     */
    public static int checkQuote(String s, int start) {
        char quoteChar = s.charAt(start);
        if (quoteChar == '"' || quoteChar == '\'') {
            start++;
            final int len = s.length();
            int pos;
            while (start < len && (pos = s.indexOf(quoteChar, start)) != -1) {
                if (s.charAt(pos - 1) != '\\') {
                    return pos;
                }
                start = pos + 1;
            }
            return -1; // Unmatched quote
        }
        return 0; // Not quoted
    }

    /**
     * Matches a string against a pattern that may contain wildcards.
     *
     * @param s the string to match
     * @param pat the pattern with optional wildcards (*)
     * @param emptyMatch whether to allow empty matches for wildcards
     * @return true if the string matches the pattern
     */
    public static boolean match(String s, String pat, boolean emptyMatch) {
        assert (!s.isEmpty());
        assert (!pat.isEmpty());

        // If pattern does not contain a wildcard just compare strings.
        int beginIndex = pat.indexOf('*');
        if (beginIndex < 0) {
            return s.equals(pat);
        }

        // Make sure start of the strings match
        if (beginIndex > s.length()
            || !s.substring(0, beginIndex).equals(pat.substring(0, beginIndex))) {
            return false;
        }

        // Make sure there is something present in the middle to match the wildcard.
        // If emptyMatch is true, allow a match of "".
        int endLength = pat.length() - beginIndex - 1;
        if (endLength == 0) {
            return true;
        }
        if (endLength > s.length()) {
            return false;
        }
        int endIndex = s.length() - endLength;
        if (endIndex < beginIndex || (!emptyMatch && endIndex == beginIndex)) {
            return false;
        }

        // Make sure end of the strings match
        if (!s.substring(endIndex, s.length() - endIndex)
            .equals(pat.substring(beginIndex + 1, pat.length() - beginIndex - 1))) {
            return false;
        }

        return true;
    }

    private StringUtil() {}
}
