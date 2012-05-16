#ifndef GW2DATTOOLS_INTERFACE_ANDATINTERFACE_H
#define GW2DATTOOLS_INTERFACE_ANDATINTERFACE_H

#include "gw2DatTools/dllMacros.h"

namespace gw2dt
{
namespace interface
{

class GW2DATTOOLS_API ANDatInterface
{
    public:
        struct FileRecord
        {
            uint64_t offset;
            uint32_t size;
            
            uint32_t baseId;
            uint32_t fileId;
            
            bool isCompressed;
        };
        
        virtual ~ANDatInterface() {} = 0;
        
        virtual uint8_t* getBuffer(const FileRecord& iFileRecord, const uint32_t& ioOutputSize) const = 0;
        virtual void getBuffer(const ANDatInterface::FileRecord& iFileRecord, uint8_t* ioBuffer, const uint32_t& ioOutputSize) const = 0;
        
        virtual const FileRecord& getFileRecordForFileId(const uint32_t& iFileId) const = 0;
        virtual const FileRecord& getFileRecordForBaseId(const uint32_t& iBaseId) const = 0;
        
        virtual const std::vector<FileRecord>& getFileRecordVect() const = 0;
        
        virtual const FileType retrieveFileType(const FileRecord& iFileRecord) = 0;
};

std::unique_ptr<ANDatInterface>&& createANDatInterface(const char* iDatPath);

}
}

#endif // GW2DATTOOLS_INTERFACE_ANDATINTERFACE_H
