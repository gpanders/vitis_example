#include <stdlib.h>

#define PACKSIZE 16

typedef float data_t;

struct bus {
    data_t arr[PACKSIZE];
};

#ifdef __cplusplus
extern "C" {
#endif

void axpy(struct bus const *a, struct bus const *x, struct bus const *y, struct bus *out, size_t len);

#ifdef __cplusplus
}
#endif
