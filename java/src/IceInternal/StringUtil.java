// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class StringUtil
{
    //
    // Return the index of the first character in str to
    // appear in match, starting from 0. Returns -1 if none is
    // found.
    //
    public static int
    findFirstOf(String str, String match)
    {
        return findFirstOf(str, match, 0);
    }

    //
    // Return the index of the first character in str to
    // appear in match, starting from start. Returns -1 if none is
    // found.
    //
    public static int
    findFirstOf(String str, String match, int start)
    {
        final int len = str.length();
        for(int i = start; i < len; i++)
        {
            char ch = str.charAt(i);
            if(match.indexOf(ch) != -1)
            {
                return i;
            }
        }

        return -1;
    }

    //
    // Return the index of the first character in str which does
    // not appear in match, starting from 0. Returns -1 if none is
    // found.
    //
    public static int
    findFirstNotOf(String str, String match)
    {
        return findFirstNotOf(str, match, 0);
    }

    //
    // Return the index of the first character in str which does
    // not appear in match, starting from start. Returns -1 if none is
    // found.
    //
    public static int
    findFirstNotOf(String str, String match, int start)
    {
        final int len = str.length();
        for(int i = start; i < len; i++)
        {
            char ch = str.charAt(i);
            if(match.indexOf(ch) == -1)
            {
                return i;
            }
        }

        return -1;
    }
}
