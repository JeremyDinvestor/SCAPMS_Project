#include "controller/ClinicManager.h"
#include <QFile>
#include <QTextStream>
#include "controller/SearchEngine.h"
#include "controller/SortEngine.h"
#include "utils/Exceptions.h"
#include "utils/IDGenerator.h"
namespace scapms {

Rec ClinicManager::require(const QString& id) const {
    Rec r = db_.get(id);
    if (!r) throw ClinicException("Patient not found: " + id);
    return r;
}

QString ClinicManager::addPatient(const QString& type, QString id, const QString& name, int age,
                                  const QString& contact, const QString& detail) {
    id = id.trimmed();
    if (id.isEmpty()) {
        do { id = IDGenerator::nextPatientId(); } while (db_.get(id));
    }
    db_.add(makePatient(type, id, name.trimmed(), age, contact.trimmed(), detail.trimmed()));
    IDGenerator::observePatientId(id);
    return id;
}

void ClinicManager::editPatient(const QString& id, const QString& name, int age,
                                const QString& contact, const QString& detail) {
    Rec old = require(id);
    Rec np = makePatient(old->category(), id, name.trimmed(), age, contact.trimmed(), detail.trimmed());
    np->validate();
    for (const auto& a : old->appointments()) np->addLoadedAppointment(a);
    UndoOp op;
    op.type = UndoOp::EditPatient;
    op.patient = old->clone();
    op.patientId = id;
    op.label = "Edit patient " + id;
    forget(old.get());
    db_.replace(np);
    undo_.push(op);
}

void ClinicManager::deletePatient(const QString& id) {
    int idx = SearchEngine::linearById(db_.records(), id);
    if (idx < 0) throw ClinicException("Patient not found: " + id);
    Rec p = db_.records()[idx];
    forget(p.get());
    purgeFromQueue(id);
    db_.removeAt(idx);
    UndoOp op;
    op.type = UndoOp::DeletePatient;
    op.patient = p;
    op.index = idx;
    op.label = "Delete patient " + id;
    undo_.push(op);
}

void ClinicManager::scheduleAppointment(const QString& id, QDate date, QTime time, const QString& reason) {
    Rec p = require(id);
    if (reason.trimmed().isEmpty()) throw ClinicException("Appointment reason is required.");
    p->scheduleAppointment(Appointment(IDGenerator::nextAppointmentId(), id, date, time, reason.trimmed()));
    if (db_.sortedBy() == SortField::AppointmentDate) db_.setSortedBy(SortField::None);
}

void ClinicManager::cancelAppointment(const QString& id, int appointmentId) {
    Rec p = require(id);
    Appointment* a = p->findAppointment(appointmentId);
    if (!a) throw ClinicException("Appointment not found.");
    Status prev = a->status();
    if (!p->cancelAppointment(appointmentId)) throw ClinicException("Only pending appointments can be cancelled.");
    UndoOp op;
    op.type = UndoOp::CancelAppointment;
    op.patientId = id;
    op.appointmentId = appointmentId;
    op.prevStatus = prev;
    op.label = QString("Cancel appointment #%1").arg(appointmentId);
    undo_.push(op);
    if (db_.sortedBy() == SortField::AppointmentDate) db_.setSortedBy(SortField::None);
}

void ClinicManager::completeAppointment(const QString& id, int appointmentId) {
    Appointment* a = require(id)->findAppointment(appointmentId);
    if (!a) throw ClinicException("Appointment not found.");
    if (a->status() != Status::Pending) throw ClinicException("Only pending appointments can be completed.");
    a->setStatus(Status::Completed);
    if (db_.sortedBy() == SortField::AppointmentDate) db_.setSortedBy(SortField::None);
}

void ClinicManager::enqueue(const QString& id) {
    require(id);
    for (const auto& e : waiting())
        if (e.id == id) throw ClinicException("Patient is already in the waiting queue.");
    queue_.push({id, QDateTime::currentDateTime()});
}

Rec ClinicManager::callNext() {
    if (queue_.empty()) throw ClinicException("The waiting queue is empty.");
    QueueEntry e = queue_.front();
    queue_.pop();
    Rec r = require(e.id);
    for (auto& a : r->appointments())
        if (a.status() == Status::Pending && a.date() == QDate::currentDate()) {
            a.setStatus(Status::Completed);
            break;
        }
    touch(r.get());
    return r;
}

std::vector<QueueEntry> ClinicManager::waiting() const {
    std::vector<QueueEntry> out;
    auto copy = queue_;
    while (!copy.empty()) { out.push_back(copy.front()); copy.pop(); }
    return out;
}

void ClinicManager::purgeFromQueue(const QString& id) {
    std::queue<QueueEntry> keep;
    while (!queue_.empty()) {
        if (queue_.front().id != id) keep.push(queue_.front());
        queue_.pop();
    }
    queue_ = keep;
}

void ClinicManager::sort(const QString& algorithm, SortField field) {
    auto less = SortEngine::lessFor(field);
    if (algorithm == "Insertion Sort") SortEngine::insertionSort(db_.records(), less);
    else SortEngine::mergeSort(db_.records(), less);
    db_.setSortedBy(field);
}

std::vector<Rec> ClinicManager::search(const QString& field, const QString& algorithm, const QString& text) {
    QString key = text.trimmed();
    if (key.isEmpty()) throw ClinicException("Enter a search keyword.");
    const auto& v = db_.records();
    std::vector<int> idx;
    if (field == "Category") {
        lastMethod_ = "Linear search (category filter)";
        idx = SearchEngine::linearByCategory(v, key);
    } else {
        SortField f = (field == "ID") ? SortField::Id : SortField::Name;
        bool sorted = db_.sortedBy() == f;
        QString a = algorithm;
        if (a == "Auto") a = sorted ? "Binary" : (f == SortField::Id ? "Linear" : "Recursive");
        if (a == "Binary") {
            if (!sorted) throw ClinicException("Binary search needs the records sorted by the same field. Sort first.");
            int i = SearchEngine::binary(v, key, f);
            if (i >= 0) idx.push_back(i);
            lastMethod_ = "Binary search (list is sorted)";
            if (i < 0 && algorithm == "Auto" && f == SortField::Name) {
                SearchEngine::recursive(v, key, f, 0, idx);
                lastMethod_ += ", then recursive partial match";
            }
        } else if (a == "Recursive") {
            SearchEngine::recursive(v, key, f, 0, idx);
            lastMethod_ = "Recursive search (list is unsorted)";
        } else {
            idx = SearchEngine::linear(v, key, f);
            lastMethod_ = "Linear search (list is unsorted)";
        }
    }
    std::vector<Rec> out;
    for (int i : idx) out.push_back(v[i]);
    return out;
}

QString ClinicManager::undo() {
    if (undo_.empty()) throw ClinicException("Nothing to undo.");
    UndoOp op = undo_.top();
    undo_.pop();
    switch (op.type) {
    case UndoOp::DeletePatient:
        db_.insertAt(op.index, op.patient);
        break;
    case UndoOp::EditPatient: {
        Rec cur = db_.get(op.patientId);
        if (cur) forget(cur.get());
        db_.replace(op.patient);
        break;
    }
    case UndoOp::CancelAppointment: {
        Rec p = require(op.patientId);
        if (Appointment* a = p->findAppointment(op.appointmentId)) a->setStatus(op.prevStatus);
        break;
    }
    }
    return "Undone: " + op.label;
}

void ClinicManager::touch(PatientRecord* p) {
    int pos = -1;
    for (int i = 0; i < recentCount_; ++i) if (recent_[i] == p) pos = i;
    if (pos < 0) pos = (recentCount_ < CacheSize) ? recentCount_++ : CacheSize - 1;
    for (int i = pos; i > 0; --i) recent_[i] = recent_[i - 1];
    recent_[0] = p;
}

void ClinicManager::forget(PatientRecord* p) {
    int w = 0;
    for (int i = 0; i < recentCount_; ++i) if (recent_[i] != p) recent_[w++] = recent_[i];
    recentCount_ = w;
}

void ClinicManager::touchPatient(const QString& id) {
    if (Rec r = db_.get(id)) touch(r.get());
}

std::vector<PatientRecord*> ClinicManager::recentPatients() const {
    return std::vector<PatientRecord*>(recent_, recent_ + recentCount_);
}

Stats ClinicManager::stats(const QDate& day) const {
    Stats s;
    for (const auto& r : db_.records()) {
        ++s.total;
        QString c = r->category();
        if (c == "General") ++s.general; else if (c == "Emergency") ++s.emergency; else ++s.followUp;
        for (const auto& a : r->appointments()) {
            if (day.isValid() && a.date() != day) continue;
            ++s.appts;
            if (a.status() == Status::Pending) ++s.pending;
            else if (a.status() == Status::Completed) ++s.completed;
            else ++s.cancelled;
        }
    }
    return s;
}

QString ClinicManager::reportFrequent() const {
    auto v = db_.records();
    SortEngine::mergeSort(v, [](const Rec& a, const Rec& b) {
        if (a->completedCount() != b->completedCount()) return a->completedCount() > b->completedCount();
        return a->name().toLower() < b->name().toLower();
    });
    QString out = "MOST FREQUENTLY ATTENDED PATIENTS\n---------------------------------\n";
    int shown = 0;
    for (const auto& r : v) {
        if (r->completedCount() == 0 || shown >= 10) break;
        out += QString("%1. %2 (%3) - %4 completed\n").arg(++shown).arg(r->name(), r->id()).arg(r->completedCount());
    }
    return shown ? out : out + "(no completed appointments yet)\n";
}

QString ClinicManager::reportPending() const {
    QString out = "PATIENTS WITH PENDING APPOINTMENTS\n----------------------------------\n";
    int n = 0;
    for (const auto& r : db_.records()) {
        if (r->pendingCount() == 0) continue;
        ++n;
        out += QString("%1 (%2) - %3\n").arg(r->name(), r->id(), r->category());
        for (const auto& a : r->appointments())
            if (a.status() == Status::Pending)
                out += QString("    #%1  %2 %3  %4\n").arg(a.id()).arg(a.date().toString("yyyy-MM-dd"), a.time().toString("HH:mm"), a.reason());
    }
    return n ? out : out + "(none)\n";
}

QString ClinicManager::reportDistribution() const {
    Stats s = stats();
    return QString("PATIENT CATEGORY DISTRIBUTION\n-----------------------------\nGeneral Patients    %1\nEmergency Patients  %2\nFollow-Up Patients  %3\nTotal               %4\n")
        .arg(s.general).arg(s.emergency).arg(s.followUp).arg(s.total);
}

QString ClinicManager::reportDaily() const {
    Stats all = stats(), today = stats(QDate::currentDate());
    auto block = [](const QString& title, const Stats& s) {
        return QString("%1\nTotal Appointments: %2\nCompleted: %3\nPending: %4\nCancelled: %5\n")
            .arg(title).arg(s.appts).arg(s.completed).arg(s.pending).arg(s.cancelled);
    };
    return "DAILY APPOINTMENT SUMMARY\n-------------------------\n" +
           block("Today (" + QDate::currentDate().toString("yyyy-MM-dd") + ")", today) + "\n" + block("All dates", all);
}

void ClinicManager::save(const QString& path) const {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) throw ClinicException("Cannot write to file: " + path);
    QTextStream out(&f);
    out << "# SCAPMS data v1\n";
    for (const auto& r : db_.records())
        out << "P|" << r->category() << '|' << r->id() << '|' << r->name() << '|' << r->age() << '|'
            << r->contact() << '|' << r->detail() << '\n';
    for (const auto& r : db_.records())
        for (const auto& a : r->appointments())
            out << "A|" << a.id() << '|' << a.patientId() << '|' << a.date().toString(Qt::ISODate) << '|'
                << a.time().toString("HH:mm") << '|' << a.reason() << '|' << toString(a.status()) << '\n';
}

void ClinicManager::load(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) throw ClinicException("Cannot open file: " + path);
    QTextStream in(&f);
    ClinicDatabase tmp;
    int lineNo = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        ++lineNo;
        if (line.trimmed().isEmpty() || line.startsWith('#')) continue;
        QStringList p = line.split('|');
        try {
            if (p[0] == "P" && p.size() == 7) {
                bool ok = false;
                int age = p[4].toInt(&ok);
                if (!ok) throw ClinicException("Invalid age");
                tmp.add(makePatient(p[1], p[2], p[3], age, p[5], p[6]));
            } else if (p[0] == "A" && p.size() == 7) {
                Rec r = tmp.get(p[2]);
                if (!r) throw ClinicException("Appointment refers to an unknown patient");
                Appointment a(p[1].toInt(), p[2], QDate::fromString(p[3], Qt::ISODate),
                              QTime::fromString(p[4], "HH:mm"), p[5], statusFromString(p[6]));
                if (!a.date().isValid() || !a.time().isValid()) throw ClinicException("Invalid appointment date or time");
                r->addLoadedAppointment(a);
            } else {
                throw ClinicException("Unrecognised record");
            }
        } catch (const ClinicException& e) {
            throw ClinicException(QString("File error: %1 (line %2)").arg(e.message()).arg(lineNo));
        }
    }
    db_ = std::move(tmp);
    resetSession();
    for (const auto& r : db_.records()) {
        IDGenerator::observePatientId(r->id());
        for (const auto& a : r->appointments()) IDGenerator::observeAppointmentId(a.id());
    }
}

void ClinicManager::newDatabase() {
    db_ = ClinicDatabase();
    resetSession();
}

void ClinicManager::resetSession() {
    queue_ = std::queue<QueueEntry>();
    undo_ = std::stack<UndoOp>();
    recentCount_ = 0;
    IDGenerator::reset();
}
}
