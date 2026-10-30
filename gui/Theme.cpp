#include "gui/Theme.h"
#include <QMap>
namespace scapms {

Theme themeFor(bool dark) {
    if (dark)
        return {QColor("#0b1219"), QColor("#141d27"), QColor("#e5edf5"), QColor("#8da2b5"), QColor("#243241"), QColor("#1b2631"),
                QColor("#2dd4bf"), QColor("#38bdf8"), QColor("#04201d"), QColor("#0a1620"), QColor("#8da2b5")};
    return {QColor("#eef3f8"), QColor("#ffffff"), QColor("#1e293b"), QColor("#64748b"), QColor("#e2e8f0"), QColor("#f1f5f9"),
            QColor("#0d9488"), QColor("#0891b2"), QColor("#ffffff"), QColor("#0f2a3a"), QColor("#b6c7d4")};
}

QString styleSheet(const Theme& t) {
    QString ss = R"(
* { font-family: "Segoe UI", "Inter", sans-serif; font-size: 13px; color: @TEXT; }
QMainWindow, QWidget#central { background: @BG; }
QWidget#page, QStackedWidget { background: @BG; }
QFrame#header { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #0f766e, stop:0.6 #0891b2, stop:1 #4f46e5); }
QFrame#header QLabel { color: white; background: transparent; }
QLabel#logo { background: rgba(255,255,255,0.18); border-radius: 12px; }
QLabel#brand { font-size: 18px; font-weight: 700; }
QLabel#brandSub { color: rgba(255,255,255,0.85); }
QToolButton#themeBtn { background: rgba(255,255,255,0.18); border: none; border-radius: 10px; padding: 8px; }
QToolButton#themeBtn:hover { background: rgba(255,255,255,0.30); }
QListWidget#sidebar { background: @SIDE; border: none; padding: 12px 8px; outline: 0; }
QListWidget#sidebar::item { color: @SIDEFG; padding: 11px 12px; border-radius: 10px; margin: 2px 0; }
QListWidget#sidebar::item:hover { background: rgba(255,255,255,0.08); color: white; }
QListWidget#sidebar::item:selected { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 @PRIMARY, stop:1 @ACCENT); color: white; }
QFrame#card { background: @SURFACE; border: 1px solid @LINE; border-radius: 14px; }
QFrame#card QLabel { background: transparent; }
QLabel#pageTitle { font-size: 22px; font-weight: 700; }
QLabel#pageSub { color: @MUTED; }
QLabel#cardTitle { font-size: 14px; font-weight: 700; }
QLabel#tileValue { font-size: 26px; font-weight: 700; }
QLabel#tileCaption { color: @MUTED; }
QLabel#error { color: #dc2626; font-size: 12px; }
QLabel#serving { font-size: 16px; font-weight: 700; color: @PRIMARY; }
QLabel#infoBar { background: @SURFACE; border: 1px solid @LINE; border-left: 4px solid @PRIMARY; border-radius: 10px; padding: 8px 12px; }
QLineEdit, QComboBox, QDateEdit, QTimeEdit, QSpinBox { background: @SURFACE; border: 1px solid @LINE; border-radius: 9px; padding: 7px 10px; min-height: 20px; }
QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QTimeEdit:focus, QSpinBox:focus { border: 1px solid @PRIMARY; }
QComboBox::drop-down { border: none; width: 24px; }
QComboBox QAbstractItemView { background: @SURFACE; border: 1px solid @LINE; selection-background-color: @PRIMARY; selection-color: white; }
QPushButton { background: @BAR; border: 1px solid @LINE; border-radius: 9px; padding: 8px 14px; font-weight: 600; }
QPushButton:hover { border-color: @PRIMARY; }
QPushButton[primary="true"] { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 @PRIMARY, stop:1 @ACCENT); color: @PRIMARYFG; border: none; }
QTableWidget { background: @SURFACE; border: 1px solid @LINE; border-radius: 14px; gridline-color: transparent; alternate-background-color: @BAR; selection-background-color: rgba(13,148,136,0.22); selection-color: @TEXT; outline: 0; }
QTableWidget::item { padding: 6px; border-bottom: 1px solid @LINE; }
QHeaderView::section { background: @BAR; color: @MUTED; border: none; padding: 10px; font-weight: 700; }
QListWidget#plain { background: transparent; border: none; }
QTextEdit { background: @SURFACE; border: 1px solid @LINE; border-radius: 14px; padding: 12px; }
QMenuBar { background: @SURFACE; border-bottom: 1px solid @LINE; }
QMenuBar::item { padding: 6px 12px; border-radius: 6px; background: transparent; }
QMenuBar::item:selected { background: @BAR; }
QMenu { background: @SURFACE; border: 1px solid @LINE; padding: 6px; }
QMenu::item { padding: 7px 24px; border-radius: 6px; }
QMenu::item:selected { background: @PRIMARY; color: white; }
QStatusBar { background: @SURFACE; border-top: 1px solid @LINE; color: @MUTED; }
QToolTip { background: @SURFACE; color: @TEXT; border: 1px solid @LINE; padding: 4px; }
QScrollBar:vertical { background: transparent; width: 10px; }
QScrollBar::handle:vertical { background: @LINE; border-radius: 5px; min-height: 30px; }
QScrollBar::add-line, QScrollBar::sub-line { height: 0; width: 0; }
)";
    const QList<QPair<QString, QColor>> tokens{
        {"@PRIMARYFG", t.primaryFg}, {"@PRIMARY", t.primary}, {"@ACCENT", t.accent}, {"@SURFACE", t.surface},
        {"@SIDEFG", t.sideFg}, {"@SIDE", t.side}, {"@TEXT", t.text}, {"@MUTED", t.muted},
        {"@LINE", t.line}, {"@BAR", t.bar}, {"@BG", t.bg}};
    for (const auto& tk : tokens) ss.replace(tk.first, tk.second.name());
    return ss;
}
}
