#include "structure_read_model.h"

namespace atoms {
namespace application {

const std::vector<domain::AtomInfo>& StructureReadModel::GetCreatedAtoms() {
    return domain::createdAtoms;
}

const std::vector<domain::AtomInfo>& StructureReadModel::GetSurroundingAtoms() {
    return domain::surroundingAtoms;
}

const std::vector<domain::BondInfo>& StructureReadModel::GetCreatedBonds() {
    return domain::createdBonds;
}

const std::vector<domain::BondInfo>& StructureReadModel::GetSurroundingBonds() {
    return domain::surroundingBonds;
}

const std::map<std::string, domain::BondGroupInfo>& StructureReadModel::GetBondGroups() {
    return domain::bondGroups;
}

} // namespace application
} // namespace atoms
