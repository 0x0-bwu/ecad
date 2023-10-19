#pragma once
#include "ECadSettings.h"
namespace ecad {

class ILayoutView;

namespace eutils {
class ECAD_API ELayoutViewRenderer
{
public:
    explicit ELayoutViewRenderer(const ELayoutViewRendererSettings & settings);
    virtual ~ELayoutViewRenderer() = default;

    bool Renderer(CPtr<ILayoutView> layout);

private:
    bool RendererPNG(CPtr<ILayoutView> layout);

private:
    ELayoutViewRendererSettings m_settings;
};

}//namespace eutils
}//namespace ecad