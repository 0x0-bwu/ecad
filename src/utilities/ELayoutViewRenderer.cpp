#include "ELayoutViewRenderer.h"

#include "generic/tools/Color.hpp"

#include "EDataMgr.h"
#include "EShape.h"

namespace ecad {
namespace eutils {
using namespace generic::geometry;
ECAD_INLINE ELayoutViewRenderer::ELayoutViewRenderer(const ELayoutViewRendererSettings & settings)
 : m_settings(settings)
{
}

ECAD_INLINE bool ELayoutViewRenderer::Renderer(CPtr<ILayoutView> layout)
{
    switch (m_settings.format) {
        case ELayoutViewRendererSettings::Format::PNG :
            return RendererPNG(layout);
        default :
            return false;
    }
}

ECAD_INLINE bool ELayoutViewRenderer::RendererPNG(CPtr<ILayoutView> layout)
{   
    //todo
    return false;
}

}//namespace eutils
}//namespace ecad