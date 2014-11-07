This demo illustrates how to transfer serializable Java classes
with Ice.

The Java classes are transferred as byte sequences by Ice. It was
always possible to do this, but required you to explicitly serialize
your class into a byte sequence and then pass that sequence to your
operations to be deserialized by the receiver. You can now accomplish
the same thing more conveniently via metadata.

In your Slice definitions, you must declare a byte sequence using the
"java:serializable" metadata and specify the Java class name, as shown
below:

["java:serializable:JavaClassName"] sequence<byte> SliceType;

Now, wherever you use the declared Slice type in your operations or
data types, you can supply an instance of the designated Java class
and Ice automatically converts it to and from the byte sequence that
is passed over the wire. (The Java class you pass must derive from
java.io.Serializable.)

With the "java:serializable" metadata, if you have a serializable
class as an out-parameter, the out-parameter is passed as
Ice.Holder<JavaClassName>. For example, if you have a Java class
MyPackage.Car as an out-parameter, the out-parameter is passed as
Ice.Holder<MyPackage.Car>.

To run the demo, first start the server:

$ java -jar build/libs/server.jar

In a separate window, start the client:

$ java -jar build/libs/client.jar

The client allows you to toggle between sending a real class instance
and sending a null value, to show that passing null is supported.
