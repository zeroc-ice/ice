// **********************************************************************
//
// Copyright (c) 2003
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

class PrinterI : Printer
{
    public override void printBackwards(Ice.Current current)
    {
        char[] arr = message.ToCharArray();
        for(int i = 0; i < arr.Length / 2; i++)
        {
            char tmp = arr[arr.Length - i - 1];
            arr[arr.Length - i - 1] = arr[i];
            arr[i] = tmp;
        }
        System.Console.Out.WriteLine(new string(arr));
    }
}
