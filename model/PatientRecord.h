#pragma once
#include <memory>
#include <vector>
#include <QString>
#include <QDate>
#include "model/Appointable.h"
namespace scapms {
enum class SortField { None, Name, Id, Age, AppointmentDate };

class PatientRecord : public Appointable {
public:
    PatientRecord(QString id, QString name, int age, QString contact, QString detail)
        : id_(std::move(id)), name_(std::move(name)), age_(age), contact_(std::move(contact)), detail_(std::move(detail)) {}
    ~PatientRecord() override = default;

    virtual QString category() const = 0;
    virtual QString detailLabel() const = 0;
    virtual int priority() const = 0;
    virtual int estimatedWaitMinutes() const = 0;
    virtual std::shared_ptr<PatientRecord> clone() const = 0;
    virtual void validate() const;
    virtual void checkAppointment(const Appointment& a) const;
    QString describe() const;

    void scheduleAppointment(const Appointment& a) override;
    bool cancelAppointment(int appointmentId) override;
    int pendingCount() const override;
    int completedCount() const;
    void addLoadedAppointment(const Appointment& a) { appointments_.push_back(a); }
    Appointment* findAppointment(int id);
    QDate nextPendingDate() const;

    const QString& id() const { return id_; }
    const QString& name() const { return name_; }
    int age() const { return age_; }
    const QString& contact() const { return contact_; }
    const QString& detail() const { return detail_; }
    const std::vector<Appointment>& appointments() const { return appointments_; }
    std::vector<Appointment>& appointments() { return appointments_; }

private:
    QString id_, name_;
    int age_;
    QString contact_, detail_;
    std::vector<Appointment> appointments_;
};

using Rec = std::shared_ptr<PatientRecord>;

QString processRecord(const PatientRecord& p);
}
