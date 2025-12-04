#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#define SERVER_IP "192.168.1.76"
#define SERVER_PORT 6972

int main() {

    struct KeyMessage {
         uint32_t keysym; // X11 keysym
         uint16_t state;  // modifier masks (Shift, Ctrl, etc)
    };
    
    Display *disp = XOpenDisplay(NULL);
    if (!disp) {
        fprintf(stderr, "Cannot open display!\n");
        return 1;
    }

    int screen = DefaultScreen(disp);
    Window root = RootWindow(disp, screen);

    // Create a simple window
    Window win = XCreateSimpleWindow(disp, root, 100, 100, 400, 300, 1,
                                     BlackPixel(disp, screen), WhitePixel(disp, screen));

    XStoreName(disp, win, "Input Capture Window");

    // Select input events
    XSelectInput(disp, win,
                 KeyPressMask |
                 KeyReleaseMask |
                 ButtonPressMask |
                 ButtonReleaseMask |
                 PointerMotionMask |
                 StructureNotifyMask);

    XMapWindow(disp, win);

    XEvent ev;
    printf("Event loop started...\n");

    int sockfd;
    struct sockaddr_in server_addr;
    char msg[32];
    char buffer[1024];
    socklen_t addr_len = sizeof(server_addr);

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    
    printf("\n%d", ShiftMask);
    printf("\n%d", ControlMask);
    printf("\n%d", Mod1Mask);
    printf("\nxkl %d", XK_Shift_L);
    printf("\nxkl %d", XK_Control_L);
    printf("\nxkl %d", XK_Alt_L);

    struct
    {
         unsigned char protocol;
         unsigned short int keycode;
         unsigned short int modifiers;
         unsigned int short x;
         unsigned int short y;
         unsigned int short button_press;
    }packet;
    
    while (1) {
        XNextEvent(disp, &ev);

        if (ev.type == KeyPress) {
            KeySym key = XLookupKeysym(&ev.xkey, 0);
            char *ksname = XKeysymToString(key);
            printf("Key Press: keycode=%d (%s)\n",
                   ev.xkey.keycode,
                   ksname ? ksname : "unknown");
        }
        if (ev.type == KeyPress)
             //if (ev.type == KeyRelease || ev.type == KeyPress)
        {
            KeySym key = XLookupKeysym(&ev.xkey, 0);
            char *ksname = XKeysymToString(key);
            printf("Key Release: keycode=%d (%s)\n",
                   ev.xkey.keycode,
                   ksname ? ksname : "unknown");
            sprintf(msg, "%d", ev.xkey.keycode);
            printf("\n sending %d", ev.xkey.keycode);
            /*
            KeySym keysym = XLookupKeysym(&ev.xkey, 0);
            unsigned int keycode = ev.xkey.keycode;
            unsigned int modifiers = ev.xkey.state;
            unsigned char pressed = (ev.type == KeyPress) ? 1 : 0;
            unsigned char msg[3] = { pressed, keycode, modifiers };
            */
            
            //struct KeyMessage msg;
            //KeySym ks = XLookupKeysym(&ev.xkey, 0);

            packet.protocol = 0;
            packet.keycode = XLookupKeysym(&ev.xkey, 0);
            packet.modifiers = ev.xkey.state;
            //printf("\nsizeof keycode %d keycode %d", sizeof(keycode), keycode);
            //printf("\nsizeof modifiers %d modifiers %d", sizeof(modifiers), modifiers);

            //msg.keysym = ks;
            //msg.state = ev.xkey.state;

            sendto(sockfd, &packet, 13, 0, (struct sockaddr*)&server_addr, addr_len);
            //sendto(sockfd, &modifiers, 2, 0, (struct sockaddr*)&server_addr, addr_len);
            
            //sendto(sockfd, &keycode, sizeof(keycode), 0,
            //   (struct sockaddr *)&server_addr, addr_len);
        }
        if (ev.type == ButtonPress) {
            printf("Mouse Button Press: button=%d\n", ev.xbutton.button);
        }
        if (ev.type == ButtonRelease) {
            printf("Mouse Button Release: button=%d\n", ev.xbutton.button);
            packet.protocol = 2;
            packet.button_press=1;
            sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&server_addr, addr_len);
        }
        if (ev.type == MotionNotify) {
            printf("Mouse Move: x=%d y=%d\n",
                   ev.xmotion.x, ev.xmotion.y);

            packet.protocol = 1;
            packet.keycode = 0;
            packet.modifiers = 0;
            packet.x = ev.xmotion.x;
            packet.y = ev.xmotion.y;

            printf("\nKO %d %d", packet.x, packet.y);
            sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&server_addr, addr_len);
        }
     
        
    }

    XCloseDisplay(disp);
    return 0;
}
