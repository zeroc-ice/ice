import sys, os, string

def start(path):
    pipein, pipeout = os.pipe()
    pid = os.fork()
    if(pid == 0):
        os.close(pipein)
        os.dup2(pipeout, sys.stdout.fileno())
        os.execv(path, [])
    else:
        os.close(pipeout)
        return os.fdopen(pipein), pid

pids = []
def terminate(status):
    for pid in pids:
        if(os.name == "nt"):
            import win32api
            handle = win32api.OpenProcess(1, 0, pid)
            return (0 != win32api.TerminateProcess(handle, 0))
        else:
            os.kill(pid, 9)
    sys.exit(status)

def server():
    print "starting server...",
    server, pid = start("./server")
    pids.append(pid)
    ready = string.strip(server.readline())
    if ready != "ready":
        print "failed!"
        terminate(0)
    else:
        print "ok"

def client():
    print "starting client...",
    client, pid = start("./client")
    pids.append(pid)
    output = client.read()
    if not output:
        print "failed!"
        terminate(0)
    else:
        print "ok"
    print output,

def collocated():
    print "starting collocated...",
    client, pid = start("./collocated")
    pids.append(pid)
    output = client.read()
    if not output:
        print "failed!"
        terminate(0)
    else:
        print "ok"
    print output,
