#include "cids.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "public.sdk/source/main/pluginfactory.h"

namespace SystemInDaw {
Steinberg::FUnknown* createProcessor(void*);
Steinberg::FUnknown* createController(void*);
}

using namespace Steinberg;
using namespace Steinberg::Vst;

BEGIN_FACTORY_DEF("趣代码科技工作室", "https://xiazaiye.cn", "")

DEF_CLASS2(INLINE_UID_FROM_FUID(SystemInDaw::ProcessorUID),
           PClassInfo::kManyInstances,
           kVstAudioEffectClass,
           "SYSTEM IN",
           Vst::kDistributable,
           "Fx|Generator",
           "0.1.0",
           kVstVersionString,
           SystemInDaw::createProcessor)

DEF_CLASS2(INLINE_UID_FROM_FUID(SystemInDaw::ControllerUID),
           PClassInfo::kManyInstances,
           kVstComponentControllerClass,
           "SYSTEM IN Controller",
           0,
           "",
           "0.1.0",
           kVstVersionString,
           SystemInDaw::createController)

END_FACTORY
