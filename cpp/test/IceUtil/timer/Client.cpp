// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Timer.h>
#include <TestCommon.h>

#include <vector>

using namespace IceUtil;
using namespace std;

class TestTask : public IceUtil::TimerTask, IceUtil::Monitor<IceUtil::Mutex>
{
public:

    TestTask() : _count(0)
    {
    }

    TestTask(const IceUtil::Time& scheduledTime) : _scheduledTime(scheduledTime), _count(0)
    {
    }
    
    virtual void 
    run()
    {
        Lock sync(*this);
        ++_count;
        _run = IceUtil::Time::now();
        //cerr << "run: " << _scheduledTime.toMicroSeconds() << " " << _run.toMicroSeconds() << endl;
        notifyAll();
    }

    virtual bool
    operator<(const TestTask& r) const
    {
        return _scheduledTime < r._scheduledTime;
    }

    virtual bool
    hasRun() const
    {
        Lock sync(*this);
        return _run != IceUtil::Time();
    }

    int 
    getCount() const
    {
        Lock sync(*this);
        return _count;
    }
    
    virtual IceUtil::Time
    getRunTime() const
    {
        Lock sync(*this);
        return _run;
    }

    IceUtil::Time
    getScheduledTime() const
    {
        return _scheduledTime;
    }

    virtual void
    waitForRun()
    {
        Lock sync(*this);
        while(_run == IceUtil::Time())
        {
            if(!timedWait(IceUtil::Time::seconds(10)))
            {
                test(false); // Timeout.
            }
        }
    }

private:

    IceUtil::Time _run;
    IceUtil::Time _scheduledTime;
    int _count;
};
typedef IceUtil::Handle<TestTask> TestTaskPtr;

int main(int argc, char* argv[])
{
    cout << "testing timer... " << flush;
    {
        IceUtil::TimerPtr timer = new IceUtil::Timer();

        {
            TestTaskPtr task = new TestTask();
            timer->schedule(task, IceUtil::Time::now());
            task->waitForRun();
	    while(true)
	    {
		timer->schedule(task, IceUtil::Time::now());
		try
		{
		    timer->schedule(task, IceUtil::Time::now());
		    test(task->hasRun());
		}
		catch(const IceUtil::IllegalArgumentException&)
		{
		    break;
		}
	    }
            task->waitForRun();
        }

        {
            TestTaskPtr task = new TestTask();
            test(!timer->cancel(task));
            timer->schedule(task, IceUtil::Time::now() + IceUtil::Time::seconds(1));
            test(!task->hasRun() && timer->cancel(task) && !task->hasRun());
            test(!timer->cancel(task));
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1100));
            test(!task->hasRun());
        }

        {
            vector<TestTaskPtr> tasks;
            IceUtil::Time start = IceUtil::Time::now() + IceUtil::Time::milliSeconds(100);
            for(int i = 0; i < 100; ++i)
            {
                 tasks.push_back(new TestTask(start + IceUtil::Time::milliSeconds(i)));
            }

            random_shuffle(tasks.begin(), tasks.end());
            
	    vector<TestTaskPtr>::const_iterator p;
            for(p = tasks.begin(); p != tasks.end(); ++p)
            {
                timer->schedule(*p, (*p)->getScheduledTime());
            }

            for(p = tasks.begin(); p != tasks.end(); ++p)
            {
                (*p)->waitForRun();
            }            

            test(IceUtil::Time::now() - start > IceUtil::Time::milliSeconds(99));

            sort(tasks.begin(), tasks.end());
            for(p = tasks.begin(); p + 1 != tasks.end(); ++p)
            {
                if((*p)->getRunTime() > (*(p + 1))->getRunTime())
                {
                    test(false);
                }
            }
        }

        {
            TestTaskPtr task = new TestTask();
            timer->scheduleRepeated(task, IceUtil::Time::milliSeconds(20));
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
            test(task->hasRun());
	    test(task->getCount() > 1);
	    test(task->getCount() < 26);
            test(timer->cancel(task));
            int count = task->getCount();
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(100));
            test(count == task->getCount());
        }

        timer->destroy();
    }
    cout << "ok" << endl;
    return EXIT_SUCCESS;
}
