#pragma once
#include "EGdsObjects.h"
#include "EGdsRecords.h"
namespace ecad {

namespace ext {
namespace gds {

class ECAD_API EGdsReader
{
public:
    EGdsReader(EGdsDB & db);
    virtual ~EGdsReader();

    virtual bool operator() (const std::string & filename);

    virtual void ReadBitArray(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::vector<int> & data);
    virtual void ReadInteger2(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::vector<int> & data);
    virtual void ReadInteger4(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::vector<int> & data);
    virtual void ReadInteger(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::vector<int> & data);
    virtual void ReadReal4(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::vector<double> & data);
    virtual void ReadReal8(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::vector<double> & data);
    virtual void ReadFloat(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::vector<double> & data);
    virtual void ReadString(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::string & data);
    virtual void ReadBeginEnd(const EGdsRecords::EnumType & recordType);
protected:
    virtual void Reset();
    virtual void PrintUnsupportedRecords();

protected:
    EGdsDB & m_db;
    size_t m_fileSize;
    EGdsRecords::EnumType m_status;
	std::vector<size_t> m_unsupportRecords;//record the times of unsupported records 
protected:
    // temporary data
    std::string m_string; ///< STRING 
    std::string m_sname;  ///< SNAME 
    int m_layer; ///< LAYER 
    int m_dataType; ///< DATATYPE
    int m_pathType; ///< PATHTYPE
    int m_textType; ///< TEXTTYPE
    int m_spacing[2]; ///< SPACING 
    int m_width; ///< WIDTH 
    int m_columns; ///< COLROW, number of columns 
    int m_rows; ///< COLROW, number of rows 
    double m_angle; ///< ANGLE 
    double m_magnification; ///< MAG 
    int m_strans; ///< STRANS
    int m_presentation; ///< PRESENTATION
    std::vector<EPoint2D> m_points; ///< XY
};

}//namespace gds   
}//namespace ext
}//namespace ecad