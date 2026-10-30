#pragma once
#include <QStringList>
#include "model/PatientRecord.h"
namespace scapms {

class GeneralPatient : public PatientRecord {
public:
    using PatientRecord::PatientRecord;
    QString category() const override { return "General"; }
    QString detailLabel() const override { return "Insurance No."; }
    int priority() const override { return 3; }
    int estimatedWaitMinutes() const override { return 30; }
    Rec clone() const override { return std::make_shared<GeneralPatient>(*this); }
    void checkAppointment(const Appointment& a) const override;
};

class EmergencyPatient : public PatientRecord {
public:
    using PatientRecord::PatientRecord;
    QString category() const override { return "Emergency"; }
    QString detailLabel() const override { return "Severity"; }
    int priority() const override { return 1; }
    int estimatedWaitMinutes() const override { return 0; }
    Rec clone() const override { return std::make_shared<EmergencyPatient>(*this); }
    void validate() const override;
    void checkAppointment(const Appointment& a) const override;
};

class FollowUpPatient : public PatientRecord {
public:
    using PatientRecord::PatientRecord;
    QString category() const override { return "Follow-Up"; }
    QString detailLabel() const override { return "Previous visit"; }
    int priority() const override { return 2; }
    int estimatedWaitMinutes() const override { return 15; }
    Rec clone() const override { return std::make_shared<FollowUpPatient>(*this); }
    void validate() const override;
    void checkAppointment(const Appointment& a) const override;
};

QStringList patientTypes();
Rec makePatient(const QString& type, const QString& id, const QString& name, int age,
                const QString& contact, const QString& detail);
}
