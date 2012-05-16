#include "gw2DatTools/c_api/compression_inflateBuffer.h"

#include <exception>

#include "gw2DatTools/compression/inflateBuffer.h"

#ifdef __cplusplus
extern "C" {
#endif 

GW2DATTOOLS_API uint8_t* GW2DATTOOLS_APIENTRY compression_inflateBuffer(uint32_t* iInputTab, const uint32_t iInputSize, uint32_t* ioOutputSize)
{
    if (ioOutputSize == nullptr)
    {
        printf("GW2DATTOOLS_C_API(compression_inflateBuffer): ioOutputSize is NULL.");
        return NULL;
    }
    
    try
    {
        return gw2dt::compression::inflateBuffer(iInputTab, iInputSize, *ioOutputSize);
    }
    catch(std::exception& iException)
    {
        printf("GW2DATTOOLS_C_API(compression_inflateBuffer): %s", iException.what());
        return NULL;
    }
}

#ifdef __cplusplus
}
#endif 
