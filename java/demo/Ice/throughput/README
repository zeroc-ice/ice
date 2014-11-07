A simple throughput demo that allows you to send sequences of various
types between client and server and to measure the maximum bandwidth
that can be achieved using serialized synchronous requests.

To run the demo, first start the server:

$ java -jar build/libs/server.jar

In a separate window, start the client:

$ java -jar build/libs/client.jar

The performance for byte sequences is expected to be greater than 
for other types because the cost of marshaling and unmarshaling is
lower than for more complex types. With byte sequences there is no
need to allocate and deallocate objects, which adds overhead.
