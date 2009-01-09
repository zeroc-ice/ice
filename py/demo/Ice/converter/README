This demo illustrates how to use a string converter plug-in with Ice
for Python. In this demo, the client represents an application that
uses ISO-Latin-1 as its character set, while the server uses UTF-8.

The demo sends and receives the greeting "Bonne journée" which in
Latin-1 encoding is "Bonne journ\351e" and in UTF-8 encoding is 
"Bonne journ\303\251e".

The demo prints the strings as they are received to show how, without
conversion, they are not in the format expected by the application.

To run the demo, first start the server:

$ python Server.py

In a separate window, start the client:

$ python Client.py
