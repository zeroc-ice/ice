public interface IThreadRunnable
{
	void Run();
}

public class SupportClass
{
    /// <summary>
    /// Creates an instance of a received Type
    /// </summary>
    /// <param name="classType">The Type of the new class instance to return</param>
    /// <returns>An Object containing the new instance</returns>
    public static System.Object CreateNewInstance(System.Type classType)
    {
	System.Reflection.ConstructorInfo[] constructors = classType.GetConstructors();

	if(constructors.Length == 0)
	    return null;

	System.Reflection.ParameterInfo[] firstConstructor = constructors[0].GetParameters();
	int countParams = firstConstructor.Length;

	System.Type[] constructor = new System.Type[countParams];
	for(int i = 0; i < countParams; i++)
	{
	    constructor[i] = firstConstructor[i].ParameterType;
	}

	return classType.GetConstructor(constructor).Invoke(new System.Object[]{});
    }

	/*******************************/
	public class ThreadClass:IThreadRunnable
	{
		private System.Threading.Thread threadField;

		public ThreadClass()
		{
			threadField = new System.Threading.Thread(new System.Threading.ThreadStart(Run));
		}

		public ThreadClass(System.Threading.ThreadStart p1)
		{
			threadField = new System.Threading.Thread(p1);
		}

		public virtual void Run()
		{
		}

		public virtual void Start()
		{
			threadField.Start();
		}

		public System.Threading.Thread Instance
		{
			get
			{
				return threadField;
			}
			set
			{
				threadField	= value;
			}
		}

		public string Name
		{
			get
			{
				return threadField.Name;
			}
			set
			{
				if(threadField.Name == null)
					threadField.Name = value; 
			}
		}

		public System.Threading.ThreadPriority Priority
		{
			get
			{
				return threadField.Priority;
			}
			set
			{
				threadField.Priority = value;
			}
		}

		public bool IsAlive
		{
			get
			{
				return threadField.IsAlive;
			}
		}

		public bool IsBackground
		{
			get
			{
				return threadField.IsBackground;
			} 
			set
			{
				threadField.IsBackground = value;
			}
		}

		public void Join()
		{
			threadField.Join();
		}

		public void Join(long p1)
		{
			lock(this)
			{
				threadField.Join(new System.TimeSpan(p1 * 10000));
			}
		}

		public void Join(long p1, int p2)
		{
			lock(this)
			{
				threadField.Join(new System.TimeSpan(p1 * 10000 + p2 * 100));
			}
		}

		public void Resume()
		{
			threadField.Resume();
		}

		public void Abort()
		{
			threadField.Abort();
		}

		public void Abort(System.Object stateInfo)
		{
			lock(this)
			{
				threadField.Abort(stateInfo);
			}
		}

		public void Suspend()
		{
			threadField.Suspend();
		}

		public override string ToString()
		{
			return "Thread[" + Name + "," + Priority.ToString() + "," + "" + "]";
		}

		public static ThreadClass Current()
		{
			ThreadClass CurrentThread = new ThreadClass();
			CurrentThread.Instance = System.Threading.Thread.CurrentThread;
			return CurrentThread;
		}
	}

	/*******************************/
	public static void WriteStackTrace(System.Exception throwable, System.IO.TextWriter stream)
	{
		stream.Write(throwable.StackTrace);
		stream.Flush();
	}

	/*******************************/
	public class UdpClientSupport : System.Net.Sockets.UdpClient
	{
		public UdpClientSupport(int port) : base(port)
		{
		}

		public UdpClientSupport() : base()
		{
		}

		public UdpClientSupport(System.Net.IPEndPoint IP) : base(IP)
		{
		}

		public UdpClientSupport(string host, int port) : base(host,port)
		{
		}

		public static void Receive(System.Net.Sockets.UdpClient tempClient, out PacketSupport packet)
		{
			System.Net.IPEndPoint remoteIpEndPoint = 
				new System.Net.IPEndPoint(System.Net.IPAddress.Any, 0);

			PacketSupport tempPacket;
			try
			{
				byte[] data_in = tempClient.Receive(ref remoteIpEndPoint); 
				tempPacket = new PacketSupport(data_in, data_in.Length);
				tempPacket.IPEndPoint = remoteIpEndPoint;
			}
			catch ( System.Exception e )
			{
				throw new System.Exception(e.Message); 
			}
			packet = tempPacket;
		}

		public static void Send(System.Net.Sockets.UdpClient tempClient, PacketSupport packet)
		{
			tempClient.Send(packet.Data,packet.Length, packet.IPEndPoint);     
		}
	}

	/*******************************/
	public static sbyte[] ToSByteArray(byte[] byteArray)
	{
		sbyte[] sbyteArray = new sbyte[byteArray.Length];
		for(int index=0; index < byteArray.Length; index++)
			sbyteArray[index] = (sbyte) byteArray[index];
		return sbyteArray;
	}


	/// <summary>
	/// Converts a string to an array of bytes
	/// </summary>
	/// <param name="sourceString">The string to be converted</param>
	/// <returns>The new array of bytes</returns>
	public static byte[] ToByteArray(string sourceString)
	{
		byte[] byteArray = new byte[sourceString.Length];
		for (int index=0; index < sourceString.Length; index++)
			byteArray[index] = (byte) sourceString[index];
		return byteArray;
	}

	/*******************************/
	public class PacketSupport
	{
		private byte[] data;
		private int length;
		private System.Net.IPEndPoint ipEndPoint;

		public PacketSupport(byte[] data, int length)
		{
			if(length > data.Length)
				throw new System.ArgumentException("illegal length"); 

			this.data = data;
			this.length = length;
			this.ipEndPoint = null;
		}

		public PacketSupport(byte[] data, int length, System.Net.IPEndPoint ipendpoint)
		{
			if(length > data.Length)
				throw new System.ArgumentException("illegal length"); 

			this.data = data;
			this.length = length;
			this.ipEndPoint = ipendpoint;
		}

		public System.Net.IPEndPoint IPEndPoint
		{
			get 
			{
				return this.ipEndPoint;
			}
			set 
			{
				this.ipEndPoint = value;
			}
		}

		public int Port
		{
			get
			{
				return this.ipEndPoint.Port;
			}
			set
			{
				if(value < 0 || value > 0xFFFF)
					throw new System.ArgumentException("Port out of range:"+ value);

				this.ipEndPoint.Port = value;
			}
		}

		public int Length
		{
			get 
			{
				return this.length;
			}
			set
			{
				if(value > data.Length)
					throw new System.ArgumentException("illegal length"); 

				this.length = value;
			}
		}

		public byte[] Data
		{
			get 
			{
				return this.data;
			}

			set 
			{
				this.data = value;
			}
		}
	}

	/*******************************/
	/// <summary>
	/// This method is used as a dummy method to simulate VJ++ behavior
	/// </summary>
	/// <param name="literal">The literal to return</param>
	/// <returns>The received value</returns>
	public static long Identity(long literal)
	{
		return literal;
	}

	/// <summary>
	/// This method is used as a dummy method to simulate VJ++ behavior
	/// </summary>
	/// <param name="literal">The literal to return</param>
	/// <returns>The received value</returns>
	public static ulong Identity(ulong literal)
	{
		return literal;
	}

	/// <summary>
	/// This method is used as a dummy method to simulate VJ++ behavior
	/// </summary>
	/// <param name="literal">The literal to return</param>
	/// <returns>The received value</returns>
	public static float Identity(float literal)
	{
		return literal;
	}

	/// <summary>
	/// This method is used as a dummy method to simulate VJ++ behavior
	/// </summary>
	/// <param name="literal">The literal to return</param>
	/// <returns>The received value</returns>
	public static double Identity(double literal)
	{
		return literal;
	}

}
