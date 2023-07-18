import sys

sys.path.append('./protopy')    

import epicethernetoutput_pb2



listout=[];
boolstate=False;

for i in range(16):
    frameforoutput= epicethernetoutput_pb2.DigitalOutput()
    frameforoutput.id=i
    frameforoutput.value=boolstate
    boolstate=not boolstate
    listout.append(frameforoutput)


print(listout)

SerializeToString