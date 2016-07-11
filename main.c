#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define TITLE "Bloomberg - Session Timeout Warning"
#define KEYCODE XK_Y
#define TIMEOUT_USEC 500000
#define NUM_ATTEMPTS 3

void LOG(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fprintf(stderr, "%d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900,
            tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);
}

char* getWindowTitle(Display* display,  Window* w)
{
    XTextProperty p;
    if (0 != XGetWMName(display, *w, &p)) {
        return (char*) p.value;
    } else {
        return NULL;
    }
}

// Function to create a keyboard event
XKeyEvent createKeyEvent(Display *display, Window *window, Window *root,
                         int press, int keycode, int modifiers) {

    XKeyEvent event;

    event.display     = display;
    event.window      = *window;
    event.root        = *root;
    event.subwindow   = None;
    event.time        = CurrentTime;
    event.x           = 1;
    event.y           = 1;
    event.x_root      = 1;
    event.y_root      = 1;
    event.same_screen = True;
    event.keycode     = XKeysymToKeycode(display, keycode);
    event.state       = modifiers;
    event.type = press ? KeyPress : KeyRelease;

    return event;
}

void sendKeyToWindow(Display* display,  Window *root, Window* window) {

    Window winFocus;
    int    revert;
    XKeyEvent event;
    XGetInputFocus(display, &winFocus, &revert);
    LOG("Window focused");
    event = createKeyEvent(display, window, root, 1, KEYCODE, 0);
    XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);
    LOG("KeyPress sent");
    event = createKeyEvent(display, window, root, 0, KEYCODE, 0);
    XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);
    LOG("KeyRelease sent");
}

void processWindow(Display* display,  Window *root, Window* window) {
    int i;
    Status s;
    XWindowAttributes wa;

    for (i = 0; i < NUM_ATTEMPTS; i++) {
        LOG("Attempt %d", i + 1);

        s = XGetWindowAttributes(display, *window, &wa);
        if (0 == s) {
            LOG("Window DOESN'T exist");
            break;
        }

        LOG("Window exists. Trying to send an event to it");
        sendKeyToWindow(display, root, window);
        usleep(TIMEOUT_USEC);
    }
}

int ignoreError(Display *d, XErrorEvent *e) {

//    LOG("Error ignored");
    return 0;
}

int processEvent(Display *display, Window root, XEvent *event) {

    if (event->type == CreateNotify) {
        XCreateWindowEvent *createevent = (XCreateWindowEvent*) event;
        const char *p = getWindowTitle(display, &createevent->window);
        if (NULL != p) {
            if (strcmp(TITLE, p) == 0) {
                LOG("Window found");
                processWindow(display, &root, &createevent->window);
            }
        }
    }
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
        processEvent(display, root, &event);
    }

    return 0;
}
