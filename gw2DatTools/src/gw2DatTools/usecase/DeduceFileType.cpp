#include "gw2DatTools/usecase/DeduceFileType.h"

namespace gw2dt
{
namespace usecase
{

GW2DATTOOLS_API format::FileType GW2DATTOOLS_APIENTRY deduceFileType(uint8_t* iUncompressedBuffer, const uint32_t iInputSize)
{
	return format::FT_Unknown;
}

}
}

