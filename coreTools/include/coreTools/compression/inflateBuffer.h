#ifndef GW2RE_CORETOOLS_COMPRESSION_INFLATEBUFFER_H
#define GW2RE_CORETOOLS_COMPRESSION_INFLATEBUFFER_H

#include <cstdint>
#include <string>

#include "coreTools/dllMacros.h"

namespace gw2re
{
namespace coreTools
{
namespace compression
{

/** @Inputs:
 *    - iInputTab: Pointer to the buffer to inflate
 *    - iInputSize: Size of the input buffer
 *    - ioOutputSize: if the value is 0 then we decode everything
 *                    else we decode until we reach the io_outputSize
 *  @Outputs:
 *    - ioOutputSize: size of the outputBuffer
 *    - ioErrorMsg: error that occured if it failed
 *  @Return:
 *    - Pointer to the outputBuffer
 *  @Throws:
 *    - coreTools::exception::Exception in case an error is found 
 *      before trying to inflate the buffer
 */
GW2RE_CORETOOLS_API uint8_t* GW2RE_CORETOOLS_APIENTRY inflateBuffer(uint32_t* iInputTab, const uint32_t iInputSize, uint32_t& ioOutputSize, std::string& ioErrorMsg);

}
}
}

#endif // GW2RE_CORETOOLS_COMPRESSION_INFLATEBUFFER_H
