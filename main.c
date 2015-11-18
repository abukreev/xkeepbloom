#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define TITLE "Bloomberg - Session Timeout Warning"
#define KEYCODE XK_Y

void LOG(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fprintf(stderr, "%d-%d-%d %d:%d:%d ", tm.tm_year + 1900, tm.tm_mon + 1,
            tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fprintf(stderr, format, args);
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

   if (press)
      event.type = KeyPress;
   else
      event.type = KeyRelease;

   return event;
}

int findWindow(Display *display, const Window *current, Window *found) {

    Window root;
    Window parent;
    Window *children = NULL;
    unsigned int nchildren;
    int i;
    int res = 1;

    LOG("Find window");

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

int ignoreError(Display *d, XErrorEvent *e) {

//    LOG("Error ignored");
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
        if (event.type == CreateNotify) {
            XCreateWindowEvent *createevent = (XCreateWindowEvent*) &event;
//            LOG("Created: 0x%08x", (int) createevent->window);
            const char *p = getWindowTitle(display, &createevent->window);
//            fprintf(stderr, " \"%s\"", p);
//            fprintf(stderr, "\n");
            if (NULL != p) {
//                fprintf(stderr, " strcmp(\"%s\", \"%s\") = %d\n", TITLE, p, strcmp(TITLE, p));
                if (strcmp(TITLE, p) == 0) {
                    LOG("Window found");
                    processWindow(display, &root, &createevent->window);
                }
            }
        }
    }

    return 0;
}
