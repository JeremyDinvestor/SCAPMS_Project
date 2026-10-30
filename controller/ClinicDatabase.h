#pragma once
#include <vector>
#include "model/PatientRecord.h"
namespace scapms {

class ClinicDatabase {
public:
    void add(const Rec& p);
    Rec get(const QString& id) const;
    void removeAt(int index);
    void insertAt(int index, const Rec& p);
    void replace(const Rec& p);
    std::vector<Rec>& records() { return records_; }
    const std::vector<Rec>& records() const { return records_; }
    SortField sortedBy() const { return sorted_; }
    void setSortedBy(SortField f) { sorted_ = f; }
private:
    std::vector<Rec> records_;
    SortField sorted_ = SortField::None;
};
}
