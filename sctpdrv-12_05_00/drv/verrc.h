//
// this defines all the missing constants
//
#if DBG
#define VER_BUILD_STRING "checked build"
#else
#define VER_BUILD_STRING ""
#endif

#ifdef VER_BETA
#define  VER_FILEDESCRIPTION_STR     VER_INTERNAL_FILEDESCRIPTION_STR " beta " VER_BUILD_STRING
#else
#define  VER_FILEDESCRIPTION_STR     VER_INTERNAL_FILEDESCRIPTION_STR " " VER_BUILD_STRING
#endif

#ifndef NT_INCLUDED

#include <winver.h>

//
// from ntverp.h
//

/* default is nodebug */
#if DBG
#define VER_DEBUG                   VS_FF_DEBUG
#else
#define VER_DEBUG                   0
#endif

/* default is prerelease */
#if ALPHA
#define VER_PRERELEASE              VS_FF_PRERELEASE
#elif BETA
#define VER_PRERELEASE              VS_FF_PRERELEASE
#else
#define VER_PRERELEASE              0
#endif



#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK
#define VER_FILEOS                  VOS_NT_WINDOWS32
#define VER_FILEFLAGS               (VER_PRERELEASE|VER_DEBUG)

#define VER_COMPANYNAME_STR         "Bruce Cran"

/*-----------------------------------------------*/
/* the following lines are specific to this file */
/*-----------------------------------------------*/



/* VER_FILETYPE, VER_FILESUBTYPE, VER_FILEDESCRIPTION_STR
 * and VER_INTERNALNAME_STR must be defined before including
COMMON.VER
 * The strings don't need a '\0', since common.ver has them.
 */
#ifndef VER_FILETYPE
#define       VER_FILETYPE    VFT_DRV
#endif
/* possible values:           VFT_UNKNOWN
                              VFT_APP
                              VFT_DLL
                              VFT_DRV
                              VFT_FONT
                              VFT_VXD
                              VFT_STATIC_LIB
*/
#ifndef VER_FILESUBTYPE
#define       VER_FILESUBTYPE VFT2_DRV_INSTALLABLE
#endif
/* possible values            VFT2_UNKNOWN
                              VFT2_DRV_PRINTER
                              VFT2_DRV_KEYBOARD
                              VFT2_DRV_LANGUAGE
                              VFT2_DRV_DISPLAY
                              VFT2_DRV_MOUSE
                              VFT2_DRV_NETWORK
                              VFT2_DRV_SYSTEM
                              VFT2_DRV_INSTALLABLE
                              VFT2_DRV_SOUND
                              VFT2_DRV_COMM
*/


#if DBG

#ifdef VER_BETA
#define VER_FILEVERSION_STR VER_PRODUCTVERSION_STR " (beta checked build)"
#else
#define VER_FILEVERSION_STR VER_PRODUCTVERSION_STR " (checked build)"
#endif

#else

#ifdef VER_BETA
#define VER_FILEVERSION_STR VER_PRODUCTVERSION_STR " (beta)"
#else
#define VER_FILEVERSION_STR VER_PRODUCTVERSION_STR
#endif

#endif


#include "common.ver"
#endif
