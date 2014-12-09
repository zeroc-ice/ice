
lines = [line.strip() for line in open('c:/users/matthew/log.txt')]

def channel(l):
	i = l.find('java.nio.SocketChannelImpl')
	if i != -1:
		j = l.find(':', i)
		return l[i:j]
	return ""

w = {}
r = {}
requests = {}
state = 0
for l in lines:
	if 'read:' in l:
		i = l.find('read:')
		j = l.find('bytes', i)
		bytes = int(l[i + len('read:'):j])
		c = channel(l)
		if c in r:
			r[c] += bytes
		else:
			r[c] = bytes
	if 'write:' in l:
		i = l.find('write:')
		j = l.find('bytes', i)
		bytes = int(l[i + len('write:'):j])
		c = channel(l)
		if c in w:
			w[c] += bytes
		else:
			w[c] = bytes
	if state == 0:
		if 'sending asynchronous request' in l:
			state = 1
		elif 'received request' in l:
			state = 2
		elif 'sending reply' in l:
			state = 3
		elif 'received reply' in l:
			state = 4
	elif state != 0:
		i = l.find('request id = ')
		j = l.find('oneway')
		if i != -1 and j == -1:
			id = int(l[i + len('request id = '):])
			if state == 1:
				requests[id] = "sending"
			elif state == 2:
				if not id in requests:
					print("2: " + str(id) + " not in requests")
				requests[id] = "received"
			elif state == 3:
				if not id in requests:
					print("3: " + str(id) + " not in requests")
				requests[id] = "replied"
			elif state == 4:
				if not id in requests:
					print("4: " + str(id) + " not in requests")
				else:
					del requests[id]
			state = 0
				
print(requests)
print(w)
print(r)