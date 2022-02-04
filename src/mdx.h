#ifndef EUCLID__MDX_H
#define EUCLID__MDX_H 1

#include "utils.h"

// IDS - Intermediate Data Structure
#define IDS_STRLS_CRTDIMS       ((void *) 0x00)
#define IDS_STRLS_CRTMBRS       ((void *) 0x01)
#define IDS_OBJLS_BIUCUBE       ((void *) 0x02)
#define IDS_CXOBJ_ISRTCUBEMEARS ((void *) 0x03)

typedef struct __vector_measures__ {
    ArrayList *ls_vector;
    ArrayList *ls_mears_vals;
} IDSVectorMears;

#endif
