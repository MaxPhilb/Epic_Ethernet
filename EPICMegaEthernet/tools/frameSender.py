import sys
import time
import socket

sys.path.append('./protopy')    

import epicethernetoutput_pb2



listout=epicethernetoutput_pb2.EpicEthernetOutput()
listout.nbChannel=32
boolstate=True

for i in range(5):
    output=listout.digoutputs.add()
    output.numChannel=i
    output.value=boolstate
    boolstate=not boolstate
    


print(listout)

ip="192.168.2.160"
#ip="127.0.0.1"
sock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
sock.connect((ip,4200))

message=listout.SerializeToString()
#+"\n".encode('ascii')
sock.send(message)
time.sleep(10)
for i in range(5):
    output=listout.digoutputs.add()
    output.numChannel=i
    output.value=boolstate
    boolstate=not boolstate

message=listout.SerializeToString()
#+"\n".encode('ascii')
sock.send(message)

sock.close()