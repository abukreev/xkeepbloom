#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define TITLE "Bloomberg - Session Timeout Warning"
//#define X_OFFSET 285
//#define Y_OFFSET 128
#define X_OFFSET 200 
#define Y_OFFSET 59

char* getWindowTitle(Display* display,  Window* w)
{
    XTextProperty p;
    if (0 != XGetWMName(display, *w, &p)) {
        return (char*) p.value;
    } else {
//        fprintf(stderr, "XGetWMName returned an error\n");
        return NULL;
    }
}

void mouseClick(Display *display, Window *root, Window *w, int x, int y) {

    XEvent event;
    int rc;

    memset(&event, 0x00, sizeof(event));
	
    event.type = ButtonPress;
    event.xbutton.button = Button1;
    event.xbutton.same_screen = True;
    event.xbutton.x = 1 + X_OFFSET;
    event.xbutton.y = 28 + Y_OFFSET;

//    rc = XWarpPointer(display, *w, *root, 0, 0, 0, 0, x, y);
//    if (0 == rc) {
//        fprintf(stderr, "XWarpPointer returned %d\n", rc);
//        return;
//    }

//    if (False == XQueryPointer(display, 
//                               RootWindow(display, DefaultScreen(display)),
//                              &event.xbutton.root, &event.xbutton.window,
//                              &event.xbutton.x_root, &event.xbutton.y_root,
//                              &event.xbutton.x, &event.xbutton.y,
//                              &event.xbutton.state)) {
//        fprintf(stderr, "XQueryPointer returned False\n");
//    }
//	
//    event.xbutton.subwindow = event.xbutton.window;
//	
//    while (event.xbutton.subwindow) {
//        event.xbutton.window = event.xbutton.subwindow;
//	XQueryPointer(display, event.xbutton.window, &event.xbutton.root,
//                     &event.xbutton.subwindow, &event.xbutton.x_root,
//                     &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y,
//                     &event.xbutton.state);
//    }

    rc = XSendEvent(display, *w, True, 0xfff, &event);
    if (0 == rc) {
        fprintf(stderr, "XSendEvent returned %d\n", rc);
        return;
    }
    fprintf(stderr, "KeyPress sent\n");
    
    XFlush(display);
    
    usleep(100000);
    
    event.type = ButtonRelease;
    event.xbutton.state = 0x100;

    rc = XSendEvent(display, *w, True, 0xfff, &event);
    if (0 == rc) {
        fprintf(stderr, "XSendEvent returned %d\n", rc);
        return;
    }
    fprintf(stderr, "KeyRelease sent\n");
    
    XFlush(display);
}

int findWindow(Display *display, const Window *current, Window *found) {

    Window root;
    Window parent;
    Window *children = NULL;
    unsigned int nchildren;
    int i;
    int res = 1;

    fprintf(stderr, "Find window\n");

    if (0 == XQueryTree(display, *current, &root, &parent, &children,
                       &nchildren)) {
        return 1;
    }

    for (i = 0; i < nchildren; ++i) {
        if (0 == findWindow(display, &children[i], found)) {
            res = 0;
            break;
        }
    }

    XFree(children);

    return res;
}

void processWindow(Display* display,  Window *root, Window* window) {

//    Window *button = NULL;
    XWindowAttributes wa;
    int rc;

    rc = XGetWindowAttributes(display, *window, &wa);
    if (0 == rc) {
        fprintf(stderr, "XGetWindowAttributes returned %d\n", rc);
        return;
    }
    fprintf(stderr, "Size: %d, %d, %d, %d\n",
            wa.x, wa.y, wa.width, wa.height);

    usleep(500000);

    mouseClick(display, root, window, X_OFFSET, Y_OFFSET);

//    if (0 == findWindow(display, window, button)) {
//        fprintf(stderr, "Found the button\n");
//    } else {
//        fprintf(stderr, "NOT found the button\n");
//    }
}

int ignoreError(Display *d, XErrorEvent *e) {

//    fprintf(stderr, "Error ignored\n");
    return 0;
}

int main()
{
    Display *display;
    Window root;

    display = XOpenDisplay(NULL);
    root = DefaultRootWindow(display);
    XSelectInput(display, root, StructureNotifyMask | SubstructureNotifyMask | PropertyChangeMask);

    XEvent event;

    XSetErrorHandler(ignoreError);

    while (1) {
        XNextEvent(display, &event);
        if /*(event.type == MapNotify) {
            XMapEvent *mapevent = (XMapEvent*) &event;
            fprintf(stderr, "Mapped:");
            fprintf(stderr, " 0x%08x", (int) mapevent->window);
            fprintf(stderr, " \"%s\"", getWindowTitle(display, &mapevent->window));
            fprintf(stderr, "\n");
        } else if (event.type == MapRequest) {
            XMapRequestEvent *mapreqevent = (XMapRequestEvent*) &event;
            fprintf(stderr, "Map request:");
            fprintf(stderr, " 0x%08x", (int) mapreqevent->window);
            fprintf(stderr, " \"%s\"", getWindowTitle(display, &mapreqevent->window));
            fprintf(stderr, "\n");
        } else if*/ (event.type == CreateNotify) {
            XCreateWindowEvent *createevent = (XCreateWindowEvent*) &event;
            fprintf(stderr, "Created:");
            fprintf(stderr, " 0x%08x", (int) createevent->window);
            const char *p = getWindowTitle(display, &createevent->window);
            fprintf(stderr, " \"%s\"", p);
            fprintf(stderr, "\n");
            if (NULL != p && strcmp(TITLE, p) == 0) {
                fprintf(stderr, "Window found\n");
                processWindow(display, &root, &createevent->window);
            }
//            char* title = getWindowTitle(display, &createevent->window);
//            if (NULL != title && 0 == strncmp(TITLE, title, MIN(strlen(TITLE), strlen(title)))) {
//                processWindow(display, &root, &createevent->window);
//            }
//        } else if (event.type == PropertyNotify) {
//            XPropertyEvent *propevent = (XPropertyEvent*) &event;
//            fprintf(stderr, "Property:");
//            fprintf(stderr, " 0x%08x", (int) propevent->window);
//            fprintf(stderr, " \"%s\"", getWindowTitle(display, &propevent->window));
//            fprintf(stderr, "\n");
        }
    }

    return 0;
}
