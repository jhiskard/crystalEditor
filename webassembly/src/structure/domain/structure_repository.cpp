#include "structure_repository.h"

namespace {
atoms::domain::StructureStateStore& Store() {
    return atoms::domain::StructureStateStore::Instance();
}

const atoms::domain::StructureStateStore& StoreConst() {
    return atoms::domain::StructureStateStore::Instance();
}
} // namespace

namespace structure {
namespace domain {

StructureRepository& StructureRepository::Instance() {
    static StructureRepository repository;
    return repository;
}

atoms::domain::StructureState& StructureRepository::GetOrCreate(atoms::domain::StructureId structureId) {
    return Store().GetOrCreate(structureId);
}

const atoms::domain::StructureState* StructureRepository::Find(atoms::domain::StructureId structureId) const {
    return StoreConst().Find(structureId);
}

bool StructureRepository::Remove(atoms::domain::StructureId structureId) {
    return Store().Remove(structureId);
}

void StructureRepository::RemoveUnassigned() {
    Store().RemoveUnassigned();
}

void StructureRepository::Clear() {
    Store().Clear();
}

const std::unordered_map<atoms::domain::StructureId, atoms::domain::StructureState>&
StructureRepository::All() const {
    return StoreConst().All();
}

std::vector<atoms::domain::AtomInfo>& StructureRepository::CreatedAtoms() {
    return Store().CreatedAtoms();
}

std::vector<atoms::domain::AtomInfo>& StructureRepository::SurroundingAtoms() {
    return Store().SurroundingAtoms();
}

std::map<std::string, atoms::domain::AtomGroupInfo>& StructureRepository::AtomGroups() {
    return Store().AtomGroups();
}

std::vector<atoms::domain::BondInfo>& StructureRepository::CreatedBonds() {
    return Store().CreatedBonds();
}

std::vector<atoms::domain::BondInfo>& StructureRepository::SurroundingBonds() {
    return Store().SurroundingBonds();
}

std::map<std::string, atoms::domain::BondGroupInfo>& StructureRepository::BondGroups() {
    return Store().BondGroups();
}

CellInfo& StructureRepository::Cell() {
    return Store().Cell();
}

bool& StructureRepository::CellVisible() {
    return Store().CellVisible();
}

bool& StructureRepository::SurroundingsVisible() {
    return Store().SurroundingsVisible();
}

std::vector<vtkSmartPointer<vtkActor>>& StructureRepository::CellEdgeActors() {
    return Store().CellEdgeActors();
}

uint32_t StructureRepository::GenerateAtomId() {
    return Store().GenerateAtomId();
}

uint32_t StructureRepository::GenerateBondId() {
    return Store().GenerateBondId();
}

void StructureRepository::ResetBondIdCounter(uint32_t startId) {
    Store().ResetBondIdCounter(startId);
}

atoms::domain::AtomInfo* StructureRepository::FindAtomById(uint32_t atomId) {
    return Store().FindAtomById(atomId);
}

atoms::domain::BondInfo* StructureRepository::FindBondById(uint32_t bondId) {
    return Store().FindBondById(bondId);
}

void StructureRepository::RebuildIndexes() {
    Store().RebuildIndexes();
}

StructureRepository& GetStructureRepository() {
    return StructureRepository::Instance();
}

} // namespace domain
} // namespace structure

