#pragma once
#include <queue>
#include <stack>
#include <vector>
#include <QDateTime>
#include "controller/ClinicDatabase.h"
#include "model/Patients.h"
namespace scapms {

struct QueueEntry {
    QString id;
    QDateTime joined;
};

struct Stats {
    int total = 0, general = 0, emergency = 0, followUp = 0;
    int appts = 0, pending = 0, completed = 0, cancelled = 0;
};

struct UndoOp {
    enum Type { DeletePatient, EditPatient, CancelAppointment } type = DeletePatient;
    Rec patient;
    int index = -1;
    QString patientId;
    int appointmentId = 0;
    Status prevStatus = Status::Pending;
    QString label;
};

class ClinicManager {
public:
    static constexpr int CacheSize = 10;

    ClinicDatabase& db() { return db_; }
    const ClinicDatabase& db() const { return db_; }

    QString addPatient(const QString& type, QString id, const QString& name, int age,
                       const QString& contact, const QString& detail);
    void editPatient(const QString& id, const QString& name, int age, const QString& contact, const QString& detail);
    void deletePatient(const QString& id);

    void scheduleAppointment(const QString& id, QDate date, QTime time, const QString& reason);
    void cancelAppointment(const QString& id, int appointmentId);
    void completeAppointment(const QString& id, int appointmentId);

    void enqueue(const QString& id);
    Rec callNext();
    std::vector<QueueEntry> waiting() const;

    void sort(const QString& algorithm, SortField field);
    std::vector<Rec> search(const QString& field, const QString& algorithm, const QString& text);
    const QString& lastSearchMethod() const { return lastMethod_; }

    bool canUndo() const { return !undo_.empty(); }
    QString undoLabel() const { return undo_.empty() ? QString() : undo_.top().label; }
    QString undo();

    void touchPatient(const QString& id);
    std::vector<PatientRecord*> recentPatients() const;

    Stats stats(const QDate& day = QDate()) const;
    QString reportFrequent() const;
    QString reportPending() const;
    QString reportDistribution() const;
    QString reportDaily() const;

    void save(const QString& path) const;
    void load(const QString& path);
    void newDatabase();

private:
    Rec require(const QString& id) const;
    void purgeFromQueue(const QString& id);
    void touch(PatientRecord* p);
    void forget(PatientRecord* p);
    void resetSession();

    ClinicDatabase db_;
    std::queue<QueueEntry> queue_;
    std::stack<UndoOp> undo_;
    PatientRecord* recent_[CacheSize] = {};
    int recentCount_ = 0;
    QString lastMethod_;
};
}
