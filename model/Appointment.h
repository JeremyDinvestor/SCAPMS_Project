#pragma once
#include <QString>
#include <QDate>
#include <QTime>
namespace scapms {
enum class Status { Pending, Completed, Cancelled };

inline QString toString(Status s) {
    switch (s) {
    case Status::Pending: return "Pending";
    case Status::Completed: return "Completed";
    default: return "Cancelled";
    }
}
inline Status statusFromString(const QString& s) {
    if (s == "Completed") return Status::Completed;
    if (s == "Cancelled") return Status::Cancelled;
    return Status::Pending;
}

class Appointment {
public:
    Appointment(int id, QString patientId, QDate date, QTime time, QString reason, Status st = Status::Pending)
        : id_(id), patientId_(std::move(patientId)), date_(date), time_(time), reason_(std::move(reason)), status_(st) {}
    int id() const { return id_; }
    const QString& patientId() const { return patientId_; }
    QDate date() const { return date_; }
    QTime time() const { return time_; }
    const QString& reason() const { return reason_; }
    Status status() const { return status_; }
    void setStatus(Status s) { status_ = s; }
private:
    int id_;
    QString patientId_;
    QDate date_;
    QTime time_;
    QString reason_;
    Status status_;
};
}
