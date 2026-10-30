#include "controller/ClinicDatabase.h"
#include "controller/SearchEngine.h"
#include "utils/Exceptions.h"
namespace scapms {

void ClinicDatabase::add(const Rec& p) {
    p->validate();
    if (SearchEngine::linearById(records_, p->id()) >= 0)
        throw ClinicException("Patient ID already exists: " + p->id());
    records_.push_back(p);
    sorted_ = SortField::None;
}

Rec ClinicDatabase::get(const QString& id) const {
    int i = SearchEngine::linearById(records_, id);
    return i >= 0 ? records_[i] : nullptr;
}

void ClinicDatabase::removeAt(int index) {
    records_.erase(records_.begin() + index);
    sorted_ = SortField::None;
}

void ClinicDatabase::insertAt(int index, const Rec& p) {
    if (index < 0 || index > static_cast<int>(records_.size())) index = static_cast<int>(records_.size());
    records_.insert(records_.begin() + index, p);
    sorted_ = SortField::None;
}

void ClinicDatabase::replace(const Rec& p) {
    int i = SearchEngine::linearById(records_, p->id());
    if (i < 0) throw ClinicException("Patient not found: " + p->id());
    records_[i] = p;
    sorted_ = SortField::None;
}
}
