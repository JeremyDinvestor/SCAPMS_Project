#pragma once
#include <QString>
namespace scapms {
class IDGenerator {
public:
    static QString nextPatientId() { return QString("P%1").arg(++p_, 3, 10, QChar('0')); }
    static int nextAppointmentId() { return ++a_; }
    static void observePatientId(const QString& id) {
        bool ok = false;
        int n = id.mid(1).toInt(&ok);
        if (ok && n > p_) p_ = n;
    }
    static void observeAppointmentId(int id) { if (id > a_) a_ = id; }
    static void reset() { p_ = 0; a_ = 0; }
private:
    inline static int p_ = 0;
    inline static int a_ = 0;
};
}
