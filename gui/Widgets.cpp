#include "gui/Widgets.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QVBoxLayout>
#include "gui/Icons.h"
namespace scapms {

StatTile::StatTile(const QString& icon, const QColor& c1, const QColor& c2, const QString& caption, QWidget* parent)
    : QFrame(parent) {
    setObjectName("card");
    auto* ico = new QLabel;
    ico->setFixedSize(48, 48);
    ico->setAlignment(Qt::AlignCenter);
    ico->setPixmap(iconPixmap(icon, Qt::white, 48).scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ico->setStyleSheet(QString("QLabel{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2);border-radius:14px;}")
                           .arg(c1.name(), c2.name()));
    value_ = new QLabel("0");
    value_->setObjectName("tileValue");
    auto* cap = new QLabel(caption);
    cap->setObjectName("tileCaption");
    auto* text = new QVBoxLayout;
    text->setSpacing(0);
    text->addWidget(value_);
    text->addWidget(cap);
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(16, 14, 16, 14);
    row->setSpacing(14);
    row->addWidget(ico);
    row->addLayout(text);
    row->addStretch();
}

void StatTile::setValue(const QString& v) { value_->setText(v); }

void BarChart::setBars(std::vector<Bar> bars, int total) {
    bars_ = std::move(bars);
    total_ = total;
    updateGeometry();
    update();
}

void BarChart::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int rowH = 30, labelW = 90, valueW = 34;
    QColor txt = palette().color(QPalette::WindowText);
    int y = 4;
    for (const auto& b : bars_) {
        p.setPen(txt);
        p.drawText(QRect(0, y, labelW, rowH), Qt::AlignVCenter | Qt::AlignLeft, b.label);
        QRectF track(labelW, y + rowH / 2 - 5, qMax(20, width() - labelW - valueW), 10);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(128, 128, 128, 55));
        p.drawRoundedRect(track, 5, 5);
        double f = total_ > 0 ? double(b.value) / total_ : 0.0;
        if (f > 0) {
            QRectF fill = track;
            fill.setWidth(qMax(10.0, track.width() * f));
            p.setBrush(b.color);
            p.drawRoundedRect(fill, 5, 5);
        }
        p.setPen(txt);
        p.drawText(QRect(width() - valueW, y, valueW, rowH), Qt::AlignVCenter | Qt::AlignRight, QString::number(b.value));
        y += rowH;
    }
}

namespace {
bool badgeColors(const QString& t, QColor& bg, QColor& fg) {
    struct E { const char* name; const char* bg; const char* fg; };
    static const E table[] = {
        {"Emergency", "#fee2e2", "#b91c1c"}, {"General", "#d1fae5", "#047857"}, {"Follow-Up", "#dbeafe", "#1d4ed8"},
        {"Pending", "#fef3c7", "#b45309"},   {"Completed", "#dcfce7", "#15803d"}, {"Cancelled", "#e5e7eb", "#4b5563"}};
    for (const auto& e : table)
        if (t == e.name) { bg = QColor(e.bg); fg = QColor(e.fg); return true; }
    return false;
}
}

void BadgeDelegate::paint(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QString text = index.data().toString();
    QColor bg, fg;
    if (!badgeColors(text, bg, fg)) { QStyledItemDelegate::paint(p, option, index); return; }
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.text.clear();
    QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, p, opt.widget);
    QFont f = opt.font;
    f.setBold(true);
    f.setPointSizeF(f.pointSizeF() - 1);
    QFontMetrics fm(f);
    QRect r(opt.rect.left() + 8, 0, fm.horizontalAdvance(text) + 22, 22);
    r.moveTop(opt.rect.center().y() - 11);
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(Qt::NoPen);
    p->setBrush(bg);
    p->drawRoundedRect(r, 11, 11);
    p->setPen(fg);
    p->setFont(f);
    p->drawText(r, Qt::AlignCenter, text);
    p->restore();
}
}
