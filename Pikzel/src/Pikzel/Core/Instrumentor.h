#pragma once

#include "Tracy.hpp"


#ifdef PKZL_PROFILE
#define PKZL_PROFILE_BEGIN_SESSION(name, filepath)
#define PKZL_PROFILE_END_SESSION()
#define PKZL_PROFILE_SCOPE(name) ZoneScopedN(name)
#define PKZL_PROFILE_FUNCTION() ZoneScoped
#define PKZL_PROFILE_FRAMEMARKER() FrameMark
#define PKZL_PROFILE_SETVALUE(v) ZoneValue(v)
#else
#define PKZL_PROFILE_BEGIN_SESSION(name, filepath)
#define PKZL_PROFILE_END_SESSION()
#define PKZL_PROFILE_SCOPE(name)
#define PKZL_PROFILE_FUNCTION()
#define PKZL_PROFILE_FRAMEMARKER()
#define PKZL_PROFILE_SETVALUE(v)
#endif
