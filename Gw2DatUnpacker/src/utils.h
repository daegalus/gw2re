// File: utils.h

#pragma once

#ifndef UTILS_H
#define UTILS_H 1

#define MAX_CODE_BITS_LENGTH 32
#define MAX_CODE_NUMBER 285

// Simple macros for return values
#define OK_RES 0
#define KO_RES 1

#define CHECK(var_res)          \
    do                          \
    {                           \
        if ((var_res) != OK_RES)\
        {                       \
            return KO_RES;      \
        }                       \
    }                           \
    while(0)

namespace gw2du
{

struct HuffmanTree
{
    uint32 m_codeComp[MAX_CODE_BITS_LENGTH];
    byte m_codeBits[MAX_CODE_BITS_LENGTH];
    uint16 m_tabOffset[MAX_CODE_BITS_LENGTH];
    uint32 m_codeValue[MAX_CODE_NUMBER];
    byte m_hasData;
};

typedef struct HuffmanTree HuffmanTree_t;

struct State
{
    uint32* m_input;
    uint32 m_inputSize;
    uint32 m_inputPos;

    uint32 m_head;
    byte m_bits;
    uint32 m_buffer;
};

typedef struct State State_t;

// Bits manipulation
byte needBits(State_t* i_state, byte i_bits);
#define READBITS(var_state, var_bits) (((var_state).m_head) >> (32 - (var_bits)))
byte dropBits(State_t* i_state, byte i_bits);

// Read the next code
byte readCode(HuffmanTree_t* i_huffmanTree, State_t* i_state, uint16* io_code);

// Parse and build a huffmanTree
byte parseHuffmanTree(State_t* i_state, HuffmanTree_t* i_huffmanTree, HuffmanTree_t* i_huffmanTreeDict);

// Initialize the statis HuffmanTreeDict
byte initializeHuffmanTreeDict(HuffmanTree_t* i_huffmanTree);

}; // namespace gw2du

#endif
