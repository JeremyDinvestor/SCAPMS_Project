#pragma once
#include <functional>
#include <vector>
#include <QMainWindow>
#include "controller/ClinicManager.h"

class QListWidget;
class QStackedWidget;
class QToolButton;
class QLabel;
class QAction;

namespace scapms {
class Panel;
class SearchPanel;

class MainWindow : public QMainWindow {
public:
    MainWindow();
private:
    void refreshAll();
    void guarded(const std::function<void()>& fn);
    void applyTheme(bool dark);
    ClinicManager manager_;
    QListWidget* sidebar_;
    QStackedWidget* stack_;
    QToolButton* themeBtn_;
    SearchPanel* search_;
    std::vector<Panel*> panels_;
    QLabel* counts_;
    QAction* undoAct_;
    bool dark_ = false;
};
}
