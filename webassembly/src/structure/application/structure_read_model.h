#pragma once

#include "../../structure/domain/atoms/atom_manager.h"
#include "../../structure/domain/atoms/bond_manager.h"

namespace atoms {
namespace application {

class StructureReadModel {
public:
    static const std::vector<domain::AtomInfo>& GetCreatedAtoms();
    static const std::vector<domain::AtomInfo>& GetSurroundingAtoms();
    static const std::vector<domain::BondInfo>& GetCreatedBonds();
    static const std::vector<domain::BondInfo>& GetSurroundingBonds();
    static const std::map<std::string, domain::BondGroupInfo>& GetBondGroups();
};

} // namespace application
} // namespace atoms



