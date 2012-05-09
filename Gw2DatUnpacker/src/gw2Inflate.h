// File gw2Inflate.h

#pragma once

#ifndef INFLATE_H_INCLUDED
#define INFLATE_H_INCLUDED

//#define DLL_EXPORT __declspec(dllexport)
#define DLL_EXPORT

namespace gw2du
{

/** @Inputs:
 *    - i_input: Pointer to the buffer to inflate
 *    - i_inputSize: Size of the input buffer
 *  @Outputs:
 *    - io_outputSize: Pointer to the size of the outputBuffer
 *  @Return:
 *    - Pointer to the outputBuffer, NULL if it failed, client is responsible to free this pointer
 */
byte* DLL_EXPORT inflate(byte* i_input, uint32 i_inputSize, uint32& io_outputSize);

}; // namespace gw2du

#endif // INFLATE_H_INCLUDED
