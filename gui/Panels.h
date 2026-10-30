#pragma once
#include <functional>
#include <QWidget>
#include "controller/ClinicManager.h"

class QTableWidget;
class QComboBox;
class QLineEdit;
class QLabel;
class QSpinBox;
class QDateEdit;
class QTimeEdit;
class QStackedWidget;
class QTextEdit;
class QListWidget;

namespace scapms {
class StatTile;
class BarChart;

class Panel : public QWidget {
    Q_OBJECT
public:
    explicit Panel(ClinicManager& m, QWidget* parent = nullptr) : QWidget(parent), m_(m) {}
    virtual void refresh() = 0;
signals:
    void changed();
    void status(const QString& message);
protected:
    void run(const std::function<void()>& fn, bool notify = true);
    ClinicManager& m_;
};

class DashboardPanel : public Panel {
public:
    explicit DashboardPanel(ClinicManager& m, QWidget* parent = nullptr);
    void refresh() override;
private:
    void checkAlerts();
    StatTile *tPatients_, *tEmergency_, *tPending_, *tWaiting_;
    BarChart *cats_, *statuses_;
    QListWidget *reminders_, *recent_;
};

class PatientPanel : public Panel {
public:
    explicit PatientPanel(ClinicManager& m, QWidget* parent = nullptr);
    void refresh() override;
private:
    QString detail() const;
    void setDetail(const PatientRecord& r);
    int ageValue() const;
    void clearForm();
    QComboBox* type_;
    QLineEdit *id_, *name_, *age_, *contact_, *insurance_;
    QSpinBox* severity_;
    QDateEdit* prevVisit_;
    QStackedWidget* detailStack_;
    QLabel *detailLabel_, *idHint_;
    QTableWidget* table_;
};

class AppointmentPanel : public Panel {
public:
    explicit AppointmentPanel(ClinicManager& m, QWidget* parent = nullptr);
    void refresh() override;
private:
    void selected(QString& patientId, int& appointmentId) const;
    QComboBox* patient_;
    QDateEdit* date_;
    QTimeEdit* time_;
    QLineEdit* reason_;
    QTableWidget* table_;
};

class QueuePanel : public Panel {
public:
    explicit QueuePanel(ClinicManager& m, QWidget* parent = nullptr);
    void refresh() override;
private:
    QComboBox* patient_;
    QLabel* serving_;
    QTableWidget* table_;
};

class SearchPanel : public Panel {
public:
    explicit SearchPanel(ClinicManager& m, QWidget* parent = nullptr);
    void refresh() override;
    void focusSearch();
private:
    void setInfo(const QString& text);
    QComboBox *field_, *algo_, *sortAlgo_, *sortBy_;
    QLineEdit* text_;
    QLabel* info_;
    QTableWidget* table_;
};

class ReportPanel : public Panel {
public:
    explicit ReportPanel(ClinicManager& m, QWidget* parent = nullptr);
    void refresh() override;
private:
    void generate();
    QComboBox* which_;
    QTextEdit* output_;
};
}
