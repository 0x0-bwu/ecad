#include "EStackupPrismaThermalModel.h"
#include "models/thermal/utils/EStackupPrismaThermalModelQuery.h"
#include "models/geometry/ELayerCutModel.h"


namespace ecad::model {

ECAD_INLINE EStackupPrismaThermalModel::EStackupPrismaThermalModel(CPtr<ILayoutView> layout)
 : EPrismaThermalModel(layout)
{
}

} //namespace ecad::model