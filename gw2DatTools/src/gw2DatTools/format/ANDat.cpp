#include "ANDat.h"

#include "Utils.h"

namespace gw2dt
{
namespace formats
{

std::unique_ptr<ANDat>&& parseANDat(std::istream& iStream, const uint64_t& iOffset, const uint32_t iSize);
{
    iStream.seek(iOffset);
    
    std::unique_ptr<ANDat> pANDat(new ANDat());
    readStruct(iStream, pANDat->header);
    
    return std::move(pANDat);
}

}
}

#endif // GW2DATTOOLS_FORMATS_ANDAT_H
