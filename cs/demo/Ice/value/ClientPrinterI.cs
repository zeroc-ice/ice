// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

class ClientPrinterI : ClientPrinter
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
