A simple throughput demo that allows you to send sequences of various
types between client and server and to measure the maximum bandwidth
that can be achieved using serialized synchronous requests.

To run the demo, first you need to start an Ice throughput server from
another language mapping (Java, C++, C#, or Python). Please refer to
the README in the server subdirectory for more information on starting
the server.

After starting the server, open a separate window and start the
client:

$ node Client.js

The performance for byte sequences is expected to be greater than
for other types because the cost of marshaling and unmarshaling is
lower than for more complex types.
