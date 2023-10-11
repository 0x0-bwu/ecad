#include "EGdsFileIO.h"

#include "EGdsParser.h"
#include <fstream>
namespace ecad {

namespace ext {
namespace gds {

ECAD_INLINE EGdsReader::EGdsReader(EGdsDB & db)
 : m_db(db)
{
}

ECAD_INLINE EGdsReader::~EGdsReader()
{
}

ECAD_INLINE bool EGdsReader::operator() (const std::string & filename)
{
    // calculate file size 
    std::ifstream in (filename.c_str());
    if (!in.good()) return false;
    std::streampos begin = in.tellg();
    in.seekg(0, std::ios::end);
    std::streampos end = in.tellg();
    m_fileSize = (end-begin);
    in.close();
	// reset temporary data 
    m_status = EGdsRecords::UNKNOWN;
    Reset();
	m_unsupportRecords.assign(EGdsRecords::UNKNOWN, 0); 
    // read gds 
    bool res = EGdsParser(*this)(filename);
	PrintUnsupportedRecords();
	return res; 
}

ECAD_INLINE void EGdsReader::Reset()
{
    m_string.clear();
    m_sname.clear(); 
    m_layer = -1;
    m_dataType = -1;
    m_pathType = -1;
    m_textType = -1;
    m_spacing[0] = 0;
    m_spacing[1] = 0;
    m_width = 0;
    m_columns = 0;
    m_rows = 0;
    m_angle = 0;
    m_magnification = 1;
    m_strans = -1;
    m_presentation = -1;
    m_points.clear();
}

ECAD_INLINE void EGdsReader::PrintUnsupportedRecords()
{
    //todo
}

ECAD_INLINE void EGdsReader::ReadBitArray(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::vector<int> & data)
{
    ReadInteger(recordType, dataType, data);
}

ECAD_INLINE void EGdsReader::ReadInteger2(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::vector<int> & data)
{
    ReadInteger(recordType, dataType, data);
}

ECAD_INLINE void EGdsReader::ReadInteger4(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::vector<int> & data)
{
    ReadInteger(recordType, dataType, data);   
}

ECAD_INLINE void EGdsReader::ReadInteger(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::vector<int> & data)
{
    switch (recordType)
    {
		case EGdsRecords::HEADER : 
        	ECAD_ASSERT(m_status == EGdsRecords::UNKNOWN)
			m_db.SetHeader(data.front()); 
			break;
        case EGdsRecords::LAYER :
            m_layer = data.front();
            break;
		case EGdsRecords::DATATYPE :
			m_dataType = data.front();
			break; 
		case EGdsRecords::PATHTYPE :
			m_pathType = data.front();
			break; 
		case EGdsRecords::TEXTTYPE :
			m_textType = data.front();
			break; 
		case EGdsRecords::SPACING :
			m_spacing[0] = data.at(0);
			m_spacing[1] = data.at(1);
			break; 
		case EGdsRecords::WIDTH :
			m_width = data.front();
			break; 
		case EGdsRecords::STRANS :
			m_strans = data.front();
			break; 
		case EGdsRecords::PRESENTATION :
			m_presentation = data.front();
			break; 
		case EGdsRecords::COLROW :
			m_columns = data.at(0);
			m_rows = data.at(1);
			break; 
        case EGdsRecords::XY :
			{
				for (size_t i = 0, ie = data.size(); i < ie; i += 2)
					m_points.push_back(EPoint2D(data[i], data[i+1])); 
			}
			break;
        case EGdsRecords::BGNLIB: // notify database on the begin of lib 
			m_status = EGdsRecords::BGNLIB;
            break;
        case EGdsRecords::BGNSTR: // just date of creation, not interesting
			m_status = EGdsRecords::BGNSTR; 
            break;
		case EGdsRecords::BGNEXTN: // appear in PATH with PATHTYPE 4, not really useful though 
		case EGdsRecords::ENDEXTN:
        default: // other not interested record_type
			{
				// only print invalid records or unsupported records for the first time 
				// if (record_type >= (int)m_vUnsupportRecord.size() || m_vUnsupportRecord[record_type] == 0)
					// limboPrint(limbo::kWARN, "%s() invalid record_type = %s, data_type = %s\n", __func__, ::GdsParser::gds_record_ascii(record_type), ::GdsParser::gds_data_ascii(data_type));
				//todo
                m_unsupportRecords[recordType] += 1; 
			}
            break;
    }
}

ECAD_INLINE void EGdsReader::ReadReal4(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::vector<double> & data)
{
    ReadFloat(recordType, dataType, data);
}

ECAD_INLINE void EGdsReader::ReadReal8(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::vector<double> & data)
{
    ReadFloat(recordType, dataType, data);
}

ECAD_INLINE void EGdsReader::ReadFloat(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::vector<double> & data)
{
    switch (recordType)
    {
        case EGdsRecords::UNITS:
			m_db.SetUnit(data.at(0));
			m_db.SetPrecision(data.at(1));
            break;
		case EGdsRecords::ANGLE:
			m_angle = data.front();
			break; 
		case EGdsRecords::MAG:
			m_magnification = data.front();
			break; 
        default:
			{
				// only print invalid records or unsupported records for the first time 
				// if (record_type >= (int)m_vUnsupportRecord.size() || m_vUnsupportRecord[record_type] == 0)
					// limboPrint(limbo::kWARN, "%s() invalid record_type = %s, data_type = %s\n", __func__, ::GdsParser::gds_record_ascii(record_type), ::GdsParser::gds_data_ascii(data_type));
				//todo
                m_unsupportRecords[recordType] += 1; 
			}
            break;
    }
}

ECAD_INLINE void EGdsReader::ReadString(const EGdsRecords::EnumType & recordType, const EGdsData::EnumType & dataType, const std::string & data)
{
    ECAD_ASSERT(dataType == EGdsData::STRING)
    switch (recordType)
    {
		case EGdsRecords::HEADER : 
			ECAD_ASSERT(m_status == EGdsRecords::UNKNOWN)
			m_db.SetHeader(data); 
			break ;
        case EGdsRecords::STRNAME : // a new cell begins 
			ECAD_ASSERT(m_status == EGdsRecords::BGNSTR) // in a cell 
			m_db.AddCell(data); 
            break;
        case EGdsRecords::LIBNAME : // a library 
			ECAD_ASSERT(m_status == EGdsRecords::BGNLIB) // in a lib 
			m_db.SetLibName(data); 
			break; 
        case EGdsRecords::STRING :
			ECAD_ASSERT(m_status == EGdsRecords::TEXT) // in a text 
			m_string = data; 
			break; 
        case EGdsRecords::SNAME :
			m_sname = data; 
			break; 
        default: 
			{
				// only print invalid records or unsupported records for the first time
                //todo
				m_unsupportRecords[recordType] += 1; 
			}
			break;
    }   
}

ECAD_INLINE void EGdsReader::ReadBeginEnd(const EGdsRecords::EnumType & recordType)
{
    switch (recordType)
    {
        case EGdsRecords::BOX :
        case EGdsRecords::BOUNDARY :
        case EGdsRecords::PATH :
		case EGdsRecords::TEXT :
		case EGdsRecords::SREF :
		case EGdsRecords::AREF :
            m_status = recordType;
            break;
        case EGdsRecords::ENDEL :
            {
                m_db.Layers().insert(m_layer);
                switch (m_status)
                {
                    case EGdsRecords::BOX :
                    case EGdsRecords::BOUNDARY :
                        ECAD_ASSERT(m_layer != -1)
						m_db.Cells().back().AddPolygon(m_layer, m_dataType, m_points); 
						break; 
                    case EGdsRecords::PATH :
                        ECAD_ASSERT(m_layer != -1)
						m_db.Cells().back().AddPath(m_layer, m_dataType, m_pathType, m_width, m_points); 
                        break;
					case EGdsRecords::TEXT :
                        ECAD_ASSERT(m_layer != -1 && !m_string.empty())
						m_db.Cells().back().AddText(m_layer, m_dataType, m_textType, m_string, m_points.front(), m_width, m_presentation, m_angle, m_magnification, m_strans); 
						break; 
                    case EGdsRecords::SREF :
						m_db.Cells().back().AddCellReference(m_sname, m_points.front(), m_angle, m_magnification, m_strans); 
                        break;
                    case EGdsRecords::AREF :
						m_db.Cells().back().AddCellRefArray(m_sname, m_columns, m_rows, m_spacing, m_points, m_angle, m_magnification, m_strans); 
                        break;
                    default: break;
                }
                m_status = EGdsRecords::BGNSTR; // go back to upper level 
				// reset temporary data 
				Reset();
            }
            break;
        case EGdsRecords::ENDLIB : // notify database on the end of lib 
			m_status = EGdsRecords::UNKNOWN; // go back to upper level  
            break;
        case EGdsRecords::ENDSTR : // currently not interested, add stuff here if needed 
			m_status = EGdsRecords::BGNLIB; // go back to upper level 
            break;
        default : // be careful here, you may dump a lot of unnecessary error message for unknown record_type 
			{
				// only print invalid records or unsupported records for the first time
                //todo
				m_unsupportRecords[recordType] += 1; 
			}
            break;
    }   
}

}//namespace gds   
}//namespace ext
}//namespace ecad
