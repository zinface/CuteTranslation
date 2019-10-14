#include "xdotool.h"
#include <stdio.h>


#define MAXSTR 1000

Xdotool xdotool;

Xdotool::Xdotool()
{
    this->display = XOpenDisplay(nullptr);
    if (this->display == nullptr)
    {
        fprintf (stderr, "xdotool: fail to open display\n");
    }
    this->screen = XDefaultScreen(display);
    this->root_window = RootWindow(display, screen);



}

Xdotool::~Xdotool()
{
    XCloseDisplay(display);
}


int Xdotool::getMousePosition (int &root_x, int &root_y)
{
    unsigned int mask;

    /*
    Bool XQueryPointer(display, w, root_return, child_return, root_x_return, root_y_return,
                         win_x_return, win_y_return, mask_return)
          Display *display;
          Window w;
          Window *root_return, *child_return;
          int *root_x_return, *root_y_return;
          int *win_x_return, *win_y_return;
          unsigned int *mask_return;
    */
    Window return_window;
    XQueryPointer(display, root_window, &return_window, &return_window, &root_x, &root_y, &root_x, &root_y, &mask);
    // XCloseDisplay(display);
    return 0;
}

void Xdotool::check_status(int status, Window window)
{
    if (status == BadWindow) {
        printf("window id # 0x%lx does not exists!", window);
    }

    if (status != Success) {
        printf("XGetWindowProperty failed!");
    }
}

unsigned char* Xdotool::get_string_property(const char* property_name, Window window)
{
    Atom actual_type, filter_atom;
    int actual_format, status;
    unsigned long nitems, bytes_after;

    filter_atom = XInternAtom(display, property_name, True);
    status = XGetWindowProperty(display, window, filter_atom, 0, MAXSTR, False, AnyPropertyType,
                                &actual_type, &actual_format, &nitems, &bytes_after, &prop);
    check_status(status, window);
    return prop;
}

unsigned long Xdotool::get_long_property(const char* property_name, Window window)
{
    get_string_property(property_name, window);
    unsigned long long_property = static_cast<unsigned long>(prop[0] + (prop[1]<<8) + (prop[2]<<16) + (prop[3]<<24));
    return long_property;
}

unsigned long Xdotool::getActiveWindowPID()
{
    unsigned long window;
    window = get_long_property("_NET_ACTIVE_WINDOW", root_window);
    return get_long_property(("_NET_WM_PID"), window);
}

char* Xdotool::getActiveWindowName()
{
    unsigned long window;
    window = get_long_property("_NET_ACTIVE_WINDOW", root_window);
    get_string_property("_NET_WM_NAME", window);
    return reinterpret_cast<char *>(prop) ;

}
