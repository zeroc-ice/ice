A more advanced throughput demo that allows you to send sequences of
various types between client and server and to measure the maximum
bandwidth that can be achieved using asynchronous requests and
replies.

To run the demo, first start the server:

$ server

In a separate window, start the client:

$ client

We utilize some optimizations for the byte sequence operations in this
demo. First, we use the "cpp:array" metadata for in parameters, which
means the servant implementation in the server receives the sequence
as a pair of pointers into the marshaling buffer, thereby eliminating
an extra copy of the sequence data.

We also use the "cpp:array" metadata for return values, along with
AMD. This also reduces the amount of copying that occurs while
marshaling the reply message.
