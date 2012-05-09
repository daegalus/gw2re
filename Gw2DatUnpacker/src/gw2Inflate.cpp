// File: gw2Inflate.cpp

#include "stdafx.h"
#include "gw2Inflate.h"

#include "utils.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

namespace gw2du
{

byte inflate_data(State_t* i_state, byte* i_output, uint32 i_outputSize, HuffmanTree_t* i_huffmanTreeDict)
{
    assert(i_state != NULL);
    assert(i_output != NULL);
    assert(i_huffmanTreeDict != NULL);

    uint32 l_outputPos = 0;

    // Reading the const write size addition value
    CHECK(needBits(i_state, 8));
    CHECK(dropBits(i_state, 4));
    uint16 l_writeSizeConstAdd = READBITS((*i_state), 4) + 1;
    CHECK(dropBits(i_state, 4));

    // Declaring our HuffmanTrees
    HuffmanTree_t l_huffmanTreeCode;
    HuffmanTree_t l_huffmanTreeCopy;

    while (l_outputPos < i_outputSize)
    {
        // Resetting Huffmantrees
        memset(&l_huffmanTreeCode, 0, sizeof(HuffmanTree_t));
        memset(&l_huffmanTreeCopy, 0, sizeof(HuffmanTree_t));

        // Reading HuffmanTrees
        CHECK(parseHuffmanTree(i_state, &l_huffmanTreeCode, i_huffmanTreeDict));
        CHECK(parseHuffmanTree(i_state, &l_huffmanTreeCopy, i_huffmanTreeDict));

        // Reading MaxCount
        CHECK(needBits(i_state, 4));
        uint32 l_maxCount = (READBITS((*i_state), 4) + 1) << 12;
        CHECK(dropBits(i_state, 4));

        uint32 l_currentCount = 0;

        while ((l_currentCount < l_maxCount) &&
               (l_outputPos < i_outputSize))
        {
            ++l_currentCount;

            // Reading next code
            uint16 l_code = 0;
            CHECK(readCode(&l_huffmanTreeCode, i_state, &l_code));

            if (l_code < 0x100)
            {
                i_output[l_outputPos] = (byte)(l_code);
                ++l_outputPos;
                continue;
            }

            // We are in copy mode !
            // Reading the additional info to know the write size
            l_code -= 0x100;

            // write size
            div_t l_codeDiv4 = div(l_code, 4);

            uint32 l_writeSize = 0;
            if (l_codeDiv4.quot == 0)
            {
                l_writeSize = l_code;
            }
            else if (l_codeDiv4.quot < 7)
            {
                l_writeSize = ((1 << (l_codeDiv4.quot - 1)) * (4 + l_codeDiv4.rem));
            }
            else if (l_code == 28)
            {
                l_writeSize = 0xFF;
            }
            else
            {
                printf("gw2Inflate: Invalid value for writeSize code!\n");
                return KO_RES;
            }

            //additional bits
            if (l_codeDiv4.quot > 1 && l_code != 28)
            {
                byte l_writeSizeAddBits = l_codeDiv4.quot - 1;
                CHECK(needBits(i_state, l_writeSizeAddBits));
                l_writeSize |= READBITS((*i_state), l_writeSizeAddBits);
                CHECK(dropBits(i_state, l_writeSizeAddBits));
            }
            l_writeSize += l_writeSizeConstAdd;

            // write offset
            // Reading the write offset
            CHECK(readCode(&l_huffmanTreeCopy, i_state, &l_code));

            div_t l_codeDiv2 = div(l_code, 2);

            uint32 l_writeOffset = 0;
            if (l_codeDiv2.quot == 0)
            {
                l_writeOffset = l_code;
            }
            else if (l_codeDiv2.quot < 17)
            {
                l_writeOffset = ((1 << (l_codeDiv2.quot - 1)) * (2 + l_codeDiv2.rem));
            }
            else
            {
                printf("gw2Inflate: Invalid value for writeOffset code!\n");
                return KO_RES;
            }

            //additional bits
            if (l_codeDiv2.quot > 1)
            {
                byte l_writeOffsetAddBits = l_codeDiv2.quot - 1;
                CHECK(needBits(i_state, l_writeOffsetAddBits));
                l_writeOffset |= READBITS((*i_state), l_writeOffsetAddBits);
                CHECK(dropBits(i_state, l_writeOffsetAddBits));
            }
            l_writeOffset += 1;

            uint32 l_alreadyWritten = 0;
            while ((l_alreadyWritten < l_writeSize) &&
                   (l_outputPos < i_outputSize))
            {
                i_output[l_outputPos] = i_output[l_outputPos - l_writeOffset];
                ++l_outputPos;
                ++l_alreadyWritten;
            }
        }
    }
    return OK_RES;
}

byte* DLL_EXPORT inflate(byte* i_input, uint32 i_inputSize, uint32& io_outputSize)
{
    assert(i_input != NULL);
    assert(io_outputSize != NULL);

    // Initilalize huffmanTreeDict
    HuffmanTree_t l_huffmanTreeDict;
    if (initializeHuffmanTreeDict(&l_huffmanTreeDict) != OK_RES)
    {
        io_outputSize = 0;
        return 0;
    }

    // Initialize state
    State_t l_state;
    l_state.m_input = (uint32*)i_input;
    l_state.m_inputSize = i_inputSize;
    l_state.m_inputPos = 0;

    l_state.m_head = 0;
    l_state.m_bits = 0;
    l_state.m_buffer = 0;

    // Skipping header & Getting size of the uncompressed data
    if (needBits(&l_state, 32) != OK_RES ||
        dropBits(&l_state, 32) != OK_RES ||
        needBits(&l_state, 32) != OK_RES)
    {
        printf("gw2Inflate: Error while reading the first two bytes of the file.\n");
        io_outputSize = 0;
        return NULL;
    }

    // Getting size of the uncompressed data
    uint32 l_outputSize = READBITS(l_state, 32);
    if (dropBits(&l_state, 32) != OK_RES)
    {
        printf("gw2Inflate: Error while reading the first two bytes of the file.\n");
        io_outputSize = 0;
        return NULL;
    }

    byte* l_output = (byte*)malloc(l_outputSize);

    if (inflate_data(&l_state, l_output, l_outputSize, &l_huffmanTreeDict) != OK_RES)
    {
        free(l_output);
        io_outputSize = 0;
        return NULL;
    }

    io_outputSize = l_outputSize;

    return l_output;
}

}; // namespace gw2du
