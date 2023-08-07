import sys
import time
import socket
import traceback

sys.path.append('./protopy')    

import epicethernetoutput_pb2
import epicethernetinput_pb2


listout=epicethernetoutput_pb2.EpicEthernetOutput()
listout.nbChannel=32
boolstate=True

epicIn=epicethernetinput_pb2.EpicEthernetInput()



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
'''
time.sleep(10)
for i in range(5):
    output=listout.digoutputs.add()
    output.numChannel=i
    output.value=boolstate
    boolstate=not boolstate

message=listout.SerializeToString()
#+"\n".encode('ascii')
sock.send(message)
'''
while True:
    data = sock.recv(2048)
    if not data:
        break
    #print (data)
    print ("size message ")
    #print (data)
    print (len(data))
    #print (''.join('{:02x}'.format(x) for x in data))
    #print (epicIn.IsInitialized())
    try:
        epicIn.ParseFromString(data)
    except :
        print (traceback.print_exc())
    

    print (epicIn.DeviceName)
    print (epicIn.MacAddress)
    print (epicIn.timeStamp)

    
    print()
    print (" ANA INPUT: ")
    for listAna in epicIn.anainputs:
        print ("id "+ str(listAna.id) +" " + str(listAna.value))

    print()
    print (" DIG INPUT:")
    
    for listDig in epicIn.diginputs:
        print ("id "+ str(listDig.id)+" "+ str(listDig.value))

    print()
    print()

sock.close()