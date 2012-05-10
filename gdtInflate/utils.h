#ifndef GDT_INFLATE_UTILS_H
#define GDT_INFLATE_UTILS_H 1

#include "stdafx.h"

#define GDTINFLATE_MAX_CODE_BITS_LENGTH 32
#define GDTINFLATE_MAX_CODE_NUMBER 285

// Simple macros for return values
#define GDTINFLATE_OK_RES 0
#define GDTINFLATE_KO_RES 1

#define GDTINFLATE_CHECK(var_res)          \
    do                                     \
    {                                      \
        if ((var_res) != GDTINFLATE_OK_RES)\
        {                                  \
            return GDTINFLATE_KO_RES;      \
        }                                  \
    }                                      \
    while(0)

#define GDTINFLATE_READBITS(var_state, var_bits) (((var_state).m_head) >> (32 - (var_bits)))

namespace gdt
{
namespace inflate
{

struct HuffmanTree
{
    uint32_t m_codeComp[GDTINFLATE_MAX_CODE_BITS_LENGTH];
    uint8_t m_codeBits[GDTINFLATE_MAX_CODE_BITS_LENGTH];
    uint16_t m_tabOffset[GDTINFLATE_MAX_CODE_BITS_LENGTH];
    uint32_t m_codeValue[GDTINFLATE_MAX_CODE_NUMBER];
    uint8_t m_hasData;
};

typedef struct HuffmanTree HuffmanTree_t;

struct State
{
    uint32_t* m_input;
    uint32_t m_inputSize;
    uint32_t m_inputPos;

    uint32_t m_head;
    uint8_t m_bits;
    uint32_t m_buffer;
};

typedef struct State State_t;

// Bits manipulation
uint8_t needBits(State_t* i_state, uint8_t i_bits);
uint8_t dropBits(State_t* i_state, uint8_t i_bits);

// Read the next code
uint8_t readCode(HuffmanTree_t* i_huffmanTree, State_t* i_state, uint16_t* io_code);

// Parse and build a huffmanTree
uint8_t parseHuffmanTree(State_t* i_state, HuffmanTree_t* i_huffmanTree, HuffmanTree_t* i_huffmanTreeDict);

// Initialize the statis HuffmanTreeDict
uint8_t initializeHuffmanTreeDict(HuffmanTree_t* i_huffmanTree);

}
}

#endif // GDT_INFLATE_UTILS_H
