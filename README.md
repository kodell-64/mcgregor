# MCGREGOR

### synergy-like keyboard/mouse sharing program for two Linux machines

I have some nostalgic machines, 486s with X11 running, and needed a way to control their mouse/keyboard from my workstation.

Compile the client for the older machine:

```
gcc c-3.c -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -lXtst -o client
```

Compile the server for the machine with mouse/keyboard

```
gcc -g s-2.c -lX11 -lXtst -o server
```


#### REVISION HISTORY
* Dec 4/25 initial checkin