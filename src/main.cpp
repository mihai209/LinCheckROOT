#include "gui_main.h"
#include <iostream>
#include <cstdlib>
#include <glib.h>
#include <cstring>

// Entry point for LinCheckROOT
int main(int argc, char* argv[])
{
    // Suppress non-critical GTK warnings (theme, modules, settings)
    // These are cosmetic warnings that don't affect functionality
    g_setenv("G_DEBUG", "", TRUE);
    
    // Suppress GTK theme warnings by using XDG_DATA_DIRS properly
    const char* xdg_data = g_getenv("XDG_DATA_DIRS");
    if (!xdg_data || !strstr(xdg_data, "/usr/share")) {
        g_setenv("XDG_DATA_DIRS", "/usr/local/share:/usr/share", FALSE);
    }

    // Run GUI
    int result = GUI::init(argc, argv);
    return result;
}
