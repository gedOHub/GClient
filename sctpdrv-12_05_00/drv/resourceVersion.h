#define STRINGER(w, x, y, z) #w "." #x "." #y "." #z
#define XSTRINGER(w, x, y, z) STRINGER(w, x, y, z)
#include "version.h"

#define VER_PRODUCTVERSION VER_MAJOR,VER_MINOR,VER_REV,VER_PRODUCTBUILD
#define VER_PRODUCTVERSION_STR XSTRINGER(VER_MAJOR,VER_MINOR,VER_REV,VER_PRODUCTBUILD)
#define VER_PRODUCTVERSION_NUMBER (\
    ((VER_MAJOR & 0xf) << 27) + \
    ((VER_MINOR & 0xf) << 23) + \
    ((VER_REV & 0xff) << 15) + \
    (VER_PRODUCTBUILD & 0xffff))

/*
    Other stuff the template didn't think was important
*/

#define VER_LEGALCOPYRIGHT_YEARS    "2010-2013"
#define VER_LEGALCOPYRIGHT_STR "(C) " VER_LEGALCOPYRIGHT_YEARS
