#ifndef GW2DATTOOLS_COMPRESSION_INFLATEBUFFERUTILS_H
#define GW2DATTOOLS_COMPRESSION_INFLATEBUFFERUTILS_H

#include "gw2DatTools/exception/Exception.h"

#include <cstdint>

namespace gw2dt
{
namespace compression
{

static const uint32_t MaxCodeBitsLength = 32; // Max number of bits per code
static const uint32_t MaxSymbolValue = 285;   // Max value for a symbol

struct HuffmanTree
{
    uint32_t codeCompTab[MaxCodeBitsLength];
    uint8_t codeBitsTab[MaxCodeBitsLength];
    uint16_t symbolValueTabOffsetTab[MaxCodeBitsLength];
    uint32_t symbolValueTab[MaxSymbolValue];
};

struct State
{
    uint32_t* input;
    uint32_t inputSize;
    uint32_t inputPos;

    uint32_t head;
    uint32_t buffer;
    uint8_t bits;
};

// Parse and build a huffmanTree
void parseHuffmanTree(State& ioState, HuffmanTree& ioHuffmanTree);

// Initialize the statis HuffmanTreeDict
void initializeHuffmanTreeDict(HuffmanTree& ioHuffmanTree);

// Read the next code
void readCode(const HuffmanTree& iHuffmanTree, State& ioState, uint16_t& ioCode);

// Bits manipulation
inline void pullByte(State& ioState)
{
    // checking that we have less than 32 bits available
    if (ioState.bits >= 32)
    {
        throw exception::Exception("Tried to pull a value while we still have 32 bits available.");
    }

    // skip the last element of all 65536 bytes blocks
    if ((ioState.inputPos + 1) % (0x4000) == 0)
    {
        ++(ioState.inputPos);
    }

    // checking that inputPos is not out of bounds
    if (ioState.inputPos >= ioState.inputSize)
    {
        throw exception::Exception("Reached end of input while trying to fetch a new byte.");
    }

    // Fetching the next value
    uint32_t aValue = ioState.input[ioState.inputPos];

    // Pulling the data into head/buffer given that we need to keep the relevant bits
    if (ioState.bits == 0)
    {
        ioState.head = aValue;
        ioState.buffer = 0;
    }
    else
    {
        ioState.head = ioState.head | (aValue >> (ioState.bits));
        ioState.buffer = (aValue << (32 - ioState.bits));
    }

    // Updating state variables
    ioState.bits += 32;
    ++(ioState.inputPos);
}

inline void needBits(State& ioState, const uint8_t iBits)
{
    // checking that we request at most 32 bits
    if (iBits > 32)
    {
        throw exception::Exception("Tried to need more than 32 bits.");
    }

    if (ioState.bits < iBits)
    {
        pullByte(ioState);
    }
}

inline void dropBits(State& ioState, const uint8_t iBits)
{
    // checking that we request at most 32 bits
    if (iBits > 32)
    {
        throw exception::Exception("Tried to drop more than 32 bits.");
    }

    if (iBits > ioState.bits)
    {
        throw exception::Exception("Tried to drop more bits than we have.");
    }

    // Updating the values to drop the bits
    if (iBits == 32)
    {
        ioState.head = ioState.buffer;
        ioState.buffer = 0;
    }
    else
    {
        ioState.head = ((ioState.head) << iBits) | ((ioState.buffer) >> (32 - iBits));
        ioState.buffer = (ioState.buffer) << iBits;
    }

    // update state info
    ioState.bits -= iBits;
}

inline uint32_t readBits(const State& iState, const uint8_t iBits)
{
    return (iState.head) >> (32 - iBits);
}

void initializeHuffmanTreeDict();

}
}

#endif // GW2DATTOOLS_COMPRESSION_INFLATEBUFFERUTILS_H
