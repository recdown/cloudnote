#include "cids.h"
#include "public.sdk/source/vst/vsteditcontroller.h"

namespace SystemInDaw {
class Controller final : public Steinberg::Vst::EditController {};
Steinberg::FUnknown* createController(void*) {
    return static_cast<Steinberg::Vst::IEditController*>(new Controller);
}
}
