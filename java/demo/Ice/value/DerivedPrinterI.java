// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

class DerivedPrinterI extends DerivedPrinter
{
    public void
    printBackwards(Ice.Current current)
    {
        char[] arr = message.toCharArray();
        for(int i = 0; i < arr.length / 2; i++)
        {
            char tmp = arr[arr.length - i - 1];
            arr[arr.length - i - 1] = arr[i];
            arr[i] = tmp;
        }
        System.out.println(new String(arr));
    }

    public void
    printUppercase(Ice.Current current)
    {
        System.out.println(derivedMessage.toUpperCase());
    }
}
