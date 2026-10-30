#pragma once
#include <QColor>
#include <QString>
namespace scapms {
struct Theme {
    QColor bg, surface, text, muted, line, bar, primary, accent, primaryFg, side, sideFg;
};
Theme themeFor(bool dark);
QString styleSheet(const Theme& t);
}
