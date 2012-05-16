#ifndef GW2DATTOOLS_EXCEPTION_EXCEPTION_H
#define GW2DATTOOLS_EXCEPTION_EXCEPTION_H

#include <exception>

namespace gw2dt
{
namespace exception
{

class Exception: public std::exception
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

#endif // GW2DATTOOLS_EXCEPTION_EXCEPTION_H
