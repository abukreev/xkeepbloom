#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define TITLE "Bloomberg"

char* getWindowTitle(Display* display,  Window* w)
{
    XTextProperty p;
    if (0 != XGetWMName(display, *w, &p)) {
        return p.value;
    } else {
//        fprintf(stderr, "XGetWMName returned an error\n");
        return NULL;
    }
}

void processWindow(Display* display,  Window* w) {

    fprintf(stderr, "Window found\n");
}

int ignoreError(Display *d, XErrorEvent *e) {

    fprintf(stderr, "Error ignored\n");
    return 0;
}

int main()
{
    Display *display;
    Window rootwin;

    display = XOpenDisplay(NULL);
    rootwin = DefaultRootWindow(display);
    XSelectInput(display, rootwin, StructureNotifyMask | SubstructureNotifyMask | PropertyChangeMask);

    XEvent event;

    XSetErrorHandler(ignoreError);

    while (1) {
        XNextEvent(display, &event);
        if (event.type == MapNotify) {
            XMapEvent *mapevent = (XMapEvent*) &event;
//            fprintf(stderr, "Mapped:");
//            fprintf(stderr, " 0x%08x", (int) mapevent->window);
//            fprintf(stderr, " \"%s\"", getWindowTitle(display, &mapevent->window));
//            fprintf(stderr, "\n");
            printf("\n");
        } else if (event.type == MapRequest) {
            XMapRequestEvent *mapreqevent = (XMapRequestEvent*) &event;
//            fprintf(stderr, "Map request:");
//            fprintf(stderr, " 0x%08x", (int) mapreqevent->window);
//            fprintf(stderr, " \"%s\"", getWindowTitle(display, &mapreqevent->window));
//            fprintf(stderr, "\n");
//            fprintf("\n");
        } else if (event.type == CreateNotify) {
            XCreateWindowEvent *createevent = (XCreateWindowEvent*) &event;
//            fprintf(stderr, "Created:");
//            fprintf(stderr, " 0x%08x", (int) createevent->window);
//            fprintf(stderr, " \"%s\"", getWindowTitle(display, &createevent->window));
//            fprintf(stderr, "\n");
            char* title = getWindowTitle(display, &createevent->window);
            if (NULL != title && 0 == strncmp(TITLE, title, MIN(strlen(TITLE), strlen(title)))) {
                processWindow(display, &createevent->window);
            }
        } else if (event.type == PropertyNotify) {
            XPropertyEvent *propevent = (XPropertyEvent*) &event;
//            fprintf(stderr, "Property:");
//            fprintf(stderr, " 0x%08x", (int) propevent->window);
//            fprintf(stderr, " \"%s\"", getWindowTitle(display, &propevent->window));
//            fprintf(stderr, "\n");
        }
    }

    return 0;
}
