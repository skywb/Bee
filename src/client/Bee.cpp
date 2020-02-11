
#include "Bee.h"
#include "log/mlog.hpp"
 
BEE_LIBARY_API void SetMyLibraryLogCallback(MLogCallBack logCallback)
{
    mlog::SetMlogCallBack(logCallback);
}
