#include "EGdsParser.h"

#include "extension/gds/EGdsFileIO.h"
#include <fstream>
#include <cstring>
namespace ecad {

namespace ext {
namespace gds {

ECAD_INLINE EGdsParser::EGdsParser(EGdsReader & reader)
 : m_reader(reader)
{
    m_bcap = 4 * 1024;// 4KB
    m_blen = 0;
    m_buffer = new char[m_bcap];
    m_bptr = m_buffer;
}

ECAD_INLINE EGdsParser::~EGdsParser()
{
}

ECAD_INLINE bool EGdsParser::operator() (const std::string & filename)
{
    std::ifstream fp(filename.c_str());
    if(!fp.good()){
        //todo, report error
        return false;
    }

    bool res = (*this)(fp);
    fp.close();
    return res;
}

ECAD_INLINE bool EGdsParser::operator() (std::istream & fp)
{
	unsigned char * noByteArray;
	int noRead;
	int noBytes;
	unsigned char * record{nullptr};
    int indentAmount;
	int recordType;
	int dataType;
	int expectedDataType;
    EGdsRecords::EnumType enumRecordType;
    EGdsData::EnumType enumDataType;
    [[maybe_unused]] EGdsData::EnumType enumExpectedDataType;
	int intKtr;
	int dataKtr;
	int exponentKtr;
	[[maybe_unused]] int corruptKtr;
	unsigned int displayInteger;
#ifdef ECAD_EXT_GDS_DEBUG_MODE
	unsigned int hexDisplayInteger;
#endif 
	char displayChar1;
	char displayChar2;
	int bitArray;
	int realSign;
	int realExponent;
	unsigned long long realMantissaInt;
	double realMantissaFloat;
	double displayFloat;

	/* start out with no indent */
    indentAmount = 0;

    while (1){
        noByteArray = (unsigned char*)Parse(fp, noRead, 2);
        if(noRead != 2){
            //Error: reached the end of the file!
            if(noRead == 1 && noByteArray[0] != 0){
                //Error: read a single non-zero byte after the last record!
            }
            break;
        }
        noBytes = noByteArray[0] * 256 + noByteArray[1];

        if(noBytes != 0){
            record = (unsigned char*)Parse(fp, noRead, noBytes - 2);

            if(noRead != noBytes - 2){
                //Error: Couldn't read all of record!
                //Error: It should have had %1% bytes, could only read %2% of them!, noBytes, noRead + 2
                //Error: It's a corrupt file!
                break;
            }
        }

        recordType = record[0];
        FindRecordType(recordType, enumRecordType, expectedDataType);

        dataType = record[1];
        FindDataType(dataType, enumDataType);

        /* if it's a ENDSTR or ENDEL, subtract from indent */
        if(enumRecordType == EGdsRecords::ENDSTR ||
            enumRecordType == EGdsRecords::ENDEL){
            if(indentAmount >= 2)
                indentAmount -= 2;
        }

        if(expectedDataType != 0xffff && expectedDataType != dataType){
            ///Error, todo
        }

        if (expectedDataType == EGdsData::BIT_ARRAY){
            std::vector<int> vBitArray;
            vBitArray.reserve((noRead - 2) >> 1);
            for(dataKtr = 2; dataKtr < noRead; dataKtr += 2){
                /* use bit shifting instread of multiplication */
                bitArray = (record[dataKtr] << 8) + record[dataKtr + 1];
                vBitArray.push_back(bitArray);
            }
            m_reader.ReadBitArray(enumRecordType, enumDataType, vBitArray);
        }
        else if(expectedDataType == EGdsData::INTEGER_2){
            std::vector<int> vInteger;
            vInteger.reserve((noRead - 2) >> 1);
            for(dataKtr = 2; dataKtr < noRead; dataKtr += 2){
                displayInteger = record[dataKtr];
                displayInteger <<= 8;
                displayInteger += record[dataKtr + 1];
                if (displayInteger & 0x8000){	/* negative number, 2's comp */
                    displayInteger &= 0x7fff;
                    displayInteger ^= 0x7fff;
                    displayInteger +=  1;
                    displayInteger *= -1;
                } 
                vInteger.push_back(displayInteger);
            }
            m_reader.ReadInteger2(enumRecordType, enumDataType, vInteger);
        }
		else if (expectedDataType == EGdsData::INTEGER_4){
            std::vector<int> vInteger;
            vInteger.reserve((noRead - 2) >> 1);
            for(dataKtr = 2; dataKtr < noRead; dataKtr += 4){
                displayInteger = 0;
                for(intKtr = 0; intKtr < 4; intKtr++){
                    displayInteger <<= 8;
                    displayInteger += record[dataKtr + intKtr];
                }
                if(displayInteger & 0x80000000){	/* negative number, 2's comp */
                    displayInteger &= 0x7fffffff;
                    displayInteger ^= 0x7fffffff;
                    displayInteger +=  1;
                    displayInteger *= -1;
                }
                vInteger.push_back(displayInteger);
            }
            m_reader.ReadInteger4(enumRecordType, enumDataType, vInteger);
        }
        else if(expectedDataType == EGdsData::REAL_4){
            std::vector<double> vFloat;
            vFloat.reserve((noRead - 2) >> 2);
            for (dataKtr = 2; dataKtr < noRead; dataKtr += 4){
                realSign = record[dataKtr] & 0x80;
                realExponent = (record[dataKtr] & 0x7f) - 64;
                realMantissaInt = 0;
                for (exponentKtr = 1; exponentKtr < 4; exponentKtr++){
                    realMantissaInt <<= 8;
                    realMantissaInt += record[dataKtr + exponentKtr];
                }
                realMantissaFloat = (double)realMantissaInt / std::pow(2, 24);
                displayFloat = realMantissaFloat * std::pow(16, (float)realExponent);
                if (realSign) displayFloat *= -1;
                vFloat.push_back(displayFloat);
            }
            m_reader.ReadReal4(enumRecordType, enumDataType, vFloat);
        }
        else if (expectedDataType == EGdsData::REAL_8){
            std::vector<double> vFloat;
            vFloat.reserve((noRead - 2) >> 3);
            for (dataKtr = 2; dataKtr < noRead; dataKtr += 8){
                realSign = record[dataKtr] & 0x80;
                realExponent = (record[dataKtr] & 0x7f) - 64;
                realMantissaInt = 0;
                for (exponentKtr = 1; exponentKtr < 8; exponentKtr++){
                    realMantissaInt <<= 8;
                    realMantissaInt += record[dataKtr + exponentKtr];
                }
                realMantissaFloat = (double)realMantissaInt / std::pow(2, 56);
                displayFloat = realMantissaFloat * std::pow(16, (float)realExponent);
                if (realSign) displayFloat *= -1;
                vFloat.push_back(displayFloat);
            }
            m_reader.ReadReal8(enumRecordType, enumDataType, vFloat);
        }
        else if (expectedDataType == EGdsData::STRING){
            std::string str;
            str.reserve((noRead - 2) >> 1); 
            for (dataKtr = 2; dataKtr < noRead; dataKtr += 2)
            {
                displayChar1 = record[dataKtr];
                displayChar2 = record[dataKtr + 1];

                if (displayChar1 == '\0') break; /* quit early if encounter null character */
                else if (!std::isprint (displayChar1)){
                    displayChar1 = '.';
                }
                str.push_back(displayChar1);

                if (displayChar2 == '\0') break; /* quit early if encounter null character */
                else if (!std::isprint (displayChar2)){
                    displayChar2 = '.';
                }
                str.push_back(displayChar2);
            }
            m_reader.ReadString(enumRecordType, enumDataType, str);
        }
        else
        {
            if (expectedDataType != EGdsData::NO_DATA){
#ifdef ECAD_EXT_GDS_DEBUG_MODE
                for (dataKtr = 2; dataKtr < noRead; dataKtr++){
                    //"Error: 0x%02x # RAW(UNKNOWN)\n", record[dataKtr]);
                }
#endif 
            }
            else m_reader.ReadBeginEnd(enumRecordType); 
        }

        /* if it's a BGNSTR or the beginning of an element, add to indent */
        if( enumRecordType == EGdsRecords::BGNSTR ||
            enumRecordType == EGdsRecords::BOUNDARY ||
            enumRecordType == EGdsRecords::PATH ||
            enumRecordType == EGdsRecords::SREF ||
            enumRecordType == EGdsRecords::AREF ||
            enumRecordType == EGdsRecords::TEXT ||
            enumRecordType == EGdsRecords::TEXTNODE ||
            enumRecordType == EGdsRecords::NODE ||
            enumRecordType == EGdsRecords::BOX ){
            indentAmount += ECAD_EXT_GDS_NO_SPACES_TO_INDENT;
        }
		else
		{
#ifdef ECAD_EXT_GDS_DEBUG_MODE
			/* if it was a NULL record */
#endif 
		}
	}

	return true;
}

ECAD_INLINE const char * EGdsParser::Parse(std::istream & fp, int & noRead, size_t n)
{
    if(m_blen < n){
        if(m_bcap < n * 2){
            while(m_bcap < n){
                m_bcap *= 2;
            }

            char * buffer = new char[m_bcap];
            if(m_blen > 0){
                std::memcpy(buffer, m_bptr, m_blen);
            }
            delete [] m_buffer;
            m_buffer = buffer;
        }
        else if(m_blen > 0){
            std::memmove(m_buffer, m_bptr, m_blen);
        }

        fp.read(m_buffer + m_blen, m_bcap - m_blen);
        int numBytes = fp.gcount();
        if(numBytes < 0){
            noRead = 0;
            return nullptr;
        }
        m_blen += numBytes;
        m_bptr = m_buffer;
    }

    if(m_blen >= n){
        const char * res = m_bptr;
        m_bptr += n;
        m_blen -= n;
        noRead = n;
        return res;
    }
    else{
        noRead = 0;
        return nullptr;
    }
}

ECAD_INLINE void EGdsParser::FindRecordType(int numeric, EGdsRecords::EnumType & recordType, int & expectedDataType)
{
    recordType = GdsRecordType(numeric);
    expectedDataType = GdsRecordExpectedData(recordType);
}

ECAD_INLINE void EGdsParser::FindDataType(int numeric, EGdsData::EnumType & dataType)
{
    dataType = GdsDataType(numeric);
}

}//namespace gds   
}//namespace ext
}//namespace ecad
