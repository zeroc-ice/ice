#include <ping.hh>

#include <iostream>
#include <fstream>

using namespace std;
using namespace Test;

class Throughput_i : public POA_Test::Throughput,
		     public PortableServer::RefCountServantBase
{
public:
    Throughput_i(CORBA::ORB_ptr);
    virtual ~Throughput_i() { }
    
    virtual void sendByteSeq(const ByteSeq&);
    virtual void sendStringSeq(const StringSeq&);
    virtual void sendStringDoubleSeq(const StringDoubleSeq&);

    virtual void ping();
    virtual void ping_oneway();

    virtual void shutdown();

private:
    CORBA::ORB_var _orb;
};

Throughput_i::Throughput_i(CORBA::ORB_ptr orb) : 
    _orb(CORBA::ORB::_duplicate(orb))
{
}

void
Throughput_i::sendByteSeq(const ByteSeq&)
{
}

void
Throughput_i::sendStringSeq(const StringSeq&)
{
}

void
Throughput_i::sendStringDoubleSeq(const StringDoubleSeq&)
{
}

void
Throughput_i::ping()
{
}

void
Throughput_i::ping_oneway()
{
}

void
Throughput_i::shutdown()
{
    _orb->shutdown(0);
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    try 
    {
	CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);
	
	CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
	PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

	Throughput_i* myservant = new Throughput_i(orb);
	PortableServer::ObjectId_var myservantid = poa->activate_object(myservant);
	obj = myservant->_this();
	    
	// Obtain a reference to the object, and print it out as a
	// stringified IOR.
	CORBA::String_var sior(orb->object_to_string(obj));
	ofstream os("test.ior");
	os << sior.in() << endl;
	os.close();

	//myecho->_remove_ref();
	
	PortableServer::POAManager_var pman = poa->the_POAManager();
	pman->activate();
	cout << "Adapter ready" << endl;
	
	orb->run();
    }
    catch(CORBA::SystemException&) 
    {
	cerr << "Caught CORBA::SystemException." << endl;
    }
    catch(CORBA::Exception&) 
    {
	cerr << "Caught CORBA::Exception." << endl;
    }
    catch(omniORB::fatalException& fe) 
    {
	cerr << "Caught omniORB::fatalException:" << endl;
	cerr << "  file: " << fe.file() << endl;
	cerr << "  line: " << fe.line() << endl;
	cerr << "  mesg: " << fe.errmsg() << endl;
    }
    catch(...) 
    {
	cerr << "Caught unknown exception." << endl;
    }
    
    return 0;
}
