// Copyright (c) ZeroC, Inc.

#include "Controller.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

#include <stdexcept>

using namespace std;
using namespace Ice;
using namespace Test;

class Control final : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Control::run(int argc, char** argv)
{
    Ice::CommunicatorHolder ich = initialize(argc, argv);
    if (argc < 2)
    {
        ostringstream os;
        os << "Usage: " << argv[0] << " proxy";
        throw invalid_argument(os.str());
    }

    ControllerPrx control(ich.communicator(), argv[1]);
    control->stop();
}

DEFINE_TEST(Control)
