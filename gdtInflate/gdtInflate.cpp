#include "stdafx.h"
#include "gdtInflate.h"

#include "utils.h"

namespace gdt
{
namespace inflate
{

uint8_t inflate_data(State_t* i_state, uint8_t* i_output, uint32_t i_outputSize, HuffmanTree_t* i_huffmanTreeDict)
{
    assert(i_state != NULL);
    assert(i_output != NULL);
    assert(i_huffmanTreeDict != NULL);

    uint32_t l_outputPos = 0;

    // Reading the const write size addition value
    GDTINFLATE_CHECK(needBits(i_state, 8));
    GDTINFLATE_CHECK(dropBits(i_state, 4));
    uint16_t l_writeSizeConstAdd = GDTINFLATE_READBITS((*i_state), 4) + 1;
    GDTINFLATE_CHECK(dropBits(i_state, 4));

    // Declaring our HuffmanTrees
    HuffmanTree_t l_huffmanTreeCode;
    HuffmanTree_t l_huffmanTreeCopy;

    while (l_outputPos < i_outputSize)
    {
        // Resetting Huffmantrees
        memset(&l_huffmanTreeCode, 0, sizeof(HuffmanTree_t));
        memset(&l_huffmanTreeCopy, 0, sizeof(HuffmanTree_t));

        // Reading HuffmanTrees
        GDTINFLATE_CHECK(parseHuffmanTree(i_state, &l_huffmanTreeCode, i_huffmanTreeDict));
        GDTINFLATE_CHECK(parseHuffmanTree(i_state, &l_huffmanTreeCopy, i_huffmanTreeDict));

        // Reading MaxCount
        GDTINFLATE_CHECK(needBits(i_state, 4));
        uint32_t l_maxCount = (GDTINFLATE_READBITS((*i_state), 4) + 1) << 12;
        GDTINFLATE_CHECK(dropBits(i_state, 4));

        uint32_t l_currentCount = 0;

        while ((l_currentCount < l_maxCount) &&
               (l_outputPos < i_outputSize))
        {
            ++l_currentCount;

            // Reading next code
            uint16_t l_code = 0;
            GDTINFLATE_CHECK(readCode(&l_huffmanTreeCode, i_state, &l_code));

            if (l_code < 0x100)
            {
                i_output[l_outputPos] = (uint8_t)(l_code);
                ++l_outputPos;
                continue;
            }

            // We are in copy mode !
            // Reading the additional info to know the write size
            l_code -= 0x100;

            // write size
            div_t l_codeDiv4 = div(l_code, 4);

            uint32_t l_writeSize = 0;
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
                return GDTINFLATE_KO_RES;
            }

            //additional bits
            if (l_codeDiv4.quot > 1 && l_code != 28)
            {
                uint8_t l_writeSizeAddBits = l_codeDiv4.quot - 1;
                GDTINFLATE_CHECK(needBits(i_state, l_writeSizeAddBits));
                l_writeSize |= GDTINFLATE_READBITS((*i_state), l_writeSizeAddBits);
                GDTINFLATE_CHECK(dropBits(i_state, l_writeSizeAddBits));
            }
            l_writeSize += l_writeSizeConstAdd;

            // write offset
            // Reading the write offset
            GDTINFLATE_CHECK(readCode(&l_huffmanTreeCopy, i_state, &l_code));

            div_t l_codeDiv2 = div(l_code, 2);

            uint32_t l_writeOffset = 0;
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
                return GDTINFLATE_KO_RES;
            }

            //additional bits
            if (l_codeDiv2.quot > 1)
            {
                uint8_t l_writeOffsetAddBits = l_codeDiv2.quot - 1;
                GDTINFLATE_CHECK(needBits(i_state, l_writeOffsetAddBits));
                l_writeOffset |= GDTINFLATE_READBITS((*i_state), l_writeOffsetAddBits);
                GDTINFLATE_CHECK(dropBits(i_state, l_writeOffsetAddBits));
            }
            l_writeOffset += 1;

            uint32_t l_alreadyWritten = 0;
            while ((l_alreadyWritten < l_writeSize) &&
                   (l_outputPos < i_outputSize))
            {
                i_output[l_outputPos] = i_output[l_outputPos - l_writeOffset];
                ++l_outputPos;
                ++l_alreadyWritten;
            }
        }
    }
    return GDTINFLATE_OK_RES;
}

}
}

uint8_t* gdtInflate_impl(uint32_t* i_input, uint32_t i_inputSize, uint32_t* io_outputSize)
{
    assert(i_input != NULL);
    assert(io_outputSize != NULL);

    // Initilalize huffmanTreeDict
    gdt::inflate::HuffmanTree_t l_huffmanTreeDict;
    if (gdt::inflate::initializeHuffmanTreeDict(&l_huffmanTreeDict) != GDTINFLATE_OK_RES)
    {
        *io_outputSize = 0;
        return NULL;
    }

    // Initialize state
    gdt::inflate::State_t l_state;
    l_state.m_input = i_input;
    l_state.m_inputSize = i_inputSize;
    l_state.m_inputPos = 0;

    l_state.m_head = 0;
    l_state.m_bits = 0;
    l_state.m_buffer = 0;

    // Skipping header & Getting size of the uncompressed data
    if (gdt::inflate::needBits(&l_state, 32) != GDTINFLATE_OK_RES ||
        gdt::inflate::dropBits(&l_state, 32) != GDTINFLATE_OK_RES ||
        gdt::inflate::needBits(&l_state, 32) != GDTINFLATE_OK_RES)
    {
        printf("gw2Inflate: Error while reading the first eight bytes of the file.\n");
        *io_outputSize = 0;
        return NULL;
    }

    // Getting size of the uncompressed data
    uint32_t l_outputSize = GDTINFLATE_READBITS(l_state, 32);

	if (*io_outputSize != 0)
	{
		// We do not take max here as we won't be able to have more than the output available
		if (l_outputSize > *io_outputSize)
		{
			l_outputSize = *io_outputSize;
		}
	}

    if (gdt::inflate::dropBits(&l_state, 32) != GDTINFLATE_OK_RES)
    {
        printf("gw2Inflate: Error while reading the first eight bytes of the file.\n");
        *io_outputSize = 0;
        return NULL;
    }

    uint8_t* l_output = new uint8_t[l_outputSize];

    if (gdt::inflate::inflate_data(&l_state, l_output, l_outputSize, &l_huffmanTreeDict) != GDTINFLATE_OK_RES)
    {
        delete[] l_output;
        *io_outputSize = 0;
        return NULL;
    }

    *io_outputSize = l_outputSize;

    return l_output;
}


#ifdef __cplusplus
extern "C" {
#endif 

GDTINFLATE_API uint8_t* gdtInflate(uint32_t* i_input, uint32_t i_inputSize, uint32_t* io_outputSize)
{
    return gdtInflate_impl(i_input, i_inputSize, io_outputSize);
}

#ifdef __cplusplus
}
#endif

