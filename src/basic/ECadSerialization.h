#pragma once
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

    #ifndef BOOST_SERIALIZATION_SUPPORT
        #define BOOST_SERIALIZATION_SUPPORT
    #endif
    #include "generic/common/Archive.hpp"
    #include "generic/geometry/Serialization.hpp"
    
    #define ECAD_SERIALIZATION_ABSTRACT_CLASS(T) BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)
    #define ECAD_SERIALIZATION_FUNCTIONS_DECLARATION                                \
    friend class boost::serialization::access;                                      \
    template <typename Archive> void serialize(Archive &, const unsigned int);      \
    /**/
    #define ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION                 \
    friend class boost::serialization::access;                                      \
    template<typename Archive> void serialize(Archive &, unsigned int) {}           \
    /**/
    #ifndef ECAD_HEADER_ONLY
        #define ECAD_SERIALIZATION_CLASS_EXPORT(T) BOOST_CLASS_EXPORT(T)
        #define ECAD_SERIALIZATION_CLASS_EXPORT_KEY(T) BOOST_CLASS_EXPORT_KEY(T)
        #define ECAD_SERIALIZATION_CLASS_EXPORT_IMP(T) BOOST_CLASS_EXPORT_IMPLEMENT(T)
        #define ECAD_SERIALIZATION_FUNCTIONS_IMP(T)                                                                         \
        template void T::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive &, const unsigned int);     \
        template void T::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive &, const unsigned int);       \
        template void T::serialize<boost::archive::binary_iarchive>(boost::archive::binary_iarchive &, const unsigned int); \
        template void T::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive &, const unsigned int);     \
        template void T::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive &, const unsigned int);       \
        template void T::serialize<boost::archive::binary_oarchive>(boost::archive::binary_oarchive &, const unsigned int); \
        /**/
    #else
        #define ECAD_SERIALIZATION_CLASS_EXPORT(T) BOOST_CLASS_EXPORT(T)
        #define ECAD_SERIALIZATION_CLASS_EXPORT_KEY(T) BOOST_CLASS_EXPORT(T)
        #define ECAD_SERIALIZATION_CLASS_EXPORT_IMP(T)
        #define ECAD_SERIALIZATION_FUNCTIONS_IMP(T)
    #endif//ECAD_HEADER_ONLY
#else
    #define ECAD_SERIALIZATION_CLASS_EXPORT(T)
    #define ECAD_SERIALIZATION_ABSTRACT_CLASS(T)
    #define ECAD_SERIALIZATION_CLASS_EXPORT_KEY(T)
    #define ECAD_SERIALIZATION_CLASS_EXPORT_IMP(T)
    #define ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    #define ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT