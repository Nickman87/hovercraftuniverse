// Thin forwarding header -- matches the real SkyX 0.1 include path
// (HovercraftUniverse/dependencies/SkyX/include/SkyX.h is included as
// <SkyX.h>, flat, no subdirectory) so InGameState.cpp's #include <SkyX.h>
// resolves unchanged. See skyx_compat/SkyXAll.h for the actual shim.
#ifndef HU_SKYX_COMPAT_FORWARD_SKYX_H
#define HU_SKYX_COMPAT_FORWARD_SKYX_H
#include "skyx_compat/SkyXAll.h"
#endif
