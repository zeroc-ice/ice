This demo illustrates how to transfer serializable .NET classes
with Ice.

The .NET classes are transferred as byte sequences by Ice. It was
always possible to do this, but required you to explicitly serialize
your class into a byte sequence and then pass that sequence to
your operations to be deserialized by the receiver. You can now
accomplish the same thing more conveniently via metadata.

In your Slice definitions, you must declare a byte sequence using the
"clr:serializable" metadata and specify the .NET class name, as shown
below:

["clr:serializable:NETClassName"] sequence<byte> SliceType;

Now, wherever you use the declared Slice type in your operations or
data types, you can supply an instance of the designated .NET class
and Ice automatically converts it to and from the byte sequence that
is passed over the wire. (The .NET class must set the Serializable
attribute.)

Note that it is also necessary for your .NET class to be located in
an assembly that is referenced by both the client and server.

To run the demo, first start the server:

$ server.exe

In a separate window, start the client:

$ client.exe

The client allows you to toggle between sending a real class instance
and sending a null value, to show that passing null is supported.
