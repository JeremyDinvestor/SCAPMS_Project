#include <QApplication>
#include "gui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");
    scapms::MainWindow w;
    w.show();
    return app.exec();
}
