// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

class PrinterI extends Printer
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
}
