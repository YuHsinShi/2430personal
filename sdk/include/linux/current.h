#ifndef CURRENT_H
#define CURRENT_H

#include <pthread.h>

#if defined(WIN32)
#define current     (pthread_self().p)
#else
#define current     (pthread_self())
#endif

#endif // CURRENT_H
