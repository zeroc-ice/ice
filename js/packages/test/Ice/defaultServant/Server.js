// For JavaScript the Ice/defaultServant test is defined as a ClientServer test case because JavaScript does not support
// incoming connections. We just need to run the echo server and the JavaScript Client. This file "Server.js" prevents
// the test scripts for trying to start a C++ server, which in the case of Ice/defaultServant doesn't exist.
