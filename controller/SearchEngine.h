#pragma once
#include <vector>
#include "model/PatientRecord.h"
namespace scapms {

class SearchEngine {
public:
    static int linearById(const std::vector<Rec>& v, const QString& id) {
        for (size_t i = 0; i < v.size(); ++i)
            if (v[i]->id().compare(id, Qt::CaseInsensitive) == 0) return static_cast<int>(i);
        return -1;
    }

    static std::vector<int> linear(const std::vector<Rec>& v, const QString& key, SortField f) {
        std::vector<int> out;
        QString k = key.trimmed().toLower();
        for (size_t i = 0; i < v.size(); ++i)
            if (matches(*v[i], k, f)) out.push_back(static_cast<int>(i));
        return out;
    }

    static std::vector<int> linearByCategory(const std::vector<Rec>& v, const QString& key) {
        std::vector<int> out;
        QString k = key.trimmed().toLower();
        for (size_t i = 0; i < v.size(); ++i)
            if (v[i]->category().toLower().contains(k)) out.push_back(static_cast<int>(i));
        return out;
    }

    static int binary(const std::vector<Rec>& v, const QString& key, SortField f) {
        int lo = 0, hi = static_cast<int>(v.size()) - 1;
        QString k = key.trimmed().toLower();
        while (lo <= hi) {
            int mid = lo + (hi - lo) / 2;
            QString m = (f == SortField::Id) ? v[mid]->id().toLower() : v[mid]->name().toLower();
            if (m == k) return mid;
            if (m < k) lo = mid + 1; else hi = mid - 1;
        }
        return -1;
    }

    static void recursive(const std::vector<Rec>& v, const QString& key, SortField f,
                          size_t idx, std::vector<int>& out) {
        if (idx >= v.size()) return;
        if (matches(*v[idx], key.trimmed().toLower(), f)) out.push_back(static_cast<int>(idx));
        recursive(v, key, f, idx + 1, out);
    }

private:
    static bool matches(const PatientRecord& r, const QString& k, SortField f) {
        return f == SortField::Id ? r.id().toLower() == k : r.name().toLower().contains(k);
    }
};
}
