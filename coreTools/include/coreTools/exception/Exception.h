#ifndef GW2RE_CORETOOLS_EXCEPTION_EXCEPTION_H
#define GW2RE_CORETOOLS_EXCEPTION_EXCEPTION_H

#include <exception>

#include "coreTools/dllMacros.h"

namespace gw2re
{
namespace coreTools
{
namespace exception
{

class GW2RE_CORETOOLS_API Exception: public std::exception
{
    public:
        Exception(const char* iReason);
        virtual ~Exception();
        virtual const char* what() const;
        
    private:
        const char* _reason;
};

}
}
}

#endif // GW2RE_CORETOOLS_EXCEPTION_EXCEPTION_H
