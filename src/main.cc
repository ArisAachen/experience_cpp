#include "log.h"
#include "singleton.h"

int main() {
    // init default log
    experience::SingletonPtr<experience::Logger>::get_instance()->init_default();
    
    return 1;
}