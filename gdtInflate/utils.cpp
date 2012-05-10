#include "stdafx.h"

#include "utils.h"

namespace gdt
{
namespace inflate
{

uint8_t pull(State_t* i_state)
{
    assert(i_state != NULL);

    // checking that we have less than 32 bits available
    if (i_state->m_bits >= 32)
    {
        printf("gw2Inflate: Tried to pull a value while we still have 32 bits available!\n");
        return GDTINFLATE_KO_RES;
    }

    // skip the last element of all 65536 bytes blocks
    if ((i_state->m_inputPos + 1) % (0x4000) == 0)
    {
        ++(i_state->m_inputPos);
    }

    // checking that inputPos is not out of bounds
    if (i_state->m_inputPos >= i_state->m_inputSize)
    {
        printf("gw2Inflate: Reached end of input while trying to fetch a new value!\n");
        return GDTINFLATE_KO_RES;
    }

    // Fetching the next value
    uint32_t l_value = i_state->m_input[i_state->m_inputPos];

    // Pulling the data into head/buffer given that we need to keep the relevant bits
    if (i_state->m_bits == 0)
    {
        i_state->m_head = l_value;
        i_state->m_buffer = 0;
    }
    else
    {
        i_state->m_head = i_state->m_head | (l_value >> (i_state->m_bits));
        i_state->m_buffer = (l_value << (32 - i_state->m_bits));
    }

    // Updating state variables
    i_state->m_bits += 32;
    ++(i_state->m_inputPos);

    return GDTINFLATE_OK_RES;
}

uint8_t needBits(State_t* i_state, uint8_t i_bits)
{
    assert(i_state != NULL);

    // checking that we request at most 32 bits
    if (i_bits > 32)
    {
        printf("gw2Inflate: Tried to need more than 32 bits!\n");
        return GDTINFLATE_KO_RES;
    }

    if (i_state->m_bits < i_bits)
    {
        return pull(i_state);
    }

    return GDTINFLATE_OK_RES;
}

uint8_t dropBits(State_t* i_state, uint8_t i_bits)
{
    assert(i_state != NULL);

    // checking that we request at most 32 bits
    if (i_bits > 32)
    {
        printf("gw2Inflate: Tried to need more than 32 bits!\n");
        return GDTINFLATE_KO_RES;
    }

    if (i_bits > i_state->m_bits)
    {
        printf("gw2Inflate: Tried to drop more bits than we have!\n");
        return GDTINFLATE_KO_RES;
    }

    // Updating the values to drop the bits
    if (i_bits == 32)
    {
        i_state->m_head = i_state->m_buffer;
        i_state->m_buffer = 0;
    }
    else
    {
        i_state->m_head = ((i_state->m_head) << i_bits) | ((i_state->m_buffer) >> (32 - i_bits));
        i_state->m_buffer = (i_state->m_buffer) << i_bits;
    }

    // update state info
    i_state->m_bits -= i_bits;

    return GDTINFLATE_OK_RES;
}

uint8_t readCode(HuffmanTree_t* i_huffmanTree, State_t* i_state, uint16_t* io_code)
{
    assert(i_huffmanTree != NULL);
    assert(i_state != NULL);
    assert(io_code != NULL);

    // Special case for weird files
    if (i_huffmanTree->m_hasData == 0)
    {
        printf("gw2Inflate: HuffmanTree is empty!\n");
        return GDTINFLATE_KO_RES;
    }

    GDTINFLATE_CHECK(needBits(i_state, 32));
    uint16_t l_index = 0;
    while (GDTINFLATE_READBITS((*i_state), 32) < i_huffmanTree->m_codeComp[l_index])
    {
        ++l_index;
    }

    uint8_t l_bitsNb = i_huffmanTree->m_codeBits[l_index];
    *io_code = i_huffmanTree->m_codeValue[i_huffmanTree->m_tabOffset[l_index] -
                                          ((GDTINFLATE_READBITS((*i_state), 32) - i_huffmanTree->m_codeComp[l_index]) >> (32 - l_bitsNb))];
    return dropBits(i_state, l_bitsNb);
}

uint8_t buildHuffmanTree(HuffmanTree_t* i_huffmanTree, int16_t* i_workingBitTab, int16_t* i_workingCodeTab)
{
    assert(i_huffmanTree != NULL);
    assert(i_workingBitTab != NULL);
    assert(i_workingCodeTab != NULL);

    // Building the HuffmanTree
    uint32_t l_code = 0;
    uint8_t l_bitsNb = 0;
    uint16_t l_codeCompIndex = 0;
    uint16_t l_codeOffset = 0;

    while (l_bitsNb < GDTINFLATE_MAX_CODE_BITS_LENGTH)
    {
        if (i_workingBitTab[l_bitsNb] != -1)
        {
            int16_t l_currentSymbol = i_workingBitTab[l_bitsNb];
            while (l_currentSymbol != -1)
            {
                // Registering the code
                i_huffmanTree->m_codeValue[l_codeOffset] = l_currentSymbol;

                ++l_codeOffset;
                l_currentSymbol = i_workingCodeTab[l_currentSymbol];
                --l_code;
            }

            // Minimum code value for l_bitsNb bits
            i_huffmanTree->m_codeComp[l_codeCompIndex] = ((l_code + 1) << (32 - l_bitsNb));

            // Number of bits for l_codeCompIndex index
            i_huffmanTree->m_codeBits[l_codeCompIndex] = l_bitsNb;

            // Offset in m_codeValue table to reach the value
            i_huffmanTree->m_tabOffset[l_codeCompIndex] = l_codeOffset - 1;

            // Flag to know if the tree is empty or not
            i_huffmanTree->m_hasData = 1;

            ++l_codeCompIndex;
        }
        l_code = (l_code << 1) + 1;
        ++l_bitsNb;
    }

    return GDTINFLATE_OK_RES;
}

uint8_t parseHuffmanTree(State_t* i_state, HuffmanTree_t* i_huffmanTree, HuffmanTree_t* i_huffmanTreeDict)
{
    assert(i_state != NULL);
    assert(i_huffmanTree != NULL);
    assert(i_huffmanTreeDict != NULL);

    // Reading the number of symbols to read
    GDTINFLATE_CHECK(needBits(i_state, 16));
    uint16_t l_numberOfSymbols = (uint16_t)(GDTINFLATE_READBITS((*i_state), 16));
    GDTINFLATE_CHECK(dropBits(i_state, 16));

    if (l_numberOfSymbols > GDTINFLATE_MAX_CODE_NUMBER)
    {
        printf("Too many symbols to decode: %u\n", l_numberOfSymbols);
        return GDTINFLATE_KO_RES;
    }

    int16_t l_workingBitTab[GDTINFLATE_MAX_CODE_BITS_LENGTH];
    int16_t l_workingCodeTab[GDTINFLATE_MAX_CODE_NUMBER];

    // Initialize our workingTabs
    memset(&l_workingBitTab, 0xFF, GDTINFLATE_MAX_CODE_BITS_LENGTH * sizeof(int16_t));
    memset(&l_workingCodeTab, 0xFF, GDTINFLATE_MAX_CODE_NUMBER * sizeof(int16_t));

    int16_t l_remainingSymbols = l_numberOfSymbols - 1;

    // Fetching the code repartition
    while (l_remainingSymbols > -1)
    {
        uint16_t l_code = 0;
        GDTINFLATE_CHECK(readCode(i_huffmanTreeDict, i_state, &l_code));

        uint16_t l_codeNumberOfBits = l_code & 0x1F;
        uint16_t l_codeNumberOfSymbols = (l_code >> 5) + 1;

        if (l_codeNumberOfBits == 0)
        {
            l_remainingSymbols -= l_codeNumberOfSymbols;
        }
        else
        {
            while (l_codeNumberOfSymbols > 0)
            {
                if (l_workingBitTab[l_codeNumberOfBits] == -1)
                {
                    l_workingBitTab[l_codeNumberOfBits] = l_remainingSymbols;
                }
                else
                {
                    l_workingCodeTab[l_remainingSymbols] = l_workingBitTab[l_codeNumberOfBits];
                    l_workingBitTab[l_codeNumberOfBits] = l_remainingSymbols;
                }
                --l_remainingSymbols;
                --l_codeNumberOfSymbols;
            }
        }
    }

    // Effectively build the Huffmanree
    return buildHuffmanTree(i_huffmanTree, &l_workingBitTab[0], &l_workingCodeTab[0]);
}

uint8_t fillTabsHelper(uint8_t i_bits, int16_t i_symbol, int16_t* i_workingBitTab, int16_t* i_workingCodeTab)
{
    assert(i_workingBitTab != NULL);
    assert(i_workingCodeTab != NULL);

    // checking out of bounds
    if (i_bits >= GDTINFLATE_MAX_CODE_BITS_LENGTH || i_symbol >= GDTINFLATE_MAX_CODE_NUMBER)
    {
        return GDTINFLATE_KO_RES;
    }

    if (i_workingBitTab[i_bits] == -1)
    {
        i_workingBitTab[i_bits] = i_symbol;
    }
    else
    {
        i_workingCodeTab[i_symbol] = i_workingBitTab[i_bits];
        i_workingBitTab[i_bits] = i_symbol;
    }

    return GDTINFLATE_OK_RES;
}

uint8_t initializeHuffmanTreeDict(HuffmanTree_t* i_huffmanTree)
{
    assert(i_huffmanTree != NULL);

    int16_t l_workingBitTab[GDTINFLATE_MAX_CODE_BITS_LENGTH];
    int16_t l_workingCodeTab[GDTINFLATE_MAX_CODE_NUMBER];

    // Initialize our workingTabs
    memset(&l_workingBitTab, 0xFF, GDTINFLATE_MAX_CODE_BITS_LENGTH * sizeof(int16_t));
    memset(&l_workingCodeTab, 0xFF, GDTINFLATE_MAX_CODE_NUMBER * sizeof(int16_t));

    uint8_t l_result =
    fillTabsHelper(3, 0x0A, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(3, 0x09, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(3, 0x08, &l_workingBitTab[0], &l_workingCodeTab[0]) ||

    fillTabsHelper(4, 0x0C, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(4, 0x0B, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(4, 0x07, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(4, 0x00, &l_workingBitTab[0], &l_workingCodeTab[0]) ||

    fillTabsHelper(5, 0xE0, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(5, 0x2A, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(5, 0x29, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(5, 0x06, &l_workingBitTab[0], &l_workingCodeTab[0]) ||

    fillTabsHelper(6, 0x4A, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(6, 0x40, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(6, 0x2C, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(6, 0x2B, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(6, 0x28, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(6, 0x20, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(6, 0x05, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(6, 0x04, &l_workingBitTab[0], &l_workingCodeTab[0]) ||

    fillTabsHelper(7, 0x49, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(7, 0x48, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(7, 0x27, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(7, 0x26, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(7, 0x25, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(7, 0x0D, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(7, 0x03, &l_workingBitTab[0], &l_workingCodeTab[0]) ||

    fillTabsHelper(8, 0x6A, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(8, 0x69, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(8, 0x4C, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(8, 0x4B, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(8, 0x47, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(8, 0x24, &l_workingBitTab[0], &l_workingCodeTab[0]) ||

    fillTabsHelper(9, 0xE8, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(9, 0xA0, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(9, 0x89, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(9, 0x88, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(9, 0x68, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(9, 0x67, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(9, 0x63, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(9, 0x60, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(9, 0x46, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(9, 0x23, &l_workingBitTab[0], &l_workingCodeTab[0]) ||

    fillTabsHelper(10, 0xE9, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(10, 0xC9, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(10, 0xC0, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(10, 0xA9, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(10, 0xA8, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(10, 0x8A, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(10, 0x87, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(10, 0x80, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(10, 0x66, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(10, 0x65, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(10, 0x45, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(10, 0x44, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(10, 0x43, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(10, 0x2D, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(10, 0x02, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(10, 0x01, &l_workingBitTab[0], &l_workingCodeTab[0]) ||

    fillTabsHelper(11, 0xE5, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(11, 0xC8, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(11, 0xAA, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(11, 0xA5, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(11, 0xA4, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(11, 0x8B, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(11, 0x85, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(11, 0x84, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(11, 0x6C, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(11, 0x6B, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(11, 0x64, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(11, 0x4D, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(11, 0x0E, &l_workingBitTab[0], &l_workingCodeTab[0]) ||

    fillTabsHelper(12, 0xE7, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(12, 0xCA, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(12, 0xC7, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(12, 0xA7, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(12, 0xA6, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(12, 0x86, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(12, 0x83, &l_workingBitTab[0], &l_workingCodeTab[0]) ||

    fillTabsHelper(13, 0xE6, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(13, 0xE4, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(13, 0xC4, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(13, 0x8C, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(13, 0x2E, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(13, 0x22, &l_workingBitTab[0], &l_workingCodeTab[0]) ||

    fillTabsHelper(14, 0xEC, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(14, 0xC6, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(14, 0x6D, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(14, 0x4E, &l_workingBitTab[0], &l_workingCodeTab[0]) ||

    fillTabsHelper(15, 0xEA, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(15, 0xCC, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(15, 0xAC, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(15, 0xAB, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(15, 0x8D, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(15, 0x11, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(15, 0x10, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(15, 0x0F, &l_workingBitTab[0], &l_workingCodeTab[0]) ||

    fillTabsHelper(16, 0xFF, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xFE, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xFD, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xFC, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xFB, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xFA, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xF9, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xF8, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xF7, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xF6, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xF5, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xF4, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xF3, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xF2, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xF1, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xF0, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xEF, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xEE, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xED, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xEB, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xE3, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xE2, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xE1, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xDF, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xDE, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xDD, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xDC, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xDB, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xDA, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xD9, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xD8, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xD7, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xD6, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xD5, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xD4, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xD3, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xD2, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xD1, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xD0, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xCF, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xCE, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xCD, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xCB, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xC5, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xC3, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xC2, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xC1, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xBF, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xBE, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xBD, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xBC, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xBB, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xBA, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xB9, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xB8, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xB7, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xB6, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xB5, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xB4, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xB3, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xB2, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xB1, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xB0, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xAF, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xAE, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xAD, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xA3, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xA2, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0xA1, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x9F, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x9E, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x9D, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x9C, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x9B, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x9A, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x99, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x98, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x97, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x96, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x95, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x94, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x93, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x92, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x91, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x90, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x8F, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x8E, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x82, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x81, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x7F, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x7E, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x7D, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x7C, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x7B, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x7A, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x79, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x78, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x77, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x76, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x75, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x74, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x73, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x72, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x71, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x70, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x6F, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x6E, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x62, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x61, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x5F, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x5E, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x5D, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x5C, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x5B, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x5A, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x59, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x58, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x57, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x56, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x55, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x54, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x53, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x52, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x51, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x50, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x4F, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x42, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x41, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x3F, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x3E, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x3D, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x3C, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x3B, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x3A, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x39, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x38, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x37, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x36, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x35, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x34, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x33, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x32, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x31, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x30, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x2F, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x21, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x1F, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x1E, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x1D, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x1C, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x1B, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x1A, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x19, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x18, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x17, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x16, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x15, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x14, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x13, &l_workingBitTab[0], &l_workingCodeTab[0]) ||
    fillTabsHelper(16, 0x12, &l_workingBitTab[0], &l_workingCodeTab[0]);

    GDTINFLATE_CHECK(l_result);

    return buildHuffmanTree(i_huffmanTree, &l_workingBitTab[0], &l_workingCodeTab[0]);
}

}
}
