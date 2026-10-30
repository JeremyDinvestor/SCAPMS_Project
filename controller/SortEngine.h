#pragma once
#include <functional>
#include <vector>
#include "model/PatientRecord.h"
namespace scapms {

class SortEngine {
public:
    using Less = std::function<bool(const Rec&, const Rec&)>;

    static Less lessFor(SortField f) {
        switch (f) {
        case SortField::Id:
            return [](const Rec& a, const Rec& b) { return a->id().toLower() < b->id().toLower(); };
        case SortField::Age:
            return [](const Rec& a, const Rec& b) { return a->age() < b->age(); };
        case SortField::AppointmentDate:
            return [](const Rec& a, const Rec& b) { return a->nextPendingDate() < b->nextPendingDate(); };
        default:
            return [](const Rec& a, const Rec& b) { return a->name().toLower() < b->name().toLower(); };
        }
    }

    static void insertionSort(std::vector<Rec>& v, const Less& less) {
        for (size_t i = 1; i < v.size(); ++i) {
            Rec key = v[i];
            size_t j = i;
            while (j > 0 && less(key, v[j - 1])) { v[j] = v[j - 1]; --j; }
            v[j] = key;
        }
    }

    static void mergeSort(std::vector<Rec>& v, const Less& less) {
        if (v.size() > 1) msort(v, 0, v.size() - 1, less);
    }

private:
    static void msort(std::vector<Rec>& v, size_t lo, size_t hi, const Less& less) {
        if (lo >= hi) return;
        size_t mid = lo + (hi - lo) / 2;
        msort(v, lo, mid, less);
        msort(v, mid + 1, hi, less);
        std::vector<Rec> tmp;
        tmp.reserve(hi - lo + 1);
        size_t i = lo, j = mid + 1;
        while (i <= mid && j <= hi) tmp.push_back(less(v[j], v[i]) ? v[j++] : v[i++]);
        while (i <= mid) tmp.push_back(v[i++]);
        while (j <= hi) tmp.push_back(v[j++]);
        for (size_t k = 0; k < tmp.size(); ++k) v[lo + k] = tmp[k];
    }
};
}
