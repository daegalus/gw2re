#ifndef GW2DATTOOLS_FORMATS_UTILS_H
#define GW2DATTOOLS_FORMATS_UTILS_H

namespace gw2dt
{
namespace formats
{

template <typename Struct>
void readStruct(std::istream& iStream, Struct& iStruct)
{
    iStream.read(reinterpret_cast<char*>(&iStruct), sizeof(Struct));
}

template <typename Struct>
void readNStruct(std::istream& iStream, Struct& iStruct, const uint32_t iNum)
{
    iStream.read(reinterpret_cast<char*>(&iStruct), sizeof(Struct) * iNum);
}

template <typename Struct>
void readStruct<std::vector<Struct>>(std::istream& iStream, std::vector<Struct>& iStructVect)
{
    iStream.read(reinterpret_cast<char*>(iStructVect.data()), sizeof(Struct) * iStructVect.size());
}


}
}

#endif // GW2DATTOOLS_FORMATS_UTILS_H
