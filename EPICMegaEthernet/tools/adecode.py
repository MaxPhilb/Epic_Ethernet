import sys
import time
import socket
import traceback
from bitarray import bitarray








for i in range(5):
    pass
    ##boolstate=not boolstate
    


ip="192.168.2.160"
#ip="127.0.0.1"
sock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
sock.connect((ip,4200))

message="t"
#+"\n".encode('ascii')

sock.send(message.encode())

SizeOfMessage=24+32+6+4+4

while True:
    data = sock.recv(SizeOfMessage)
   
    if not data:
        pass
        #break
    #print (data)
    print ("size message ")
    #print (data)
    print (len(data))
    print (''.join('{:02x}'.format(x) for x in data))
    
   
    digitalInput=bitarray()
    for i in range(24):
        for j in range(8):
            
            #print ("num "+ str((8*i)+j) + " " + str((data[i]>>j) & 1))
            digitalInput.append((data[i]>>(7-j)) & 1)

    print (digitalInput)
    
    analogInput=[]
    index=0
    for i in range(24,24+32,2):
        
        high=data[i]
        low=data[i+1]
       
        val=(high<<8)+low
        print (str(index) + " " +str( val))
        analogInput.append(val)
        index+=1

    macAddress=[]
    for i in range(56,62,1):
        macAddress.append(data[i])
    
    print ("mac address " + str(macAddress))

    """
    print (''.join('{:02x}'.format(data[62]) ))
    print (''.join('{:02x}'.format(data[63])))
    print (''.join('{:02x}'.format(data[64])))
    print (''.join('{:02x}'.format(data[65])))
    """
    
    timestamp=(data[62]<<(8*3))+(data[63]<<(8*2))+(data[64]<<8)+data[65]
    
    print ("timestamp " + str(timestamp))

    name=""
    nameByte=[]
    for i in range(66,70,1):
        
        nameByte.append(data[i])
    
    print (nameByte)
    name=bytes(nameByte).decode('utf-8')
    print ("name " + name)

    print()
    print()

sock.close()