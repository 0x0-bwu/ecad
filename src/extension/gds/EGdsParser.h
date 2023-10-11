#pragma once
#include "EGdsObjects.h"
#include "EGdsRecords.h"
#include <istream>

#define ECAD_EXT_GDS_NO_SPACES_TO_INDENT 2
namespace ecad {
namespace ext {
namespace gds {

class EGdsReader;
class ECAD_API EGdsParser
{
public:
    EGdsParser(EGdsReader & reader);
    virtual ~EGdsParser();

    bool operator() (const std::string & filename);
    bool operator() (std::istream & fp);

protected:
    CPtr<char> Parse(std::istream & fp, int & noRead, size_t n);
    void FindRecordType(int numeric, EGdsRecords::EnumType & recordType, int & expectedDataType);
    void FindDataType(int numeric, EGdsData::EnumType & dataType);

protected:
    EGdsReader & m_reader;
    Ptr<char> m_buffer;
    Ptr<char> m_bptr;
    size_t m_bcap;//buffer capacity
    size_t m_blen;//current buffer size, from m_bptr to m_buffer + m_bcap
};

}//namespace gds   
}//namespace ext
}//namespace ecad