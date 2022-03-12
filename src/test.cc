
#include "thread.h"
#include <functional>
namespace experience {

void test() {
    
}

int main () {
    Thread<std::function<void()>> th("test", test);
    return 1;
}


}