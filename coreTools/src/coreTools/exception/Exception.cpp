#include "coreTools/exception/Exception.h"

namespace gw2re
{
namespace coreTools
{
namespace exception
{

Exception::Exception(const std::string& iReason) :
    _reason(iReason)
{
}

Exception::~Exception()
{
}

const char* Exception::what() const
{
    return _reason.c_str();
}

}
}
}
