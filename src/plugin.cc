#include "plugin.h"
#include "application.h"

experience::Application app;


bool Start() {
    app.init_log();
    app.start();
    return true;
}


bool Stop() {
    app.stop();
    return true;
}



const char* Info() {
    return "This is DSService daemon";
}