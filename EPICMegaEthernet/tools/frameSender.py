import sys
import socket

sys.path.append('./protopy')    

import epicethernetoutput_pb2



listout=epicethernetoutput_pb2.EpicEthernetOutput();
boolstate=False;

for i in range(16):
    output=listout.digoutputs.add()
    output.id=i
    output.value=boolstate
    boolstate=not boolstate
    


print(listout)

sock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
sock.connect(("192.168.2.160",4200))
message=listout.SerializeToString()+"\n".encode('ascii')
sock.send(message)
sock.close()