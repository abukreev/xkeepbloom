#include <args.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

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
                         int press, int keycode) {

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
    event.state       = 0;
    event.type = press ? KeyPress : KeyRelease;

    return event;
}

void processWindow(Display* display,  Window *root, Window* window) {

    Window winFocus;
    int    revert;
    XKeyEvent event;
    XGetInputFocus(display, &winFocus, &revert);
    LOG("Window focused");
    event = createKeyEvent(display, window, root, 1, KEYCODE);
    XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);
    LOG("KeyPress sent");
    event = createKeyEvent(display, window, root, 0, KEYCODE);
    XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);
    LOG("KeyRelease sent");
}

int ignoreError(Display *d, XErrorEvent *e) {

//    LOG("Error ignored");
    return 0;
}

time_t today_time() {

    time_t cur_time;
    struct tm*  midnight_time_tm;

    cur_time = time(NULL);
    midnight_time_tm = localtime(&cur_time);
    midnight_time_tm->tm_sec = 0;
    midnight_time_tm->tm_min = 0;
    midnight_time_tm->tm_hour = 0;

    return cur_time - mktime(midnight_time_tm);
}

void print_time(FILE* stream, time_t time_value) {

    char buf[6];
    struct tm* tm_info;

    tm_info = gmtime(&time_value);

    strftime(buf, sizeof(buf), "%H:%M", tm_info);
    fprintf(stream, "%s", buf);
}

int main(int argc, char* argv[])
{
    Display *display;
    Window root;

    time_t cur_time;
    int sleep_period = 0;
    struct arguments args = { 0 };

    if (parse_arguments(argc, argv, &args)) {
        return EXIT_FAILURE;
    }

    if (args.help) {
        return EXIT_SUCCESS;
    }

    display = XOpenDisplay(NULL);
    root = DefaultRootWindow(display);
    XSelectInput(display, root, StructureNotifyMask | SubstructureNotifyMask | PropertyChangeMask);

    XEvent event;

    XSetErrorHandler(ignoreError);

    while (1) {

        if (args.min_time_present || args.max_time_present) {
            cur_time = today_time();
            if (args.min_time_present && cur_time < args.min_time) {
                sleep_period = args.min_time - cur_time;
                printf("Sleeping for %d seconds\n", sleep_period);
                sleep(sleep_period);
                continue;
            }
            if (args.max_time_present && cur_time > args.max_time) {
                sleep_period = 86400 - cur_time + (args.min_time_present ? args.min_time : 0)
                printf("Sleeping for %d seconds\n", sleep_period);
                sleep(sleep_period);
                continue;
            }
        }

        XNextEvent(display, &event);
        if (event.type == CreateNotify) {
            XCreateWindowEvent *createevent = (XCreateWindowEvent*) &event;
            const char *p = getWindowTitle(display, &createevent->window);
            if (NULL != p) {
                if (strcmp(TITLE, p) == 0) {
                    LOG("Window found");
                    processWindow(display, &root, &createevent->window);
                }
            }
        }
    }

    return 0;
}
