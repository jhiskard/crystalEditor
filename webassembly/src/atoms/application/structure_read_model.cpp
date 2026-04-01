#include "structure_read_model.h"
#include "../../structure/domain/structure_repository.h"

namespace atoms {
namespace application {

const std::vector<domain::AtomInfo>& StructureReadModel::GetCreatedAtoms() {
    return structure::domain::GetStructureRepository().CreatedAtoms();
}

const std::vector<domain::AtomInfo>& StructureReadModel::GetSurroundingAtoms() {
    return structure::domain::GetStructureRepository().SurroundingAtoms();
}

const std::vector<domain::BondInfo>& StructureReadModel::GetCreatedBonds() {
    return structure::domain::GetStructureRepository().CreatedBonds();
}

const std::vector<domain::BondInfo>& StructureReadModel::GetSurroundingBonds() {
    return structure::domain::GetStructureRepository().SurroundingBonds();
}

const std::map<std::string, domain::BondGroupInfo>& StructureReadModel::GetBondGroups() {
    return structure::domain::GetStructureRepository().BondGroups();
}

} // namespace application
} // namespace atoms
