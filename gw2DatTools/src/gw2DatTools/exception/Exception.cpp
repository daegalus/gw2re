#include "gw2DatTools/exception/Exception.h"

namespace gw2dt
{
namespace exception
{

Exception::Exception(const char* iReason) :
    _reason(iReason)
{
}

Exception::~Exception()
{
}

const char* Exception::what() const
{
    return _reason;
}

}
}
