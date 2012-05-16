#include "Mft.h"

#include "Utils.h"

namespace gw2dt
{
namespace formats
{

std::unique_ptr<Mft>&& parseMft(std::istream& iStream, const uint64_t& iOffset, const uint32_t iSize)
{
    iStream.seek(iOffset);
    
    std::unique_ptr<Mft> pMft(new Mft());
    readStruct(iStream, pMft->header);
    
    pMft->entries.resize(pMft->header.mbOfEntries - 1);
    readStruct(iStream, pMft->entries);
    
    return std::move(pMft);
}

}
}
