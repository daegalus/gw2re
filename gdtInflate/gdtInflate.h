#ifndef GDTINFLATE_H
#define GDTINFLATE_H 1

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GDTINFLATE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GDTINFLATE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef GDTINFLATE_EXPORTS
#define GDTINFLATE_API __declspec(dllexport)
#else
#define GDTINFLATE_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif 

/** @Inputs:
 *    - i_input: Pointer to the buffer to inflate
 *    - i_inputSize: Size of the input buffer
 *    - io_outputSize: if the value pointed by io_outputSize is 0 then we decode everything
 *                     else we decode until we reach the outputSize
 *  @Outputs:
 *    - io_outputSize: Pointer to the size of the outputBuffer
 *  @Return:
 *    - Pointer to the outputBuffer, NULL if it failed, client is responsible to free this pointer
 */
GDTINFLATE_API uint8_t* gdtInflate(uint32_t* i_input, uint32_t i_inputSize, uint32_t* io_outputSize);

#ifdef __cplusplus
}
#endif

#endif // GDTINFLATE_H