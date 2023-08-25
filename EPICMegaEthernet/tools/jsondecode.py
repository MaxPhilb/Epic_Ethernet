import sys
import time
import socket
import traceback
from bitarray import bitarray
import json







for i in range(5):
    pass
    ##boolstate=not boolstate
    


ip="192.168.0.2"
#ip="127.0.0.1"
sock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
sock.connect((ip,4200))

message="t"
#+"\n".encode('ascii')

sock.send(message.encode())

SizeOfMessage=2048

while True:
    data = sock.recv(SizeOfMessage).decode('latin-1')
    if not data:
        break
    print (data)
    json_object=json.loads(data)
    print(json.dumps(json_object))

    print(json.dumps(json_object, indent=2))
    print ()
    print()
    print()

sock.close()