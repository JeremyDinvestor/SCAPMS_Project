#pragma once
#include <QColor>
#include <QIcon>
#include <QPixmap>
namespace scapms {
QPixmap iconPixmap(const QString& name, const QColor& color, int size = 64);
QIcon makeIcon(const QString& name, const QColor& color, const QColor& selected = QColor());
}
