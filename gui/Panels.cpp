#include "gui/Panels.h"
#include <QtWidgets>
#include "gui/Icons.h"
#include "gui/Widgets.h"
#include "utils/Exceptions.h"
namespace scapms {
namespace {

QWidget* pageHeader(const QString& title, const QString& sub) {
    auto* w = new QWidget;
    auto* l = new QVBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    auto* t = new QLabel(title);
    t->setObjectName("pageTitle");
    auto* s = new QLabel(sub);
    s->setObjectName("pageSub");
    l->addWidget(t);
    l->addWidget(s);
    return w;
}

QFrame* makeCard() {
    auto* f = new QFrame;
    f->setObjectName("card");
    return f;
}

QFrame* titledCard(const QString& title, QWidget* content) {
    auto* f = makeCard();
    auto* l = new QVBoxLayout(f);
    l->setContentsMargins(16, 14, 16, 14);
    auto* h = new QLabel(title);
    h->setObjectName("cardTitle");
    l->addWidget(h);
    l->addWidget(content, 1);
    return f;
}

QFrame* formCard(const QString& title, QFormLayout* form, QLayout* buttons) {
    auto* f = makeCard();
    auto* l = new QVBoxLayout(f);
    l->setContentsMargins(16, 14, 16, 14);
    l->setSpacing(10);
    auto* h = new QLabel(title);
    h->setObjectName("cardTitle");
    l->addWidget(h);
    form->setHorizontalSpacing(12);
    l->addLayout(form);
    if (buttons) l->addLayout(buttons);
    l->addStretch();
    return f;
}

QPushButton* button(const QString& text, const QString& tip, const QString& icon, bool primary = false) {
    auto* b = new QPushButton(text);
    b->setToolTip(tip);
    b->setIcon(makeIcon(icon, primary ? QColor("#ffffff") : QColor("#0d9488")));
    b->setIconSize(QSize(16, 16));
    b->setCursor(Qt::PointingHandCursor);
    b->setProperty("primary", primary);
    return b;
}

QTableWidget* makeTable(const QStringList& headers, const QList<int>& badgeCols = {}) {
    auto* t = new QTableWidget;
    t->setColumnCount(headers.size());
    t->setHorizontalHeaderLabels(headers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setAlternatingRowColors(true);
    t->setShowGrid(false);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    t->horizontalHeader()->setHighlightSections(false);
    t->verticalHeader()->hide();
    t->verticalHeader()->setDefaultSectionSize(42);
    for (int c : badgeCols) t->setItemDelegateForColumn(c, new BadgeDelegate(t));
    return t;
}

void fillPatientTable(QTableWidget* t, const std::vector<Rec>& rows) {
    t->setRowCount(static_cast<int>(rows.size()));
    for (int r = 0; r < t->rowCount(); ++r) {
        const PatientRecord& p = *rows[r];
        QStringList c{p.id(), p.name(), QString::number(p.age()), p.contact(), p.category(),
                      QString::number(p.priority()), QString::number(p.pendingCount())};
        for (int i = 0; i < c.size(); ++i) {
            auto* item = new QTableWidgetItem(c[i]);
            item->setToolTip(processRecord(p));
            t->setItem(r, i, item);
        }
    }
}

void fillPatientCombo(QComboBox* box, const std::vector<Rec>& rows) {
    QString cur = box->currentData().toString();
    box->clear();
    for (const auto& r : rows) box->addItem(r->id() + " - " + r->name(), r->id());
    int i = box->findData(cur);
    if (i >= 0) box->setCurrentIndex(i);
}

QVBoxLayout* pageLayout(QWidget* page, const QString& title, const QString& sub) {
    page->setObjectName("page");
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(22, 20, 22, 20);
    root->setSpacing(14);
    root->addWidget(pageHeader(title, sub));
    return root;
}

const QStringList kPatientHeaders{"ID", "Name", "Age", "Contact", "Category", "Priority", "Pending"};
}

void Panel::run(const std::function<void()>& fn, bool notify) {
    try {
        fn();
        if (notify) emit changed();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", "Error: " + QString::fromUtf8(e.what()));
    } catch (...) {
        QMessageBox::critical(this, "Error", "An unexpected error occurred.");
    }
}

DashboardPanel::DashboardPanel(ClinicManager& m, QWidget* parent) : Panel(m, parent) {
    tPatients_ = new StatTile("users", QColor("#0d9488"), QColor("#14b8a6"), "Total patients");
    tEmergency_ = new StatTile("alert", QColor("#ef4444"), QColor("#f97316"), "Emergency cases");
    tPending_ = new StatTile("calendar", QColor("#6366f1"), QColor("#8b5cf6"), "Pending appointments");
    tWaiting_ = new StatTile("clock", QColor("#f59e0b"), QColor("#fbbf24"), "Waiting now");
    cats_ = new BarChart;
    statuses_ = new BarChart;
    reminders_ = new QListWidget;
    recent_ = new QListWidget;
    reminders_->setObjectName("plain");
    recent_->setObjectName("plain");

    auto* root = pageLayout(this, "Dashboard", "Clinic overview at a glance");
    auto* tiles = new QHBoxLayout;
    tiles->setSpacing(14);
    for (auto* t : {tPatients_, tEmergency_, tPending_, tWaiting_}) tiles->addWidget(t);
    root->addLayout(tiles);
    auto* grid = new QGridLayout;
    grid->setSpacing(14);
    grid->addWidget(titledCard("Patient categories", cats_), 0, 0);
    grid->addWidget(titledCard("Appointment status", statuses_), 0, 1);
    grid->addWidget(titledCard("Today's reminders", reminders_), 1, 0);
    grid->addWidget(titledCard("Recently accessed patients", recent_), 1, 1);
    root->addLayout(grid, 1);

    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this] { refresh(); checkAlerts(); });
    timer->start(5000);
}

void DashboardPanel::refresh() {
    Stats s = m_.stats();
    tPatients_->setValue(QString::number(s.total));
    tEmergency_->setValue(QString::number(s.emergency));
    tPending_->setValue(QString::number(s.pending));
    tWaiting_->setValue(QString::number(m_.waiting().size()));
    cats_->setBars({{"General", s.general, QColor("#10b981")},
                    {"Emergency", s.emergency, QColor("#ef4444")},
                    {"Follow-Up", s.followUp, QColor("#3b82f6")}}, s.total);
    statuses_->setBars({{"Pending", s.pending, QColor("#f59e0b")},
                        {"Completed", s.completed, QColor("#22c55e")},
                        {"Cancelled", s.cancelled, QColor("#9ca3af")}}, s.appts);
    reminders_->clear();
    for (const auto& r : m_.db().records())
        for (const auto& a : r->appointments())
            if (a.status() == Status::Pending && a.date() == QDate::currentDate())
                reminders_->addItem(a.time().toString("HH:mm") + "   " + r->name() + " - " + a.reason());
    if (reminders_->count() == 0) reminders_->addItem("No pending appointments today");
    recent_->clear();
    for (PatientRecord* p : m_.recentPatients())
        recent_->addItem(p->id() + " - " + p->name() + " (" + p->category() + ")");
    if (recent_->count() == 0) recent_->addItem("No patients opened yet");
}

void DashboardPanel::checkAlerts() {
    int due = m_.stats(QDate::currentDate()).pending;
    auto w = m_.waiting();
    if (!w.empty()) {
        qint64 mins = w.front().joined.secsTo(QDateTime::currentDateTime()) / 60;
        if (mins >= 10) { emit status(QString("Alert: a patient has been waiting %1 minutes.").arg(mins)); return; }
    }
    if (due > 0) emit status(QString("Reminder: %1 appointment(s) pending today.").arg(due));
}

PatientPanel::PatientPanel(ClinicManager& m, QWidget* parent) : Panel(m, parent) {
    type_ = new QComboBox;
    type_->addItems(patientTypes());
    id_ = new QLineEdit;
    id_->setPlaceholderText("Auto-generated if empty");
    name_ = new QLineEdit;
    age_ = new QLineEdit;
    contact_ = new QLineEdit;
    idHint_ = new QLabel;
    idHint_->setObjectName("error");
    insurance_ = new QLineEdit;
    severity_ = new QSpinBox;
    severity_->setRange(1, 5);
    severity_->setValue(3);
    prevVisit_ = new QDateEdit(QDate::currentDate());
    prevVisit_->setCalendarPopup(true);
    prevVisit_->setDisplayFormat("yyyy-MM-dd");
    detailStack_ = new QStackedWidget;
    detailStack_->addWidget(insurance_);
    detailStack_->addWidget(severity_);
    detailStack_->addWidget(prevVisit_);
    detailLabel_ = new QLabel("Insurance No.:");
    table_ = makeTable(kPatientHeaders, {4});

    auto* form = new QFormLayout;
    form->addRow("Type:", type_);
    form->addRow("Patient ID:", id_);
    form->addRow(idHint_);
    form->addRow("Name:", name_);
    form->addRow("Age:", age_);
    form->addRow("Contact:", contact_);
    form->addRow(detailLabel_, detailStack_);

    auto* add = button("Add Patient", "Register a new patient", "plus", true);
    auto* edit = button("Update", "Save changes to the selected patient", "edit");
    auto* del = button("Delete", "Delete the selected patient (can be undone)", "trash");
    auto* clear = button("Clear", "Clear the form", "refresh");
    auto* row = new QGridLayout;
    row->addWidget(add, 0, 0); row->addWidget(edit, 0, 1);
    row->addWidget(del, 1, 0); row->addWidget(clear, 1, 1);

    auto* card = formCard("Patient details", form, row);
    card->setFixedWidth(360);
    auto* root = pageLayout(this, "Patient Records", "Register, edit and manage patients");
    auto* body = new QHBoxLayout;
    body->setSpacing(14);
    body->addWidget(card);
    body->addWidget(table_, 1);
    root->addLayout(body, 1);

    connect(type_, &QComboBox::currentIndexChanged, this, [this](int i) {
        static const QStringList labels{"Insurance No.:", "Severity (1-5):", "Previous Visit:"};
        detailStack_->setCurrentIndex(i);
        detailLabel_->setText(labels[i]);
    });
    connect(id_, &QLineEdit::textChanged, this, [this](const QString& t) {
        bool exists = !t.trimmed().isEmpty() && m_.db().get(t.trimmed());
        int row = table_->currentRow();
        bool self = row >= 0 && table_->item(row, 0) && table_->item(row, 0)->text() == t.trimmed();
        idHint_->setText(exists && !self ? "Error: Patient ID already exists" : "");
    });
    connect(table_, &QTableWidget::itemSelectionChanged, this, [this] {
        int row = table_->currentRow();
        if (row < 0 || !table_->item(row, 0)) return;
        Rec r = m_.db().get(table_->item(row, 0)->text());
        if (!r) return;
        m_.touchPatient(r->id());
        type_->setCurrentText(r->category());
        id_->setText(r->id());
        name_->setText(r->name());
        age_->setText(QString::number(r->age()));
        contact_->setText(r->contact());
        setDetail(*r);
    });
    connect(add, &QPushButton::clicked, this, [this] {
        run([this] {
            QString id = m_.addPatient(type_->currentText(), id_->text(), name_->text(), ageValue(), contact_->text(), detail());
            emit status("Registered patient " + id);
            clearForm();
        });
    });
    connect(edit, &QPushButton::clicked, this, [this] {
        run([this] {
            QString id = id_->text().trimmed();
            Rec r = m_.db().get(id);
            if (!r) throw ClinicException("Select a patient in the table to edit.");
            if (r->category() != type_->currentText())
                throw ClinicException("Patient type cannot be changed. Delete and re-register instead.");
            m_.editPatient(id, name_->text(), ageValue(), contact_->text(), detail());
            emit status("Updated patient " + id);
        });
    });
    connect(del, &QPushButton::clicked, this, [this] {
        int row = table_->currentRow();
        if (row < 0) { QMessageBox::information(this, "Delete", "Select a patient first."); return; }
        QString id = table_->item(row, 0)->text();
        if (QMessageBox::question(this, "Delete patient", "Delete patient " + id + "?") != QMessageBox::Yes) return;
        run([&] { m_.deletePatient(id); clearForm(); emit status("Deleted patient " + id); });
    });
    connect(clear, &QPushButton::clicked, this, [this] { clearForm(); });
}

QString PatientPanel::detail() const {
    switch (type_->currentIndex()) {
    case 0: return insurance_->text();
    case 1: return QString::number(severity_->value());
    default: return prevVisit_->date().toString("yyyy-MM-dd");
    }
}

void PatientPanel::setDetail(const PatientRecord& r) {
    switch (type_->currentIndex()) {
    case 0: insurance_->setText(r.detail()); break;
    case 1: severity_->setValue(r.detail().toInt()); break;
    default: prevVisit_->setDate(QDate::fromString(r.detail(), "yyyy-MM-dd"));
    }
}

int PatientPanel::ageValue() const {
    bool ok = false;
    int a = age_->text().trimmed().toInt(&ok);
    if (!ok) throw ClinicException("Patient age must be a whole number.");
    return a;
}

void PatientPanel::clearForm() {
    table_->clearSelection();
    table_->setCurrentCell(-1, -1);
    id_->clear(); name_->clear(); age_->clear(); contact_->clear(); insurance_->clear();
    severity_->setValue(3);
    prevVisit_->setDate(QDate::currentDate());
}

void PatientPanel::refresh() {
    QSignalBlocker b(table_);
    fillPatientTable(table_, m_.db().records());
}

AppointmentPanel::AppointmentPanel(ClinicManager& m, QWidget* parent) : Panel(m, parent) {
    patient_ = new QComboBox;
    date_ = new QDateEdit(QDate::currentDate());
    date_->setCalendarPopup(true);
    time_ = new QTimeEdit(QTime(9, 0));
    time_->setDisplayFormat("HH:mm");
    reason_ = new QLineEdit;
    table_ = makeTable({"Appt ID", "Patient", "Date", "Time", "Reason", "Status"}, {5});
    auto* sched = button("Schedule", "Schedule an appointment for the chosen patient", "calendar", true);
    auto* cancel = button("Cancel Selected", "Cancel the selected pending appointment (can be undone)", "x");
    auto* done = button("Mark Completed", "Mark the selected appointment as completed", "check");

    auto* form = new QFormLayout;
    form->addRow("Patient:", patient_);
    form->addRow("Date:", date_);
    form->addRow("Time:", time_);
    form->addRow("Reason:", reason_);
    auto* btns = new QVBoxLayout;
    btns->addWidget(sched); btns->addWidget(cancel); btns->addWidget(done);
    auto* card = formCard("New appointment", form, btns);
    card->setFixedWidth(360);

    auto* root = pageLayout(this, "Appointments", "Schedule and track appointments");
    auto* body = new QHBoxLayout;
    body->setSpacing(14);
    body->addWidget(card);
    body->addWidget(table_, 1);
    root->addLayout(body, 1);

    connect(sched, &QPushButton::clicked, this, [this] {
        run([this] {
            m_.scheduleAppointment(patient_->currentData().toString(), date_->date(), time_->time(), reason_->text());
            reason_->clear();
            emit status("Appointment scheduled");
        });
    });
    connect(cancel, &QPushButton::clicked, this, [this] {
        run([this] { QString p; int a; selected(p, a); m_.cancelAppointment(p, a); emit status("Appointment cancelled"); });
    });
    connect(done, &QPushButton::clicked, this, [this] {
        run([this] { QString p; int a; selected(p, a); m_.completeAppointment(p, a); emit status("Appointment completed"); });
    });
}

void AppointmentPanel::selected(QString& patientId, int& appointmentId) const {
    int row = table_->currentRow();
    if (row < 0) throw ClinicException("Select an appointment first.");
    patientId = table_->item(row, 1)->data(Qt::UserRole).toString();
    appointmentId = table_->item(row, 0)->text().toInt();
}

void AppointmentPanel::refresh() {
    fillPatientCombo(patient_, m_.db().records());
    int n = 0;
    for (const auto& r : m_.db().records()) n += static_cast<int>(r->appointments().size());
    table_->setRowCount(n);
    int row = 0;
    for (const auto& r : m_.db().records())
        for (const auto& a : r->appointments()) {
            auto* name = new QTableWidgetItem(r->name());
            name->setData(Qt::UserRole, r->id());
            table_->setItem(row, 0, new QTableWidgetItem(QString::number(a.id())));
            table_->setItem(row, 1, name);
            table_->setItem(row, 2, new QTableWidgetItem(a.date().toString("yyyy-MM-dd")));
            table_->setItem(row, 3, new QTableWidgetItem(a.time().toString("HH:mm")));
            table_->setItem(row, 4, new QTableWidgetItem(a.reason()));
            table_->setItem(row, 5, new QTableWidgetItem(toString(a.status())));
            ++row;
        }
}

QueuePanel::QueuePanel(ClinicManager& m, QWidget* parent) : Panel(m, parent) {
    patient_ = new QComboBox;
    serving_ = new QLabel("Now serving: -");
    serving_->setObjectName("serving");
    table_ = makeTable({"#", "ID", "Name", "Category", "Waited (min)", "Est. wait (min)"}, {3});
    auto* add = button("Add to Queue", "Add the chosen patient to the back of the queue", "plus");
    auto* next = button("Call Next", "Serve the patient at the front of the queue", "play", true);

    auto* card = makeCard();
    auto* cl = new QVBoxLayout(card);
    cl->setContentsMargins(16, 14, 16, 14);
    auto* row = new QHBoxLayout;
    row->addWidget(patient_, 1);
    row->addWidget(add);
    row->addWidget(next);
    cl->addLayout(row);
    cl->addWidget(serving_);

    auto* root = pageLayout(this, "Waiting Queue", "First come, first served");
    root->addWidget(card);
    root->addWidget(table_, 1);

    connect(add, &QPushButton::clicked, this, [this] {
        run([this] { m_.enqueue(patient_->currentData().toString()); emit status("Patient added to queue"); });
    });
    connect(next, &QPushButton::clicked, this, [this] {
        run([this] {
            Rec r = m_.callNext();
            serving_->setText("Now serving: " + r->name() + " (" + r->id() + ")");
            emit status("Called " + r->name());
        });
    });
}

void QueuePanel::refresh() {
    fillPatientCombo(patient_, m_.db().records());
    auto w = m_.waiting();
    table_->setRowCount(static_cast<int>(w.size()));
    for (int i = 0; i < table_->rowCount(); ++i) {
        Rec r = m_.db().get(w[i].id);
        if (!r) continue;
        qint64 mins = w[i].joined.secsTo(QDateTime::currentDateTime()) / 60;
        QStringList c{QString::number(i + 1), r->id(), r->name(), r->category(),
                      QString::number(mins), QString::number(r->estimatedWaitMinutes())};
        for (int k = 0; k < c.size(); ++k) table_->setItem(i, k, new QTableWidgetItem(c[k]));
    }
}

SearchPanel::SearchPanel(ClinicManager& m, QWidget* parent) : Panel(m, parent) {
    field_ = new QComboBox;
    field_->addItems({"ID", "Name", "Category"});
    algo_ = new QComboBox;
    algo_->addItems({"Auto", "Linear", "Binary", "Recursive"});
    algo_->setToolTip("Auto picks binary search when the list is sorted by the searched field");
    sortAlgo_ = new QComboBox;
    sortAlgo_->addItems({"Insertion Sort", "Merge Sort"});
    sortBy_ = new QComboBox;
    sortBy_->addItems({"Patient Name", "Patient ID", "Age", "Appointment Date"});
    text_ = new QLineEdit;
    text_->setPlaceholderText("Search keyword (Ctrl+F)");
    info_ = new QLabel;
    info_->setObjectName("infoBar");
    info_->setWordWrap(true);
    info_->hide();
    table_ = makeTable(kPatientHeaders, {4});
    auto* search = button("Search", "Search using the selected algorithm", "search", true);
    auto* all = button("Show All", "Show every patient", "users");
    auto* sort = button("SORT", "Sort the patient records", "sort", true);

    auto* sForm = new QFormLayout;
    sForm->addRow("Search by:", field_);
    sForm->addRow("Algorithm:", algo_);
    sForm->addRow("Keyword:", text_);
    auto* sBtns = new QHBoxLayout;
    sBtns->addWidget(search);
    sBtns->addWidget(all);
    auto* oForm = new QFormLayout;
    oForm->addRow("Sort Algorithm:", sortAlgo_);
    oForm->addRow("Sort By:", sortBy_);
    auto* oBtns = new QHBoxLayout;
    oBtns->addWidget(sort);

    auto* root = pageLayout(this, "Search & Sort", "Find and organise patient records");
    auto* top = new QHBoxLayout;
    top->setSpacing(14);
    top->addWidget(formCard("Search", sForm, sBtns));
    top->addWidget(formCard("Sort", oForm, oBtns));
    root->addLayout(top);
    root->addWidget(info_);
    root->addWidget(table_, 1);

    auto doSearch = [this] {
        run([this] {
            auto res = m_.search(field_->currentText(), algo_->currentText(), text_->text());
            fillPatientTable(table_, res);
            setInfo(QString("%1 result(s) - %2").arg(res.size()).arg(m_.lastSearchMethod()));
            emit status(QString("%1 result(s) found").arg(res.size()));
        }, false);
    };
    connect(search, &QPushButton::clicked, this, doSearch);
    connect(text_, &QLineEdit::returnPressed, this, doSearch);
    connect(all, &QPushButton::clicked, this, [this] { refresh(); setInfo(QString()); });
    connect(sort, &QPushButton::clicked, this, [this] {
        run([this] {
            static const SortField fields[] = {SortField::Name, SortField::Id, SortField::Age, SortField::AppointmentDate};
            QString algo = sortAlgo_->currentText();
            int n = static_cast<int>(m_.db().records().size());
            m_.sort(algo, fields[sortBy_->currentIndex()]);
            QString why = (algo == "Insertion Sort")
                ? QString("Insertion Sort on %1 record(s): simple, stable and fast for small or nearly sorted lists (O(n^2) worst case).").arg(n)
                : QString("Merge Sort on %1 record(s): stable and O(n log n) in every case, best for large lists.").arg(n);
            if (algo == "Insertion Sort" && n > 100) why += " Merge Sort is advised for this size.";
            setInfo(why);
            emit status("Records sorted");
        });
    });
}

void SearchPanel::setInfo(const QString& text) {
    info_->setText(text);
    info_->setVisible(!text.isEmpty());
}

void SearchPanel::refresh() { fillPatientTable(table_, m_.db().records()); }

void SearchPanel::focusSearch() { text_->setFocus(); text_->selectAll(); }

ReportPanel::ReportPanel(ClinicManager& m, QWidget* parent) : Panel(m, parent) {
    which_ = new QComboBox;
    which_->addItems({"Most Frequently Attended Patients", "Patients with Pending Appointments",
                      "Patient Category Distribution", "Daily Appointment Summary"});
    output_ = new QTextEdit;
    output_->setReadOnly(true);
    output_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    auto* gen = button("Generate", "Generate the selected report", "chart", true);
    auto* root = pageLayout(this, "Reports", "Summaries of clinic activity");
    auto* row = new QHBoxLayout;
    row->addWidget(which_, 1);
    row->addWidget(gen);
    root->addLayout(row);
    root->addWidget(output_, 1);
    connect(gen, &QPushButton::clicked, this, [this] { generate(); });
    connect(which_, &QComboBox::currentIndexChanged, this, [this] { generate(); });
}

void ReportPanel::generate() {
    switch (which_->currentIndex()) {
    case 0: output_->setPlainText(m_.reportFrequent()); break;
    case 1: output_->setPlainText(m_.reportPending()); break;
    case 2: output_->setPlainText(m_.reportDistribution()); break;
    default: output_->setPlainText(m_.reportDaily());
    }
}

void ReportPanel::refresh() { generate(); }
}
