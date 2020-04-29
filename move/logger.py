import matplotlib.pyplot as plt
import numpy as np
import serial
import time

Fs = 100.0;  # sampling rate
Ts = 10.0/Fs; # sampling interval
t = np.arange(0,10,Ts) # time vector; create Fs samples between 0 and 1.0 sec.
x = np.arange(0,10,Ts) # time vector; create Fs samples between 0 and 1.0 sec.
y = np.arange(0,10,Ts) # signal vector; create Fs samples
z = np.arange(0,10,Ts) # signal vector; create Fs samples
tilt = np.arange(0,10,Ts)
#n = len(y) # length of the signal
#k = np.arange(n)
#T = n/Fs
#frq = k/T # a vector of frequencies; two sides frequency range
#frq = frq[range(int(n/2))] # one side frequency range

serdev = '/dev/ttyACM0'
s = serial.Serial(serdev)

for i in range(0, int(Fs)):
    line=s.readline() # Read an echo string from K66F terminated with '\n'
    # print line
    linex =  line[0:6]
    x[i] = float(linex)

    liney =  line[8:15]
    y[i] = float(liney)

    linez =  line[17:24]
    z[i] = float(linez)
    if (x[i] > 0.5 or x[i] < -0.5) and z[i] < 0.8:
        tilt[i] = 1
    elif z[i] < 0.5 and (y[i] > 0.5 or y[i] < -0.5):
        tilt[i] = 1
    else:
        tilt[i] = 0

#Y = np.fft.fft(y)/n*2 # fft computing and normalization
#Y = Y[range(int(n/2))] # remove the conjugate frequency parts

fig, ax = plt.subplots(2, 1)
ax[0].plot(t,x,label="$x$",color="blue")
ax[0].plot(t,y,label="$y$",color="red")
ax[0].plot(t,z,label="$z$",color="green")
ax[0].set_xlabel('Time')
ax[0].set_ylabel('Acc Vector')
ax[0].legend()
#ax[1].plot(frq,abs(Y),'r') # plotting the spectrum
#ax[1].set_xlabel('Freq (Hz)')
#ax[1].set_ylabel('|Y(freq)|')
ax[1].scatter(t,tilt)
ax[1].set_xlabel('Time')
ax[1].set_ylabel('Tilt')
plt.show()
s.close()