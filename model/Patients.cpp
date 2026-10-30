#include "model/Patients.h"
#include "utils/Exceptions.h"
namespace scapms {

void GeneralPatient::checkAppointment(const Appointment& a) const {
    PatientRecord::checkAppointment(a);
    if (a.time() < QTime(8, 0) || a.time() > QTime(17, 0))
        throw ClinicException("General appointments must be between 08:00 and 17:00.");
}

void EmergencyPatient::validate() const {
    PatientRecord::validate();
    bool ok = false;
    int s = detail().toInt(&ok);
    if (!ok || s < 1 || s > 5) throw ClinicException("Emergency severity must be a number from 1 to 5.");
}

void EmergencyPatient::checkAppointment(const Appointment& a) const {
    if (!a.date().isValid() || !a.time().isValid()) throw ClinicException("Invalid appointment date or time.");
}

void FollowUpPatient::validate() const {
    PatientRecord::validate();
    QDate prev = QDate::fromString(detail(), "yyyy-MM-dd");
    if (!prev.isValid()) throw ClinicException("Previous visit date must be in yyyy-MM-dd format.");
    if (prev > QDate::currentDate()) throw ClinicException("Previous visit cannot be in the future.");
}

void FollowUpPatient::checkAppointment(const Appointment& a) const {
    PatientRecord::checkAppointment(a);
    if (a.date() <= QDate::fromString(detail(), "yyyy-MM-dd"))
        throw ClinicException("A follow-up must be after the previous visit.");
}

QStringList patientTypes() { return {"General", "Emergency", "Follow-Up"}; }

Rec makePatient(const QString& type, const QString& id, const QString& name, int age,
                const QString& contact, const QString& detail) {
    if (type == "General") return std::make_shared<GeneralPatient>(id, name, age, contact, detail);
    if (type == "Emergency") return std::make_shared<EmergencyPatient>(id, name, age, contact, detail);
    if (type == "Follow-Up") return std::make_shared<FollowUpPatient>(id, name, age, contact, detail);
    throw ClinicException("Unknown patient type: " + type);
}
}
