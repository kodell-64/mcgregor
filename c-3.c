#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT 6972

//
// 'client' listens on port 6972 to receive keyboard/mouse info from 'server' 
//

// for older compilers
#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif


int main()
{
    int sockfd, debug = 0;
    struct sockaddr_in local_addr, remote_addr;
    socklen_t addr_len;
    int n;
    Display *display;

    struct 
    {
        unsigned char protocol;
        unsigned short int keycode; 
        unsigned short int modifiers;
        unsigned short int x;
        unsigned short int y;
        unsigned short int button_press;
    } packet;

    if(debug)
    {
        printf("\n%d", sizeof(unsigned int));
        printf("\n%d", sizeof(unsigned short int));
    }

    /* Create UDP socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    /* Zero out structure */
    memset((char *)&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* Listen on all interfaces */
    local_addr.sin_port = htons(PORT);

    /* Bind socket to port */
    if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
    {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    printf("UDP client waiting for data on port %d...\n", PORT);
    display = XOpenDisplay(NULL);
    if (!display) {
        return 1;
    }

    while(1)
    {
        addr_len = sizeof(remote_addr);
        
        /* Receive from server */
        n = recvfrom(sockfd, &packet, sizeof(packet), 0,
                     (struct sockaddr *)&remote_addr, &addr_len);
        if (n < 0)
        {
            perror("recvfrom");
            break;
        }
        if(packet.protocol == 0) // keyboard
        {
            packet.keycode = XKeysymToKeycode(display, packet.keycode);
            printf("\nreceived keycode %d", packet.keycode); 
            if(packet.keycode ==0)continue;
            printf("\nreceived modifiers %d", packet.modifiers); 
            printf("\nreceived xy %d %d", packet.x, packet.y); 
            if (packet.modifiers & 1) XTestFakeKeyEvent(display, XKeysymToKeycode(display, 65505), True, 0);
            if (packet.modifiers & 4) XTestFakeKeyEvent(display, XKeysymToKeycode(display, 65507), True, 0);
            if (packet.modifiers & 8) XTestFakeKeyEvent(display, XKeysymToKeycode(display, 65513), True, 0);
            XTestFakeKeyEvent(display, packet.keycode, True, CurrentTime);
            XTestFakeKeyEvent(display, packet.keycode, False, CurrentTime);
            if (packet.modifiers & 1) XTestFakeKeyEvent(display, XKeysymToKeycode(display, 65505), False, 0);
            if (packet.modifiers & 4) XTestFakeKeyEvent(display, XKeysymToKeycode(display, 65507), False, 0);
            if (packet.modifiers & 8) XTestFakeKeyEvent(display, XKeysymToKeycode(display, 65513), False, 0);
        }
        else if(packet.protocol==1) // mouse x-y
        {
            printf("\n xy %d %d", packet.x, packet.y);
            XTestFakeMotionEvent(display, -1, packet.x, packet.y, CurrentTime);
        }
        else if(packet.protocol==2) // mouse buttons
        {
            XTestFakeButtonEvent(display, 1, True, CurrentTime);  // press
            XTestFakeButtonEvent(display, 1, False, CurrentTime); // release
        }

        XFlush(display);
    }

    XCloseDisplay(display);
    return 0;
}

