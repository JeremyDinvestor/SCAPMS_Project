#include "gui/Icons.h"
#include <QMap>
#include <QPainter>
#include <QSvgRenderer>
namespace scapms {
namespace {
const QMap<QString, QString>& paths() {
    static const QMap<QString, QString> p{
        {"logo", R"(<path d='M10 3h4v7h7v4h-7v7h-4v-7H3v-4h7z'/>)"},
        {"home", R"(<path d='M3 11l9-8 9 8v10a1 1 0 0 1-1 1h-5v-7H9v7H4a1 1 0 0 1-1-1z'/>)"},
        {"users", R"(<circle cx='9' cy='8' r='3.5'/><path d='M2 21v-1a6 6 0 0 1 12 0v1'/><path d='M16 4.5a3.5 3.5 0 0 1 0 7M18 14a6 6 0 0 1 4 6v1'/>)"},
        {"calendar", R"(<rect x='3' y='5' width='18' height='16' rx='2'/><path d='M3 10h18M8 3v4M16 3v4'/>)"},
        {"clock", R"(<circle cx='12' cy='12' r='9'/><path d='M12 7v5l3 2'/>)"},
        {"search", R"(<circle cx='11' cy='11' r='7'/><path d='M21 21l-4.3-4.3'/>)"},
        {"chart", R"(<path d='M4 20V10M10 20V4M16 20v-8M22 20H2'/>)"},
        {"plus", R"(<path d='M12 5v14M5 12h14'/>)"},
        {"edit", R"(<path d='M4 20h4L19 9l-4-4L4 16z'/>)"},
        {"trash", R"(<path d='M4 7h16M9 7V4h6v3M6 7l1 14h10l1-14'/>)"},
        {"x", R"(<path d='M6 6l12 12M18 6L6 18'/>)"},
        {"check", R"(<path d='M5 12l5 5L20 7'/>)"},
        {"play", R"(<path d='M6 4l14 8-14 8z'/>)"},
        {"refresh", R"(<path d='M20 12a8 8 0 1 1-3-6.2M20 4v5h-5'/>)"},
        {"sort", R"(<path d='M7 4v16M3 16l4 4 4-4M17 20V4M13 8l4-4 4 4'/>)"},
        {"alert", R"(<path d='M12 3l10 18H2z'/><path d='M12 10v5M12 18h.01'/>)"},
        {"moon", R"(<path d='M21 13A9 9 0 1 1 11 3a7 7 0 0 0 10 10z'/>)"},
        {"sun", R"(<circle cx='12' cy='12' r='4'/><path d='M12 2v2M12 20v2M2 12h2M20 12h2M5 5l1.5 1.5M17.5 17.5L19 19M5 19l1.5-1.5M17.5 6.5L19 5'/>)"}};
    return p;
}
}

QPixmap iconPixmap(const QString& name, const QColor& color, int size) {
    QString svg = QString("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='%1' "
                          "stroke-width='2' stroke-linecap='round' stroke-linejoin='round'>%2</svg>")
                      .arg(color.name(), paths().value(name));
    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    renderer.render(&p);
    return pm;
}

QIcon makeIcon(const QString& name, const QColor& color, const QColor& selected) {
    QIcon icon;
    icon.addPixmap(iconPixmap(name, color), QIcon::Normal);
    if (selected.isValid()) {
        QPixmap s = iconPixmap(name, selected);
        icon.addPixmap(s, QIcon::Selected);
        icon.addPixmap(s, QIcon::Active);
    }
    return icon;
}
}
