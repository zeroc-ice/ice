using System;
// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


class TwowaysAMI
{
	private static void  test(bool b)
	{
		if (!b)
		{
			throw new System.SystemException();
		}
	}
	
	private class Callback
	{
		internal Callback()
		{
			_called = false;
		}
		
		//UPGRADE_NOTE: Synchronized keyword was removed from method 'check'. Lock expression was added. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1027"'
		public virtual bool check()
		{
			lock (this)
			{
				while (!_called)
				{
					try
					{
						System.Threading.Monitor.Wait(Enclosing_Instance, TimeSpan.FromMilliseconds(5000));
					}
					catch (System.Threading.ThreadInterruptedException ex)
					{
						continue;
					}
					
					if (!_called)
					{
						return false; // Must be timeout.
					}
				}
				
				return true;
			}
		}
		
		//UPGRADE_NOTE: Synchronized keyword was removed from method 'called'. Lock expression was added. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1027"'
		public virtual void  called()
		{
			lock (this)
			{
				assert(!_called);
				_called = true;
				System.Threading.Monitor.Pulse(Enclosing_Instance);
			}
		}
		
		private bool _called;
	}
	
	
	private class AMI_MyClass_opVoidI:Test.AMI_MyClass_opVoid
	{
		public AMI_MyClass_opVoidI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response()
		{
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opByteI:Test.AMI_MyClass_opByte
	{
		public AMI_MyClass_opByteI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(sbyte r, sbyte b)
		{
			TwowaysAMI.test(b == (sbyte) SupportClass.Identity(0xf0));
			TwowaysAMI.test(r == (sbyte) SupportClass.Identity(0xff));
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opBoolI:Test.AMI_MyClass_opBool
	{
		public AMI_MyClass_opBoolI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(bool r, bool b)
		{
			TwowaysAMI.test(b);
			TwowaysAMI.test(!r);
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opShortIntLongI:Test.AMI_MyClass_opShortIntLong
	{
		public AMI_MyClass_opShortIntLongI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(long r, short s, int i, long l)
		{
			TwowaysAMI.test(s == 10);
			TwowaysAMI.test(i == 11);
			TwowaysAMI.test(l == 12);
			TwowaysAMI.test(r == 12);
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opFloatDoubleI:Test.AMI_MyClass_opFloatDouble
	{
		public AMI_MyClass_opFloatDoubleI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(double r, float f, double d)
		{
			TwowaysAMI.test(f == 3.14f);
			TwowaysAMI.test(d == 1.1e10);
			TwowaysAMI.test(r == 1.1e10);
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opStringI:Test.AMI_MyClass_opString
	{
		public AMI_MyClass_opStringI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(System.String r, System.String s)
		{
			TwowaysAMI.test(s.Equals("world hello"));
			TwowaysAMI.test(r.Equals("hello world"));
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opMyEnumI:Test.AMI_MyClass_opMyEnum
	{
		public AMI_MyClass_opMyEnumI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(Test.MyEnum r, Test.MyEnum e)
		{
			TwowaysAMI.test(e == Test.MyEnum.enum2);
			TwowaysAMI.test(r == Test.MyEnum.enum3);
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opMyClassI:Test.AMI_MyClass_opMyClass
	{
		public AMI_MyClass_opMyClassI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(Test.MyClassPrx r, Test.MyClassPrx c1, Test.MyClassPrx c2)
		{
			TwowaysAMI.test(c1.ice_getIdentity().equals(Ice.Util.stringToIdentity("test")));
			TwowaysAMI.test(c2.ice_getIdentity().equals(Ice.Util.stringToIdentity("noSuchIdentity")));
			TwowaysAMI.test(r.ice_getIdentity().equals(Ice.Util.stringToIdentity("test")));
			r.opVoid();
			c1.opVoid();
			try
			{
				c2.opVoid();
				TwowaysAMI.test(false);
			}
			catch (Ice.ObjectNotExistException ex)
			{
			}
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opStructI:Test.AMI_MyClass_opStruct
	{
		public AMI_MyClass_opStructI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(Test.Structure rso, Test.Structure so)
		{
			TwowaysAMI.test(rso.p == null);
			TwowaysAMI.test(rso.e == Test.MyEnum.enum2);
			TwowaysAMI.test(rso.s.s.equals("def"));
			TwowaysAMI.test(so.e == Test.MyEnum.enum3);
			TwowaysAMI.test(so.s.s.equals("a new string"));
			so.p.opVoid();
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opByteSI:Test.AMI_MyClass_opByteS
	{
		public AMI_MyClass_opByteSI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(sbyte[] rso, sbyte[] bso)
		{
			TwowaysAMI.test(bso.Length == 4);
			TwowaysAMI.test(bso[0] == (sbyte) 0x22);
			TwowaysAMI.test(bso[1] == (sbyte) 0x12);
			TwowaysAMI.test(bso[2] == (sbyte) 0x11);
			TwowaysAMI.test(bso[3] == (sbyte) 0x01);
			TwowaysAMI.test(rso.Length == 8);
			TwowaysAMI.test(rso[0] == (sbyte) 0x01);
			TwowaysAMI.test(rso[1] == (sbyte) 0x11);
			TwowaysAMI.test(rso[2] == (sbyte) 0x12);
			TwowaysAMI.test(rso[3] == (sbyte) 0x22);
			TwowaysAMI.test(rso[4] == (sbyte) SupportClass.Identity(0xf1));
			TwowaysAMI.test(rso[5] == (sbyte) SupportClass.Identity(0xf2));
			TwowaysAMI.test(rso[6] == (sbyte) SupportClass.Identity(0xf3));
			TwowaysAMI.test(rso[7] == (sbyte) SupportClass.Identity(0xf4));
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opBoolSI:Test.AMI_MyClass_opBoolS
	{
		public AMI_MyClass_opBoolSI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(bool[] rso, bool[] bso)
		{
			TwowaysAMI.test(bso.Length == 4);
			TwowaysAMI.test(bso[0]);
			TwowaysAMI.test(bso[1]);
			TwowaysAMI.test(!bso[2]);
			TwowaysAMI.test(!bso[3]);
			TwowaysAMI.test(rso.Length == 3);
			TwowaysAMI.test(!rso[0]);
			TwowaysAMI.test(rso[1]);
			TwowaysAMI.test(rso[2]);
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opShortIntLongSI:Test.AMI_MyClass_opShortIntLongS
	{
		public AMI_MyClass_opShortIntLongSI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(long[] rso, short[] sso, int[] iso, long[] lso)
		{
			TwowaysAMI.test(sso.Length == 3);
			TwowaysAMI.test(sso[0] == 1);
			TwowaysAMI.test(sso[1] == 2);
			TwowaysAMI.test(sso[2] == 3);
			TwowaysAMI.test(iso.Length == 4);
			TwowaysAMI.test(iso[0] == 8);
			TwowaysAMI.test(iso[1] == 7);
			TwowaysAMI.test(iso[2] == 6);
			TwowaysAMI.test(iso[3] == 5);
			TwowaysAMI.test(lso.Length == 6);
			TwowaysAMI.test(lso[0] == 10);
			TwowaysAMI.test(lso[1] == 30);
			TwowaysAMI.test(lso[2] == 20);
			TwowaysAMI.test(lso[3] == 10);
			TwowaysAMI.test(lso[4] == 30);
			TwowaysAMI.test(lso[5] == 20);
			TwowaysAMI.test(rso.Length == 3);
			TwowaysAMI.test(rso[0] == 10);
			TwowaysAMI.test(rso[1] == 30);
			TwowaysAMI.test(rso[2] == 20);
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opFloatDoubleSI:Test.AMI_MyClass_opFloatDoubleS
	{
		public AMI_MyClass_opFloatDoubleSI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(double[] rso, float[] fso, double[] dso)
		{
			TwowaysAMI.test(fso.Length == 2);
			TwowaysAMI.test(fso[0] == 3.14f);
			TwowaysAMI.test(fso[1] == 1.11f);
			TwowaysAMI.test(dso.Length == 3);
			TwowaysAMI.test(dso[0] == 1.3e10);
			TwowaysAMI.test(dso[1] == 1.2e10);
			TwowaysAMI.test(dso[2] == 1.1e10);
			TwowaysAMI.test(rso.Length == 5);
			TwowaysAMI.test(rso[0] == 1.1e10);
			TwowaysAMI.test(rso[1] == 1.2e10);
			TwowaysAMI.test(rso[2] == 1.3e10);
			TwowaysAMI.test((float) rso[3] == 3.14f);
			TwowaysAMI.test((float) rso[4] == 1.11f);
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opStringSI:Test.AMI_MyClass_opStringS
	{
		public AMI_MyClass_opStringSI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(System.String[] rso, System.String[] sso)
		{
			TwowaysAMI.test(sso.Length == 4);
			TwowaysAMI.test(sso[0].Equals("abc"));
			TwowaysAMI.test(sso[1].Equals("de"));
			TwowaysAMI.test(sso[2].Equals("fghi"));
			TwowaysAMI.test(sso[3].Equals("xyz"));
			TwowaysAMI.test(rso.Length == 3);
			TwowaysAMI.test(rso[0].Equals("fghi"));
			TwowaysAMI.test(rso[1].Equals("de"));
			TwowaysAMI.test(rso[2].Equals("abc"));
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opByteSSI:Test.AMI_MyClass_opByteSS
	{
		public AMI_MyClass_opByteSSI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(sbyte[][] rso, sbyte[][] bso)
		{
			TwowaysAMI.test(bso.Length == 2);
			TwowaysAMI.test(bso[0].Length == 1);
			TwowaysAMI.test(bso[0][0] == (sbyte) SupportClass.Identity(0xff));
			TwowaysAMI.test(bso[1].Length == 3);
			TwowaysAMI.test(bso[1][0] == (sbyte) 0x01);
			TwowaysAMI.test(bso[1][1] == (sbyte) 0x11);
			TwowaysAMI.test(bso[1][2] == (sbyte) 0x12);
			TwowaysAMI.test(rso.Length == 4);
			TwowaysAMI.test(rso[0].Length == 3);
			TwowaysAMI.test(rso[0][0] == (sbyte) 0x01);
			TwowaysAMI.test(rso[0][1] == (sbyte) 0x11);
			TwowaysAMI.test(rso[0][2] == (sbyte) 0x12);
			TwowaysAMI.test(rso[1].Length == 1);
			TwowaysAMI.test(rso[1][0] == (sbyte) SupportClass.Identity(0xff));
			TwowaysAMI.test(rso[2].Length == 1);
			TwowaysAMI.test(rso[2][0] == (sbyte) 0x0e);
			TwowaysAMI.test(rso[3].Length == 2);
			TwowaysAMI.test(rso[3][0] == (sbyte) SupportClass.Identity(0xf2));
			TwowaysAMI.test(rso[3][1] == (sbyte) SupportClass.Identity(0xf1));
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opBoolSSI:Test.AMI_MyClass_opBoolSS
	{
		public AMI_MyClass_opBoolSSI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(bool[][] rso, bool[][] bso)
		{
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opShortIntLongSSI:Test.AMI_MyClass_opShortIntLongSS
	{
		public AMI_MyClass_opShortIntLongSSI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(long[][] rso, short[][] sso, int[][] iso, long[][] lso)
		{
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opFloatDoubleSSI:Test.AMI_MyClass_opFloatDoubleSS
	{
		public AMI_MyClass_opFloatDoubleSSI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(double[][] rso, float[][] fso, double[][] dso)
		{
			TwowaysAMI.test(fso.Length == 3);
			TwowaysAMI.test(fso[0].Length == 1);
			TwowaysAMI.test(fso[0][0] == 3.14f);
			TwowaysAMI.test(fso[1].Length == 1);
			TwowaysAMI.test(fso[1][0] == 1.11f);
			TwowaysAMI.test(fso[2].Length == 0);
			TwowaysAMI.test(dso.Length == 1);
			TwowaysAMI.test(dso[0].Length == 3);
			TwowaysAMI.test(dso[0][0] == 1.1e10);
			TwowaysAMI.test(dso[0][1] == 1.2e10);
			TwowaysAMI.test(dso[0][2] == 1.3e10);
			TwowaysAMI.test(rso.Length == 2);
			TwowaysAMI.test(rso[0].Length == 3);
			TwowaysAMI.test(rso[0][0] == 1.1e10);
			TwowaysAMI.test(rso[0][1] == 1.2e10);
			TwowaysAMI.test(rso[0][2] == 1.3e10);
			TwowaysAMI.test(rso[1].Length == 3);
			TwowaysAMI.test(rso[1][0] == 1.1e10);
			TwowaysAMI.test(rso[1][1] == 1.2e10);
			TwowaysAMI.test(rso[1][2] == 1.3e10);
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opStringSSI:Test.AMI_MyClass_opStringSS
	{
		public AMI_MyClass_opStringSSI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response(System.String[][] rso, System.String[][] sso)
		{
			TwowaysAMI.test(sso.Length == 5);
			TwowaysAMI.test(sso[0].Length == 1);
			TwowaysAMI.test(sso[0][0].Equals("abc"));
			TwowaysAMI.test(sso[1].Length == 2);
			TwowaysAMI.test(sso[1][0].Equals("de"));
			TwowaysAMI.test(sso[1][1].Equals("fghi"));
			TwowaysAMI.test(sso[2].Length == 0);
			TwowaysAMI.test(sso[3].Length == 0);
			TwowaysAMI.test(sso[4].Length == 1);
			TwowaysAMI.test(sso[4][0].Equals("xyz"));
			TwowaysAMI.test(rso.Length == 3);
			TwowaysAMI.test(rso[0].Length == 1);
			TwowaysAMI.test(rso[0][0].Equals("xyz"));
			TwowaysAMI.test(rso[1].Length == 0);
			TwowaysAMI.test(rso[2].Length == 0);
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opByteBoolDI:Test.AMI_MyClass_opByteBoolD
	{
		public AMI_MyClass_opByteBoolDI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
		public virtual void  ice_response(java.util.Map ro, java.util.Map _do)
		{
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map di1 = new java.util.HashMap();
			di1.put((sbyte) 10, true);
			di1.put((sbyte) 100, false);
			TwowaysAMI.test(_do.equals(di1));
			TwowaysAMI.test(ro.size() == 4);
			TwowaysAMI.test(((System.Boolean) ro.get((sbyte) 10)) == true);
			TwowaysAMI.test(((System.Boolean) ro.get((sbyte) 11)) == false);
			TwowaysAMI.test(((System.Boolean) ro.get((sbyte) 100)) == false);
			TwowaysAMI.test(((System.Boolean) ro.get((sbyte) 101)) == true);
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opShortIntDI:Test.AMI_MyClass_opShortIntD
	{
		public AMI_MyClass_opShortIntDI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
		public virtual void  ice_response(java.util.Map ro, java.util.Map _do)
		{
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map di1 = new java.util.HashMap();
			di1.put((short) 110, - 1);
			di1.put((short) 1100, 123123);
			TwowaysAMI.test(_do.equals(di1));
			TwowaysAMI.test(ro.size() == 4);
			TwowaysAMI.test(((System.Int32) ro.get((short) 110)) == - 1);
			TwowaysAMI.test(((System.Int32) ro.get((short) 111)) == - 100);
			TwowaysAMI.test(((System.Int32) ro.get((short) 1100)) == 123123);
			TwowaysAMI.test(((System.Int32) ro.get((short) 1101)) == 0);
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opLongFloatDI:Test.AMI_MyClass_opLongFloatD
	{
		public AMI_MyClass_opLongFloatDI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
		public virtual void  ice_response(java.util.Map ro, java.util.Map _do)
		{
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map di1 = new java.util.HashMap();
			di1.put(999999110L, - 1.1f);
			di1.put(999999111L, 123123.2f);
			TwowaysAMI.test(_do.equals(di1));
			TwowaysAMI.test(ro.size() == 4);
			//UPGRADE_TODO: The equivalent in .NET for method 'java.lang.Float.floatValue' may return a different value. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1043"'
			TwowaysAMI.test((float) ((System.Single) ro.get(999999110L)) == - 1.1f);
			//UPGRADE_TODO: The equivalent in .NET for method 'java.lang.Float.floatValue' may return a different value. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1043"'
			TwowaysAMI.test((float) ((System.Single) ro.get(999999120L)) == - 100.4f);
			//UPGRADE_TODO: The equivalent in .NET for method 'java.lang.Float.floatValue' may return a different value. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1043"'
			TwowaysAMI.test((float) ((System.Single) ro.get(999999111L)) == 123123.2f);
			//UPGRADE_TODO: The equivalent in .NET for method 'java.lang.Float.floatValue' may return a different value. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1043"'
			TwowaysAMI.test((float) ((System.Single) ro.get(999999130L)) == 0.5f);
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opStringStringDI:Test.AMI_MyClass_opStringStringD
	{
		public AMI_MyClass_opStringStringDI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
		public virtual void  ice_response(java.util.Map ro, java.util.Map _do)
		{
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map di1 = new java.util.HashMap();
			di1.put("foo", "abc -1.1");
			di1.put("bar", "abc 123123.2");
			TwowaysAMI.test(_do.equals(di1));
			TwowaysAMI.test(ro.size() == 4);
			TwowaysAMI.test(((System.String) ro.get("foo")).Equals("abc -1.1"));
			TwowaysAMI.test(((System.String) ro.get("FOO")).Equals("abc -100.4"));
			TwowaysAMI.test(((System.String) ro.get("bar")).Equals("abc 123123.2"));
			TwowaysAMI.test(((System.String) ro.get("BAR")).Equals("abc 0.5"));
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opStringMyEnumDI:Test.AMI_MyClass_opStringMyEnumD
	{
		public AMI_MyClass_opStringMyEnumDI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
		public virtual void  ice_response(java.util.Map ro, java.util.Map _do)
		{
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map di1 = new java.util.HashMap();
			di1.put("abc", Test.MyEnum.enum1);
			di1.put("", Test.MyEnum.enum2);
			TwowaysAMI.test(_do.equals(di1));
			TwowaysAMI.test(ro.size() == 4);
			TwowaysAMI.test(((Test.MyEnum) ro.get("abc")) == Test.MyEnum.enum1);
			TwowaysAMI.test(((Test.MyEnum) ro.get("qwerty")) == Test.MyEnum.enum3);
			TwowaysAMI.test(((Test.MyEnum) ro.get("")) == Test.MyEnum.enum2);
			TwowaysAMI.test(((Test.MyEnum) ro.get("Hello!!")) == Test.MyEnum.enum2);
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opIntSI:Test.AMI_MyClass_opIntS
	{
		private void  InitBlock()
		{
			callback = new Callback();
		}
		internal AMI_MyClass_opIntSI(int l)
		{
			_l = l;
		}
		
		public virtual void  ice_response(int[] r)
		{
			TwowaysAMI.test(r.Length == _l);
			for (int j = 0; j < r.Length; ++j)
			{
				TwowaysAMI.test(r[j] == - j);
			}
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		private int _l;
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opContextEqualI:Test.AMI_MyClass_opContext
	{
		private void  InitBlock()
		{
			callback = new Callback();
		}
		//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
		internal AMI_MyClass_opContextEqualI(java.util.Map d)
		{
			_d = d;
		}
		
		//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
		public virtual void  ice_response(java.util.Map r)
		{
			TwowaysAMI.test(r.equals(_d));
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
		private java.util.Map _d;
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyClass_opContextNotEqualI:Test.AMI_MyClass_opContext
	{
		private void  InitBlock()
		{
			callback = new Callback();
		}
		//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
		internal AMI_MyClass_opContextNotEqualI(java.util.Map d)
		{
			_d = d;
		}
		
		//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
		public virtual void  ice_response(java.util.Map r)
		{
			TwowaysAMI.test(!r.equals(_d));
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
		private java.util.Map _d;
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	private class AMI_MyDerivedClass_opDerivedI:Test.AMI_MyDerivedClass_opDerived
	{
		public AMI_MyDerivedClass_opDerivedI()
		{
			InitBlock();
		}
		private void  InitBlock()
		{
			callback = new Callback();
		}
		public virtual void  ice_response()
		{
			callback.called();
		}
		
		public virtual void  ice_exception(Ice.LocalException ex)
		{
			TwowaysAMI.test(false);
		}
		
		public virtual bool check()
		{
			return callback.check();
		}
		
		//UPGRADE_NOTE: The initialization of  'callback' was moved to method 'InitBlock'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
		private Callback callback;
	}
	
	
	internal static void  twowaysAMI(Test.MyClassPrx p)
	{
		{
			AMI_MyClass_opVoidI cb = new AMI_MyClass_opVoidI();
			p.opVoid_async(cb);
			test(cb.check());
		}
		
		{
			AMI_MyClass_opByteI cb = new AMI_MyClass_opByteI();
			p.opByte_async(cb, (sbyte) SupportClass.Identity(0xff), (sbyte) 0x0f);
			test(cb.check());
		}
		
		{
			AMI_MyClass_opBoolI cb = new AMI_MyClass_opBoolI();
			p.opBool_async(cb, true, false);
			test(cb.check());
		}
		
		{
			AMI_MyClass_opShortIntLongI cb = new AMI_MyClass_opShortIntLongI();
			p.opShortIntLong_async(cb, (short) 10, 11, 12L);
			test(cb.check());
		}
		
		{
			AMI_MyClass_opFloatDoubleI cb = new AMI_MyClass_opFloatDoubleI();
			p.opFloatDouble_async(cb, 3.14f, 1.1e10);
			test(cb.check());
		}
		
		{
			AMI_MyClass_opStringI cb = new AMI_MyClass_opStringI();
			p.opString_async(cb, "hello", "world");
			test(cb.check());
		}
		
		{
			AMI_MyClass_opMyEnumI cb = new AMI_MyClass_opMyEnumI();
			p.opMyEnum_async(cb, Test.MyEnum.enum2);
			test(cb.check());
		}
		
		{
			AMI_MyClass_opMyClassI cb = new AMI_MyClass_opMyClassI();
			p.opMyClass_async(cb, p);
			test(cb.check());
		}
		
		{
			Test.Structure si1 = new Test.Structure();
			si1.p = p;
			si1.e = Test.MyEnum.enum3;
			si1.s = new Test.AnotherStruct();
			si1.s.s = "abc";
			Test.Structure si2 = new Test.Structure();
			si2.p = null;
			si2.e = Test.MyEnum.enum2;
			si2.s = new Test.AnotherStruct();
			si2.s.s = "def";
			
			AMI_MyClass_opStructI cb = new AMI_MyClass_opStructI();
			p.opStruct_async(cb, si1, si2);
			test(cb.check());
		}
		
		{
			//UPGRADE_NOTE: Final was removed from the declaration of 'bsi1 '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			sbyte[] bsi1 = new sbyte[]{(sbyte) 0x01, (sbyte) 0x11, (sbyte) 0x12, (sbyte) 0x22};
			//UPGRADE_NOTE: Final was removed from the declaration of 'bsi2 '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			sbyte[] bsi2 = new sbyte[]{(sbyte) SupportClass.Identity(0xf1), (sbyte) SupportClass.Identity(0xf2), (sbyte) SupportClass.Identity(0xf3), (sbyte) SupportClass.Identity(0xf4)};
			
			AMI_MyClass_opByteSI cb = new AMI_MyClass_opByteSI();
			p.opByteS_async(cb, bsi1, bsi2);
			test(cb.check());
		}
		
		{
			//UPGRADE_NOTE: Final was removed from the declaration of 'bsi1 '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			bool[] bsi1 = new bool[]{true, true, false};
			//UPGRADE_NOTE: Final was removed from the declaration of 'bsi2 '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			bool[] bsi2 = new bool[]{false};
			
			AMI_MyClass_opBoolSI cb = new AMI_MyClass_opBoolSI();
			p.opBoolS_async(cb, bsi1, bsi2);
			test(cb.check());
		}
		
		{
			//UPGRADE_NOTE: Final was removed from the declaration of 'ssi '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			short[] ssi = new short[]{1, 2, 3};
			//UPGRADE_NOTE: Final was removed from the declaration of 'isi '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			int[] isi = new int[]{5, 6, 7, 8};
			//UPGRADE_NOTE: Final was removed from the declaration of 'lsi '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			long[] lsi = new long[]{10, 30, 20};
			
			AMI_MyClass_opShortIntLongSI cb = new AMI_MyClass_opShortIntLongSI();
			p.opShortIntLongS_async(cb, ssi, isi, lsi);
			test(cb.check());
		}
		
		{
			//UPGRADE_NOTE: Final was removed from the declaration of 'fsi '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			float[] fsi = new float[]{3.14f, 1.11f};
			//UPGRADE_NOTE: Final was removed from the declaration of 'dsi '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			double[] dsi = new double[]{1.1e10, 1.2e10, 1.3e10};
			
			AMI_MyClass_opFloatDoubleSI cb = new AMI_MyClass_opFloatDoubleSI();
			p.opFloatDoubleS_async(cb, fsi, dsi);
			test(cb.check());
		}
		
		{
			//UPGRADE_NOTE: Final was removed from the declaration of 'ssi1 '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			System.String[] ssi1 = new System.String[]{"abc", "de", "fghi"};
			//UPGRADE_NOTE: Final was removed from the declaration of 'ssi2 '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			System.String[] ssi2 = new System.String[]{"xyz"};
			
			AMI_MyClass_opStringSI cb = new AMI_MyClass_opStringSI();
			p.opStringS_async(cb, ssi1, ssi2);
			test(cb.check());
		}
		
		{
			//UPGRADE_NOTE: Final was removed from the declaration of 'bsi1 '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			sbyte[][] bsi1 = {new sbyte[]{(sbyte) 0x01, (sbyte) 0x11, (sbyte) 0x12}, new sbyte[]{(sbyte) SupportClass.Identity(0xff)}};
			//UPGRADE_NOTE: Final was removed from the declaration of 'bsi2 '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			sbyte[][] bsi2 = {new sbyte[]{(sbyte) 0x0e}, new sbyte[]{(sbyte) SupportClass.Identity(0xf2), (sbyte) SupportClass.Identity(0xf1)}};
			
			AMI_MyClass_opByteSSI cb = new AMI_MyClass_opByteSSI();
			p.opByteSS_async(cb, bsi1, bsi2);
			test(cb.check());
		}
		
		{
			//UPGRADE_NOTE: Final was removed from the declaration of 'fsi '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			float[][] fsi = {new float[]{3.14f}, new float[]{1.11f}, new float[]{}};
			//UPGRADE_NOTE: Final was removed from the declaration of 'dsi '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			double[][] dsi = {new double[]{1.1e10, 1.2e10, 1.3e10}};
			
			AMI_MyClass_opFloatDoubleSSI cb = new AMI_MyClass_opFloatDoubleSSI();
			p.opFloatDoubleSS_async(cb, fsi, dsi);
			test(cb.check());
		}
		
		{
			//UPGRADE_NOTE: Final was removed from the declaration of 'ssi1 '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			System.String[][] ssi1 = {new System.String[]{"abc"}, new System.String[]{"de", "fghi"}};
			//UPGRADE_NOTE: Final was removed from the declaration of 'ssi2 '. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1003"'
			System.String[][] ssi2 = {new System.String[]{}, new System.String[]{}, new System.String[]{"xyz"}};
			
			AMI_MyClass_opStringSSI cb = new AMI_MyClass_opStringSSI();
			p.opStringSS_async(cb, ssi1, ssi2);
			test(cb.check());
		}
		
		{
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map di1 = new java.util.HashMap();
			di1.put((sbyte) 10, true);
			di1.put((sbyte) 100, false);
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map di2 = new java.util.HashMap();
			di2.put((sbyte) 10, true);
			di2.put((sbyte) 11, false);
			di2.put((sbyte) 101, true);
			
			AMI_MyClass_opByteBoolDI cb = new AMI_MyClass_opByteBoolDI();
			p.opByteBoolD_async(cb, di1, di2);
			test(cb.check());
		}
		
		{
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map di1 = new java.util.HashMap();
			di1.put((short) 110, - 1);
			di1.put((short) 1100, 123123);
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map di2 = new java.util.HashMap();
			di2.put((short) 110, - 1);
			di2.put((short) 111, - 100);
			di2.put((short) 1101, 0);
			
			AMI_MyClass_opShortIntDI cb = new AMI_MyClass_opShortIntDI();
			p.opShortIntD_async(cb, di1, di2);
			test(cb.check());
		}
		
		{
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map di1 = new java.util.HashMap();
			di1.put(999999110L, - 1.1f);
			di1.put(999999111L, 123123.2f);
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map di2 = new java.util.HashMap();
			di2.put(999999110L, - 1.1f);
			di2.put(999999120L, - 100.4f);
			di2.put(999999130L, 0.5f);
			
			AMI_MyClass_opLongFloatDI cb = new AMI_MyClass_opLongFloatDI();
			p.opLongFloatD_async(cb, di1, di2);
			test(cb.check());
		}
		
		{
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map di1 = new java.util.HashMap();
			di1.put("foo", "abc -1.1");
			di1.put("bar", "abc 123123.2");
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map di2 = new java.util.HashMap();
			di2.put("foo", "abc -1.1");
			di2.put("FOO", "abc -100.4");
			di2.put("BAR", "abc 0.5");
			
			AMI_MyClass_opStringStringDI cb = new AMI_MyClass_opStringStringDI();
			p.opStringStringD_async(cb, di1, di2);
			test(cb.check());
		}
		
		{
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map di1 = new java.util.HashMap();
			di1.put("abc", Test.MyEnum.enum1);
			di1.put("", Test.MyEnum.enum2);
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map di2 = new java.util.HashMap();
			di2.put("abc", Test.MyEnum.enum1);
			di2.put("qwerty", Test.MyEnum.enum3);
			di2.put("Hello!!", Test.MyEnum.enum2);
			
			AMI_MyClass_opStringMyEnumDI cb = new AMI_MyClass_opStringMyEnumDI();
			p.opStringMyEnumD_async(cb, di1, di2);
			test(cb.check());
		}
		
		{
			int[] lengths = new int[]{0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000};
			
			for (int l = 0; l < lengths.Length; ++l)
			{
				int[] s = new int[lengths[l]];
				for (int i = 0; i < s.Length; ++i)
				{
					s[i] = i;
				}
				AMI_MyClass_opIntSI cb = new AMI_MyClass_opIntSI(lengths[l]);
				p.opIntS_async(cb, s);
				test(cb.check());
			}
		}
		
		{
			//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
			java.util.Map ctx = new java.util.HashMap();
			ctx.put("one", "ONE");
			ctx.put("two", "TWO");
			ctx.put("three", "THREE");
			{
				test(p.ice_getContext().isEmpty());
				AMI_MyClass_opContextNotEqualI cb = new AMI_MyClass_opContextNotEqualI(ctx);
				p.opContext_async(cb);
				test(cb.check());
			}
			{
				test(p.ice_getContext().isEmpty());
				AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(ctx);
				p.opContext_async(cb, ctx);
				test(cb.check());
			}
			Test.MyClassPrx p2 = Test.MyClassPrxHelper.checkedCast(p.ice_newContext(ctx));
			test(p2.ice_getContext().equals(ctx));
			{
				AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(ctx);
				p2.opContext_async(cb);
				test(cb.check());
			}
			{
				AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(ctx);
				p2.opContext_async(cb, ctx);
				test(cb.check());
			}
		}
		
		{
			Test.MyDerivedClassPrx derived = Test.MyDerivedClassPrxHelper.checkedCast(p);
			test(derived != null);
			AMI_MyDerivedClass_opDerivedI cb = new AMI_MyDerivedClass_opDerivedI();
			derived.opDerived_async(cb);
			test(cb.check());
		}
	}
}
