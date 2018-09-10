#ifndef FAS_SIGIGNORE
#define FAS_SIGIGNORE
#include <sys/signal.h>

namespace moxie {

class SigIgnore {
public:
    SigIgnore() {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

}
#endif // FAS_SIGIGNORE

