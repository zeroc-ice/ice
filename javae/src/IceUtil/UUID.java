// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtil;

final public class UUID
{
    static class RandomByte extends java.util.Random
    {
	public byte
	nextByte()
	{
	    return (byte)next(8);
	}
    }
    
    private final static char[] hex = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    
    static String 
    byteToHexString(byte b)
    {
	char[] hexDigits = new char[2];
	hexDigits[0] = hex[(b & 0xF0) >> 4];
	hexDigits[1] = hex[(b & 0x0F)];
	return new String(hexDigits);
    }
    
    static public  String
    create()
    {
	byte[] data = new byte[16];
	
	for(int i = 0; i < data.length; ++i)
	{
	    data[i] = _rand.nextByte();
	}
	data[6] &= 0x0F;
	data[6] |= (4 << 4);
	data[8] &= 0x3F;
	data[8] |= 0x80;
	
	int index = 0;
	StringBuffer strRep = new StringBuffer(16 * 2 + 4);
	while(index < 4)
	{
	    strRep.append(byteToHexString(data[index]));
	    ++index;
	}
	strRep.append('-');
	while(index < 6)
	{
	    strRep.append(byteToHexString(data[index]));
	    ++index;
	}
	strRep.append('-');
	while(index < 8)
	{
	    strRep.append(byteToHexString(data[index]));
	    ++index;
	}
	strRep.append('-');
	while(index < 10)
	{
	    strRep.append(byteToHexString(data[index]));
	    ++index;
	}
	strRep.append('-');
	while(index < 16)
	{
	    strRep.append(byteToHexString(data[index]));
	    ++index;
	}
	
	return strRep.toString();
    }

    private static final RandomByte _rand = new RandomByte();
}
