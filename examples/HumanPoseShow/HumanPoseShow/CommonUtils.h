#ifndef COMMON_UTILS_H_
#define COMMON_UTILS_H_


#define DELETE_OBJECT(obj) \
    if (obj) \
    { \
        delete obj; \
        obj = 0; \
    }

#define INVALID_VALUE -1


#endif // COMMON_UTILS_H_