#include "gui/MainWindow.h"
#include <QtWidgets>
#include "gui/Icons.h"
#include "gui/Panels.h"
#include "gui/Theme.h"
namespace scapms {
namespace { const QString kDataFile = "scapms_data.txt"; }

MainWindow::MainWindow() {
    setWindowTitle("SCAPMS - Smart Clinic Appointment & Patient Management System");
    resize(1180, 760);

    auto* central = new QWidget;
    central->setObjectName("central");
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* header = new QFrame;
    header->setObjectName("header");
    auto* hl = new QHBoxLayout(header);
    hl->setContentsMargins(20, 12, 20, 12);
    hl->setSpacing(12);
    auto* logo = new QLabel;
    logo->setObjectName("logo");
    logo->setFixedSize(42, 42);
    logo->setAlignment(Qt::AlignCenter);
    logo->setPixmap(iconPixmap("logo", Qt::white, 48).scaled(26, 26, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto* brand = new QLabel("SCAPMS");
    brand->setObjectName("brand");
    auto* sub = new QLabel("Smart Clinic Appointment & Patient Management System");
    sub->setObjectName("brandSub");
    auto* titles = new QVBoxLayout;
    titles->setSpacing(0);
    titles->addWidget(brand);
    titles->addWidget(sub);
    themeBtn_ = new QToolButton;
    themeBtn_->setObjectName("themeBtn");
    themeBtn_->setToolTip("Toggle light / dark theme");
    themeBtn_->setCursor(Qt::PointingHandCursor);
    hl->addWidget(logo);
    hl->addLayout(titles);
    hl->addStretch();
    hl->addWidget(themeBtn_);
    root->addWidget(header);

    sidebar_ = new QListWidget;
    sidebar_->setObjectName("sidebar");
    sidebar_->setFixedWidth(220);
    sidebar_->setIconSize(QSize(20, 20));
    sidebar_->setFocusPolicy(Qt::NoFocus);
    stack_ = new QStackedWidget;
    auto* body = new QHBoxLayout;
    body->setSpacing(0);
    body->addWidget(sidebar_);
    body->addWidget(stack_, 1);
    root->addLayout(body, 1);
    setCentralWidget(central);

    auto add = [this](Panel* p, const QString& title, const QString& icon) {
        auto* item = new QListWidgetItem(makeIcon(icon, QColor("#b6c7d4"), QColor("#ffffff")), title);
        item->setSizeHint(QSize(0, 44));
        sidebar_->addItem(item);
        stack_->addWidget(p);
        panels_.push_back(p);
        connect(p, &Panel::changed, this, [this] { refreshAll(); });
        connect(p, &Panel::status, this, [this](const QString& s) { statusBar()->showMessage(s, 5000); });
    };
    add(new DashboardPanel(manager_), "Dashboard", "home");
    add(new PatientPanel(manager_), "Patient Records", "users");
    add(new AppointmentPanel(manager_), "Appointments", "calendar");
    add(new QueuePanel(manager_), "Waiting Queue", "clock");
    search_ = new SearchPanel(manager_);
    add(search_, "Search & Sort", "search");
    add(new ReportPanel(manager_), "Reports", "chart");
    connect(sidebar_, &QListWidget::currentRowChanged, stack_, &QStackedWidget::setCurrentIndex);
    sidebar_->setCurrentRow(0);

    counts_ = new QLabel;
    statusBar()->addPermanentWidget(counts_);
    connect(themeBtn_, &QToolButton::clicked, this, [this] { applyTheme(!dark_); });

    auto addAction = [this](QMenu* menu, const QString& text, const QKeySequence& key, std::function<void()> fn) {
        QAction* a = menu->addAction(text);
        if (!key.isEmpty()) a->setShortcut(key);
        connect(a, &QAction::triggered, this, [fn] { fn(); });
        return a;
    };

    QMenu* file = menuBar()->addMenu("&File");
    addAction(file, "&New", QKeySequence::New, [this] {
        if (QMessageBox::question(this, "New", "Discard all current data?") == QMessageBox::Yes)
            guarded([this] { manager_.newDatabase(); });
    });
    addAction(file, "&Save", QKeySequence("Ctrl+S"), [this] {
        guarded([this] { manager_.save(kDataFile); statusBar()->showMessage("Data saved", 5000); });
    });
    addAction(file, "&Load", QKeySequence("Ctrl+L"), [this] {
        guarded([this] { manager_.load(kDataFile); statusBar()->showMessage("Data loaded", 5000); });
    });
    file->addSeparator();
    addAction(file, "&Import...", QKeySequence(), [this] {
        QString p = QFileDialog::getOpenFileName(this, "Import patient data", QString(), "Data files (*.txt *.csv);;All files (*)");
        if (!p.isEmpty()) guarded([&] { manager_.load(p); });
    });
    addAction(file, "&Export...", QKeySequence(), [this] {
        QString p = QFileDialog::getSaveFileName(this, "Export patient data", "scapms_export.txt", "Text files (*.txt);;CSV files (*.csv)");
        if (!p.isEmpty()) guarded([&] { manager_.save(p); statusBar()->showMessage("Exported to " + p, 5000); });
    });
    file->addSeparator();
    addAction(file, "E&xit", QKeySequence("Ctrl+Q"), [this] { close(); });

    QMenu* edit = menuBar()->addMenu("&Edit");
    undoAct_ = addAction(edit, "Undo Last Administrative Operation", QKeySequence("Ctrl+Z"), [this] {
        guarded([this] { statusBar()->showMessage(manager_.undo(), 5000); });
    });
    addAction(edit, "&Find", QKeySequence("Ctrl+F"), [this] {
        sidebar_->setCurrentRow(4);
        search_->focusSearch();
    });

    applyTheme(false);
    if (QFile::exists(kDataFile)) guarded([this] { manager_.load(kDataFile); });
    refreshAll();
}

void MainWindow::applyTheme(bool dark) {
    dark_ = dark;
    Theme t = themeFor(dark);
    QPalette pal;
    pal.setColor(QPalette::Window, t.bg);
    pal.setColor(QPalette::WindowText, t.text);
    pal.setColor(QPalette::Base, t.surface);
    pal.setColor(QPalette::AlternateBase, t.bar);
    pal.setColor(QPalette::Text, t.text);
    pal.setColor(QPalette::Button, t.bar);
    pal.setColor(QPalette::ButtonText, t.text);
    pal.setColor(QPalette::ToolTipBase, t.surface);
    pal.setColor(QPalette::ToolTipText, t.text);
    pal.setColor(QPalette::Highlight, t.primary);
    pal.setColor(QPalette::HighlightedText, Qt::white);
    qApp->setPalette(pal);
    qApp->setStyleSheet(scapms::styleSheet(t));
    themeBtn_->setIcon(makeIcon(dark ? "sun" : "moon", Qt::white));
    themeBtn_->setIconSize(QSize(20, 20));
}

void MainWindow::guarded(const std::function<void()>& fn) {
    try {
        fn();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", "Error: " + QString::fromUtf8(e.what()));
    } catch (...) {
        QMessageBox::critical(this, "Error", "An unexpected error occurred.");
    }
    refreshAll();
}

void MainWindow::refreshAll() {
    for (Panel* p : panels_) p->refresh();
    bool can = manager_.canUndo();
    undoAct_->setEnabled(can);
    undoAct_->setText(can ? "Undo: " + manager_.undoLabel() : "Undo Last Administrative Operation");
    counts_->setText(QString("Patients: %1  |  Waiting: %2")
                         .arg(manager_.db().records().size()).arg(manager_.waiting().size()));
}
}
