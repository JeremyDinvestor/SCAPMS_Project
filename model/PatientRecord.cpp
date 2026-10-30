#include "model/PatientRecord.h"
#include "utils/Exceptions.h"
namespace scapms {

void PatientRecord::validate() const {
    if (id_.trimmed().isEmpty()) throw ClinicException("Patient ID cannot be empty.");
    if (name_.trimmed().isEmpty()) throw ClinicException("Patient name cannot be empty.");
    if (age_ < 0 || age_ > 120) throw ClinicException("Patient age must be between 0 and 120.");
    if (contact_.trimmed().isEmpty()) throw ClinicException("Contact information is required.");
    for (const QString* s : {&id_, &name_, &contact_, &detail_})
        if (s->contains('|') || s->contains('\n')) throw ClinicException("The characters '|' and line breaks are not allowed.");
}

void PatientRecord::checkAppointment(const Appointment& a) const {
    if (!a.date().isValid() || !a.time().isValid()) throw ClinicException("Invalid appointment date or time.");
    if (a.date() < QDate::currentDate()) throw ClinicException("Appointment date cannot be in the past.");
    for (const auto& x : appointments_)
        if (x.status() == Status::Pending && x.date() == a.date() && x.time() == a.time())
            throw ClinicException("Patient already has a pending appointment at that time.");
}

void PatientRecord::scheduleAppointment(const Appointment& a) {
    checkAppointment(a);
    appointments_.push_back(a);
}

bool PatientRecord::cancelAppointment(int appointmentId) {
    for (auto& a : appointments_)
        if (a.id() == appointmentId && a.status() == Status::Pending) {
            a.setStatus(Status::Cancelled);
            return true;
        }
    return false;
}

int PatientRecord::pendingCount() const {
    int n = 0;
    for (const auto& a : appointments_) if (a.status() == Status::Pending) ++n;
    return n;
}

int PatientRecord::completedCount() const {
    int n = 0;
    for (const auto& a : appointments_) if (a.status() == Status::Completed) ++n;
    return n;
}

Appointment* PatientRecord::findAppointment(int id) {
    for (auto& a : appointments_) if (a.id() == id) return &a;
    return nullptr;
}

QDate PatientRecord::nextPendingDate() const {
    QDate best(2999, 12, 31);
    for (const auto& a : appointments_)
        if (a.status() == Status::Pending && a.date() < best) best = a.date();
    return best;
}

QString PatientRecord::describe() const {
    return QString("[%1] %2 (%3), %4 yrs, %5 %6, priority %7")
        .arg(category(), name_, id_).arg(age_).arg(detailLabel(), detail_).arg(priority());
}

QString processRecord(const PatientRecord& p) {
    return p.describe() + QString(" | est. wait %1 min").arg(p.estimatedWaitMinutes());
}
}
