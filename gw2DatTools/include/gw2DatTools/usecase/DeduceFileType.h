#ifndef GW2DATTOOLS_USECASE_DEDUCEFILETYPE_H
#define GW2DATTOOLS_USECASE_DEDUCEFILETYPE_H

#include "gw2DatTools/dllMacros.h"

#include "gw2DatTools/interface/ANDatInterface.h"

namespace gw2dt
{
namespace usecase
{

GW2DATTOOLS_API format::FileType GW2DATTOOLS_APIENTRY deduceFileType(const interface::ANDatInterface& iDatInterface, const interface::ANDatInterface::FileRecord& iFileRecord);

GW2DATTOOLS_API format::FileType GW2DATTOOLS_APIENTRY deduceFileType(uint8_t* iUncompressedBuffer, const uint32_t iInputSize);

}
}

#endif // GW2DATTOOLS_USECASE_DEDUCEFILETYPE_H
