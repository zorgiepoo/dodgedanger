
#include <stdint.h>

typedef void* ZGThread;
typedef int (*ZGThreadFunction)(void *data);

typedef void* ZGMutex;

ZGThread ZGCreateThread(ZGThreadFunction function, const char *name, void *data);
void ZGWaitThread(ZGThread thread);

// ZGDestroyMutex() is not provided because we don't need to destroy them in our game
ZGMutex ZGCreateMutex(void);
void ZGLockMutex(ZGMutex mutex);
void ZGUnlockMutex(ZGMutex mutex);

void ZGDelay(uint32_t delayMilliseconds);
