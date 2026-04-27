#include "legacy_atoms_runtime.h"

namespace workspace {
namespace legacy {

AtomsTemplate& LegacyAtomsRuntime() {
    static AtomsTemplate runtime;
    return runtime;
}

const AtomsTemplate& LegacyAtomsRuntimeConst() {
    return LegacyAtomsRuntime();
}

} // namespace legacy
} // namespace workspace
