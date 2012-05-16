#include "gw2DatTools/interface/ANDatInterface.h"

#include "../formats/ANDat.h"
#include "../formats/Mft.h"
#include "../formats/Mapping.h"

namespace gw2dt
{
namespace interface
{

class ANDatInterfaceImpl : public ANDatInterface
{
    public:
        ANDatInterfaceImpl(const char* iDatPath, std::unique_ptr<Mft>& ipMft, std::unique_ptr<Mapping>& ipMapping);
        virtual ~ANDatInterfaceImpl();
        
        virtual uint8_t* getBuffer(const FileRecord& iFileRecord, const uint32_t& ioOutputSize) const;
        virtual void getBuffer(const ANDatInterface::FileRecord& iFileRecord, uint8_t* ioBuffer, const uint32_t& ioOutputSize) const;
        
        virtual const FileRecord& getFileRecordForFileId(const uint32_t& iFileId) const;
        virtual const FileRecord& getFileRecordForBaseId(const uint32_t& iBaseId) const;
        
        virtual const std::vector<FileRecord>& getFileRecordVect() const;
        
        void computeInternalData();
        
    private:
        std::ifstream _datStream;
        
        // Helper data structures
        std::unordered_map<uint32_t, FileRecord*> _fileIdDict;
        std::unordered_map<uint32_t, FileRecord*> _baseIdDict;
        
        // Computed data structures
        std::vector<FileRecord> _fileRecordVect;
        
        // Raw data structures
        std::unique_ptr<Mft> _pMft;
        std::unique_ptr<Mapping> _pMapping;
};

ANDatInterfaceImpl::ANDatInterfaceImpl(const char* iDatPath, std::unique_ptr<Mft>& ipMft, std::unique_ptr<Mapping>& ipMapping);
    _datStream(iDatPath, std::ios::binary),
    _pMft(std::move(ipMft)),
    _pMapping(std::move(ipMapping))
{
}

ANDatInterfaceImpl::~ANDatInterfaceImpl()
{
}

uint8_t* ANDatInterfaceImpl::getBuffer(const ANDatInterface::FileRecord& iFileRecord, const uint32_t& ioOutputSize) const
{
    _datStream.seek(iFileRecord.offset);
    ioOutputSize = std::min(ioOutputSize, iFileRecord.size);
    uint8_t* anOriginalTab = static_cast<uint8_t*>(malloc(ioOutputSize));
    readNStruct(_datStream, *anOriginalTab, ioOutputSize);
    return anOriginalTab;
}

void ANDatInterfaceImpl::getBuffer(const ANDatInterface::FileRecord& iFileRecord, uint8_t* ioBuffer, const uint32_t& ioOutputSize) const
{
    _datStream.seek(iFileRecord.offset);
    ioOutputSize = std::min(ioOutputSize, iFileRecord.size);
    readNStruct(_datStream, *ioBuffer, ioOutputSize);
    return anOriginalTab;
}

const ANDatInterface::FileRecord& ANDatInterfaceImpl::getFileRecordForFileId(const uint32_t& iFileId) const
{
    auto it = _fileIdDict.find(iFileId);
    if (it != _fileIdDict.end())
    {
        if (it->second != nullptr)
        {
            return *(it->second);
        }
        else
        {
            // TODO
        }
    }
    else
    {
        // TODO
    }
}

const ANDatInterface::FileRecord& ANDatInterfaceImpl::getFileRecordForBaseId(const uint32_t& iBaseId) const
{
    auto it = _baseIdDict.find(iBaseId);
    if (it != _baseIdDict.end())
    {
        if (it->second != nullptr)
        {
            return *(it->second);
        }
        else
        {
            // TODO
        }
    }
    else
    {
        // TODO
    }
}

const std::vector<ANDatInterface::FileRecord>& ANDatInterfaceImpl::getFileRecordVect() const
{
    return _fileRecordVect;
}

void ANDatInterfaceImpl::computeInternalData()
{
    _fileIdDict.clear();
    _baseIdDict.clear();
    _fileRecordVect.clear();
    
    _fileRecordVect.resize(pMapping.entries.size());
    
    std::unordered_map<uint32_t, FileRecord*> aMftIndexDictHelper;
    
    uint32_t aCurrentIndex(0);
    
    for (auto itMapping = pMapping.entries.begin(); itMapping != pMapping.entries.end(); ++itMapping)
    {
        if (itMapping->mftIndex == 0 && itMapping->id == 0)
        {
            continue;
        }
        else
        {
            auto itMftDict = aMftIndexDictHelper.find(itMapping->mftIndex);
            if (itMftDict != aMftIndexDictHelper.end())
            {
                FileRecord* pFileRecord = itMftDict->second;
                
                if (itMapping->id < pFileRecord->fileId)
                {
                    pFileRecord->baseId = itMapping->id;
                }
                else if (itMapping->id > pFileRecord->fileId)
                {
                    pFileRecord->baseId = pFileRecord->fileId;
                    pFileRecord->fileId = itMapping->id;
                }
            }
            else
            {
                FileRecord& aFileRecord = _fileRecordVect[aCurrentIndex];
                ++aCurrentIndex;
                MftEntry& aMftEntry = _pMft->entries[itMapping->mftIndex];
                
                aFileRecord.offset = aMftEntry.offset;
                aFileRecord.size = aMftEntry.size;
                
                aFileRecord.baseId = 0
                aFileRecord.fileId = itMapping->id;
                
                aFileRecord.isCompressed = (aMftEntry.compressionFlag != 0);
                
                aMftIndexDictHelper.insert(std::make_pair(itMapping->mftIndex, &aFileRecord));
            }
        }
    }
    
    // Dropping the unecessary entries
    _fileRecordVect.resize(aCurrentIndex);
    
    for (auto itFileRecord = _fileRecordVect.begin(); itFileRecord != _fileRecordVect.end(); ++itFileRecord)
    {
        _fileIdDict.insert(std::make_pair(itFileRecord->fileId, &(*itFileRecord)));
        
        if (itFileRecord->baseId != 0)
        {
            _baseIdDict.insert(std::make_pair(itFileRecord->baseId, &(*itFileRecord)));
        }
    }
}

std::unique_ptr<ANDatInterface>&& createANDatInterface(const char* iDatPath)
{
    std::ifstream aDatStream(iDatPath, std::ios::binary);
    auto pANDat = format::parseANDat(aDatStream, 0, 0);
    
    auto pMft = pANDat::parseMft(iDatStream, pANDat->header.mftOffset, pANDat->header.mftSize);
    auto pMapping = pANDat::parseMapping(iDatStream, pMft->entries[2].offset, pMft->entries[2].size);
    
    auto pANDatInterfaceImpl = std::unique_ptr<ANDatInterfaceImpl>(new ANDatInterfaceImpl(iDatPath, pMft, pMapping));
    pANDatInterfaceImpl->computeInternalData();
    
    return std::static_pointer_cast<ANDatInterface>(pANDatInterfaceImpl);
}

}
}
