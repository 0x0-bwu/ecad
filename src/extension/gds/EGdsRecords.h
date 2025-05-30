#pragma once
#include "basic/ECadCommon.h"

namespace ecad {
namespace ext{
namespace gds{

struct EGdsRecords 
{
    enum EnumType {
        HEADER = 0x00, 
        BGNLIB = 0x01, 
        LIBNAME = 0x02, 
        UNITS = 0x03, 
        ENDLIB = 0x04, 
        BGNSTR = 0x05, 
        STRNAME = 0x06, 
        ENDSTR = 0x07, 
        BOUNDARY = 0x08, 
        PATH = 0x09, 
        SREF = 0x0a, 
        AREF = 0x0b, 
        TEXT = 0x0c, 
        LAYER = 0x0d, 
        DATATYPE = 0x0e, 
        WIDTH = 0x0f, 
        XY = 0x10, 
        ENDEL = 0x11, 
        SNAME = 0x12, 
        COLROW = 0x13, 
        TEXTNODE = 0x14, 
        NODE = 0x15, 
        TEXTTYPE = 0x16, 
        PRESENTATION = 0x17, 
        SPACING = 0x18, 
        STRING = 0x19, 
        STRANS = 0x1a, 
        MAG = 0x1b, 
        ANGLE = 0x1c, 
        UINTEGER = 0x1d, 
        USTRING = 0x1e, 
        REFLIBS = 0x1f, 
        FONTS = 0x20, 
        PATHTYPE = 0x21, 
        GENERATIONS = 0x22, 
        ATTRTABLE = 0x23, 
        STYPTABLE = 0x24, 
        STRTYPE = 0x25, 
        ELFLAGS = 0x26, 
        ELKEY = 0x27, 
        LINKTYPE = 0x28, 
        LINKKEYS = 0x29, 
        NODETYPE = 0x2a, 
        PROPATTR = 0x2b, 
        PROPVALUE = 0x2c, 
        BOX = 0x2d, 
        BOXTYPE = 0x2e, 
        PLEX = 0x2f, 
        BGNEXTN = 0x30, 
        ENDEXTN = 0x31,
        TAPENUM = 0x32, 
        TAPECODE = 0x33, 
        STRCLASS = 0x34, 
        RESERVED = 0x35, 
        FORMAT = 0x36, 
        MASK = 0x37, 
        ENDMASKS = 0x38, 
        LIBDIRSIZE = 0x39, 
        SRFNAME = 0x3a, 
        LIBSECUR = 0x3b, 
        UNKNOWN = 0x3c /* unknown is set to the total number of records */
    };
};

[[maybe_unused]] 
static const char * eGdsRecordsAscii[] = {
        "HEADER", 
        "BGNLIB", 
        "LIBNAME", 
        "UNITS", 
        "ENDLIB", 
        "BGNSTR", 
        "STRNAME", 
        "ENDSTR", 
        "BOUNDARY", 
        "PATH", 
        "SREF", 
        "AREF", 
        "TEXT", 
        "LAYER", 
        "DATATYPE", 
        "WIDTH", 
        "XY", 
        "ENDEL", 
        "SNAME", 
        "COLROW", 
        "TEXTNODE", 
        "NODE", 
        "TEXTTYPE", 
        "PRESENTATION", 
        "SPACING", 
        "STRING", 
        "STRANS", 
        "MAG", 
        "ANGLE", 
        "UINTEGER", 
        "USTRING", 
        "REFLIBS", 
        "FONTS", 
        "PATHTYPE", 
        "GENERATIONS", 
        "ATTRTABLE", 
        "STYPTABLE", 
        "STRTYPE", 
        "ELFLAGS", 
        "ELKEY", 
        "LINKTYPE", 
        "LINKKEYS", 
        "NODETYPE", 
        "PROPATTR", 
        "PROPVALUE", 
        "BOX", 
        "BOXTYPE", 
        "PLEX", 
        "BGNEXTN", 
        "ENDEXTN",
        "TAPENUM", 
        "TAPECODE", 
        "STRCLASS", 
        "RESERVED", 
        "FORMAT", 
        "MASK", 
        "ENDMASKS", 
        "LIBDIRSIZE", 
        "SRFNAME", 
        "LIBSECUR", 
        "UNKNOWN"
};

[[maybe_unused]]
static const char* eGdsRecordsDescription[] = {
    "Start of stream, contains version number of stream file", 
    "Beginning of library, plus mod and access dates", 
    "The name of the library", 
    "Size of db unit in user units and size of db unit in meters", 
    "End of the library", 
    "Begin structure, plus create and mod dates", 
    "Name of a structure", 
    "End of a structure", 
    "The beginning of a BOUNDARY element", 
    "The beginning of a PATH element", 
    "The beginning of an SREF element", 
    "The beginning of an AREF element", 
    "The beginning of a TEXT element", 
    "Layer specification", 
    "Datatype specification", 
    "Width specification, negative means absolute", 
    "An array of XY coordinates", 
    "The end of an element", 
    "The name of a referenced structure", 
    "Columns and rows for an AREF", 
    "\"Not currently used\" per GDSII Stream Format Manual, Release 6.0",
    "The beginning of a NODE element", 
    "Texttype specification", 
    "Text origin and font specification", 
    "\"Discontinued\" per GDSII Stream Format Manual, Release 6.0",
    "Character string", 
    "Refl, absmag, and absangle for SREF, AREF and TEXT", 
    "Magnification, 1 is the default", 
    "Angular rotation factor", 
    "User integer, used only in V2.0, translates to userprop 126 on instream", 
    "User string, used only in V2.0, translates to userprop 127 on instream", 
    "Names of the reference libraries", 
    "Names of the textfont definition files", 
    "Type of path ends", 
    "Number of deleted or backed up structures to retain", 
    "Name of the attribute definition file", 
    "\"Unreleased feature\" per GDSII Stream Format Manual, Release 6.0", 
    "\"Unreleased feature\" per GDSII Stream Format Manual, Release 6.0", 
    "Flags for template and exterior data", 
    "\"Unreleased feature\" per GDSII Stream Format Manual, Release 6.0", 
    "\"Unreleased feature\" per GDSII Stream Format Manual, Release 6.0", 
    "\"Unreleased feature\" per GDSII Stream Format Manual, Release 6.0", 
    "Nodetype specification", 
    "Property number", 
    "Property value", 
    "The beginning of a BOX element", 
    "Boxtype specification", 
    "Plex number and plexhead flag", 
    "Path extension beginning for pathtype 4 in CustomPlus", 
    "Path extension end for pathtype 4 in CustomPlus", 
    "Tape number for multi-reel stream file, you've got a really old file here", 
    "Tape code to verify that you've loaded a reel from the proper set", 
    "Calma use only, non-Calma programs should not use, or set to all 0", 
    "Used to be NUMTYPES per GDSII Stream Format Manual, Release 6.0", 
    "Archive or Filtered flag", 
    "Only in filtered streams, lists layer and datatype mask used", 
    "The end of mask descriptions", 
    "Number of pages in library director, a GDSII thing...", 
    "Sticks rule file name", 
    "Access control list stuff for CalmaDOS, ancient!", 
    "***ERROR*** Unknown record type type" 
};

struct EGdsData 
{
    /// @brief enum type of data type 
    enum EnumType {
        NO_DATA = 0x00, 
        BIT_ARRAY = 0x01, 
        INTEGER_2 = 0x02, 
        INTEGER_4 = 0x03, 
        REAL_4 = 0x04, 
        REAL_8 = 0x05, 
        STRING = 0x06, 
        UNKNOWN = 0x07 
    };
};

/// @brief array to map from enum of GDSII data type to ASCII 
[[maybe_unused]] static const char* eGdsDataAscii[] = {
    "NO_DATA", 
    "BIT_ARRAY", 
    "INTEGER_2", 
    "INTEGER_4", 
    "REAL_4", 
    "REAL_8", 
    "STRING", 
    "UNKNOWN" 
};

/// @brief array to map from enum of GDSII data type to descriptions 
[[maybe_unused]] static const char* eGdsDataDescription[] = {
    "No data present (nothing after the record header)",
    "Bit array (2 bytes)",
    "Two byte signed integer",
    "Four byte signed integer",
    "Four byte real (not used?)",
    "Eight byte real",
    "ASCII string (padded to an even byte count with NULL)",
    "UNKNOWN"
};

static const int eGdsRecordsExpectData[] = {
    0x02,
    0x02,
    0x06,
    0x05,
    0x00,
    0x02,
    0x06,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x02,
    0x02,
    0x03,
    0x03,
    0x00,
    0x06,
    0x02,
    0x00,
    0x00,
    0x02,
    0x01,
    0xffff,
    0x06,
    0x01,
    0x05,
    0x05,
    0xffff,
    0xffff,
    0x06,
    0x06,
    0x02,
    0x02,
    0x06,
    0x06,
    0x02,
    0x01,
    0x03,
    0xffff,
    0xffff,
    0x02,
    0x02,
    0x06,
    0x00,
    0x02,
    0x03,
    0x03,
    0x03,
    0x02,
    0x02,
    0x01,
    0x03,
    0x02,
    0x06,
    0x00,
    0x02,
    0x06,
    0x02,
    0xffff
};

ECAD_ALWAYS_INLINE int GdsRecordExpectedData(int recordType)
{
    return eGdsRecordsExpectData[recordType];
}

ECAD_ALWAYS_INLINE EGdsRecords::EnumType GdsRecordType(int numeric)
{
    if(numeric > EGdsRecords::UNKNOWN)
        numeric = static_cast<int>(EGdsRecords::UNKNOWN);
    return static_cast<EGdsRecords::EnumType>(numeric);
}

ECAD_ALWAYS_INLINE EGdsData::EnumType GdsDataType(int numeric)
{
    if(numeric > EGdsData::UNKNOWN)
        numeric = static_cast<int>(EGdsData::UNKNOWN);
    return static_cast<EGdsData::EnumType>(numeric);
}

}//namespace gds
}//namespace ext
}//namespace ecad