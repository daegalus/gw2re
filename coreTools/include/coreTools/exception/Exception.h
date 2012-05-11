#ifndef GW2RE_CORETOOLS_EXCEPTION_EXCEPTION_H
#define GW2RE_CORETOOLS_EXCEPTION_EXCEPTION_H

#include <exception>
#include <string>

#include "coreTools/dllMacros.h"

namespace gw2re
{
namespace coreTools
{
namespace exception
{

class Exception: public std::exception
{
    public:
        Exception(const std::string& iReason);
        virtual ~Exception();
        virtual const char* what() const;
        
    private:
        std::string _reason;
};

}
}
}

#endif // GW2RE_CORETOOLS_EXCEPTION_EXCEPTION_H
