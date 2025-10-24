// Copyright (c) ZeroC, Inc.

namespace Ice.binding;

public class RemoteCommunicatorI : Test.RemoteCommunicatorDisp_
{
    public override Test.RemoteObjectAdapterPrx
    createObjectAdapter(string name, string endpoints, Ice.Current current)
    {
        int retry = 5;
        while (true)
        {
            try
            {
                Ice.Communicator communicator = current.adapter.getCommunicator();
                string endpts = endpoints;
                if (endpts.IndexOf("-p", StringComparison.Ordinal) < 0)
                {
                    endpts =
                        global::Test.TestHelper.getTestEndpoint(communicator.getProperties(), _nextPort++, endpts);
                }

                communicator.getProperties().setProperty(name + ".ThreadPool.Size", "1");
                Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints(name, endpts);
                return Test.RemoteObjectAdapterPrxHelper.uncheckedCast(
                    current.adapter.addWithUUID(new RemoteObjectAdapterI(adapter)));
            }
            catch (Ice.SocketException)
            {
                if (--retry == 0)
                {
                    throw;
                }
            }
        }
    }

    public override void
    deactivateObjectAdapter(Test.RemoteObjectAdapterPrx adapter, Ice.Current current) => adapter.deactivate(); // Collocated call.

    public override void
    shutdown(Ice.Current current) => current.adapter.getCommunicator().shutdown();

    private int _nextPort = 10;
}
