#pragma once
#include "interface/IMaterialDefCollection.h"
#include "interface/IDefinitionCollection.h"
#include "interface/IIterator.h"
#include "basic/ECollection.h"
namespace ecad {

class IMaterialDef;
using EMaterialDefIterator = EUnorderedMapCollectionIterator<std::string, UPtr<IMaterialDef> >;
class ECAD_API EMaterialDefCollection
 : public EUnorderedMapCollection<std::string, UPtr<IMaterialDef> >
 , public IMaterialDefCollection
 , public IDefinitionCollection
{
    using BaseCollection = EUnorderedMapCollection<std::string, UPtr<IMaterialDef> >;
    using BaseCollection::CollectionContainer;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EMaterialDefCollection();
    virtual ~EMaterialDefCollection();

    ///Copy
    EMaterialDefCollection(const EMaterialDefCollection & other);
    EMaterialDefCollection & operator= (const EMaterialDefCollection & other);

    Ptr<IDefinition> AddDefinition(const std::string & name, UPtr<IDefinition> definition) override;
    Ptr<IDefinition> GetDefinition(const std::string & name, EDefinitionType type) const override;
    std::string GetNextDefName(const std::string & base, EDefinitionType type) const override;

    Ptr<IMaterialDef> FindMaterialDefById(EMaterialId id) const override;
    void SetDatabase(CPtr<IDatabase> database) override;
    MaterialDefIter GetMaterialDefIter() const override;
    size_t Size() const override;
    void Clear() override;

protected:
    virtual ECollectionType GetType() const override { return ECollectionType::MaterialDef; }
    virtual Ptr<EMaterialDefCollection> CloneImp() const override { return new EMaterialDefCollection(*this); }
    virtual void PrintImp(std::ostream & os) const override;

private:
    std::unordered_map<EMaterialId, Ptr<IMaterialDef> > m_idLut;
};
}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EMaterialDefCollection)