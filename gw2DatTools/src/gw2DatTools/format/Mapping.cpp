#include "Mapping.h"

#include "Utils.h"

namespace gw2dt
{
namespace formats
{

std::unique_ptr<Mapping>&& parseMapping(std::istream& iStream, const uint64_t& iOffset, const uint32_t iSize)
{
    iStream.seek(iOffset);
    
    std::unique_ptr<Mapping> pMapping(new Mapping());
    
    uint32_t aNbOfEntries = iSize / sizeof(MappingEntry);
    
    pMapping->entries.resize(aNbOfEntries);
    readStruct(iStream, pMapping->entries);
    
    return std::move(pMapping);
}

}
}
