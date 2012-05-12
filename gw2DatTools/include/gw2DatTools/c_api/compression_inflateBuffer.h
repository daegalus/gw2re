#ifndef GW2DATTOOLS_CAPI_COMPRESSION_INFLATEBUFFER_H
#define GW2DATTOOLS_CAPI_COMPRESSION_INFLATEBUFFER_H

#include <stdint.h>

#include "gw2DatTools/dllMacros.h"

#ifdef __cplusplus
extern "C" {
#endif 

/** @Inputs:
 *    - iInputTab: Pointer to the buffer to inflate
 *    - iInputSize: Size of the input buffer
 *    - ioOutputSize: if the value is 0 then we decode everything
 *                    else we decode until we reach the io_outputSize
 *  @Outputs:
 *    - ioOutputSize: size of the outputBuffer
 *  @Return:
 *    - Pointer to the outputBuffer, NULL if it failed
 */
GW2DATTOOLS_API uint8_t* GW2DATTOOLS_APIENTRY compression_inflateBuffer(uint32_t* iInputTab, const uint32_t iInputSize, uint32_t* ioOutputSize);

#ifdef __cplusplus
}
#endif 

#endif // GW2DATTOOLS_CAPI_COMPRESSION_INFLATEBUFFER_H
