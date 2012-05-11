#include "coreTools/compression_inflateBuffer.h"

#include "coreTools/compression/inflateBuffer.h"
#include "coreTools/exception/Exception.h"

#ifdef __cplusplus
extern "C" {
#endif 

GW2RE_CORETOOLS_API uint8_t* compression_inflateBuffer(uint32_t* iInputTab, const uint32_t iInputSize, uint32_t* ioOutputSize)
{
    if (ioOutputSize == nullptr)
    {
        printf("GW2RE_CORETOOLS_C_API(compression_inflateBuffer): ioOutputSize is NULL.");
        return NULL;
    }
    
    try
    {
        std::string anErrorMsg;
        uint8_t* aResultTab = gw2re::coreTools::compression::inflateBuffer(iInputTab, iInputSize, *ioOutputSize, anErrorMsg);
        if (aResultTab == NULL)
        {
            printf("GW2RE_CORETOOLS_C_API(compression_inflateBuffer): %s", anErrorMsg.c_str());
        }
        return aResultTab;
    }
    catch(gw2re::coreTools::exception::Exception& iException)
    {
        printf("GW2RE_CORETOOLS_C_API(compression_inflateBuffer): %s", iException.what());
        return NULL;
    }
}

#ifdef __cplusplus
}
#endif 
