#pragma once
#include <vector>
#include <QFrame>
#include <QStyledItemDelegate>
class QLabel;
namespace scapms {

class StatTile : public QFrame {
public:
    StatTile(const QString& icon, const QColor& c1, const QColor& c2, const QString& caption, QWidget* parent = nullptr);
    void setValue(const QString& v);
private:
    QLabel* value_;
};

class BarChart : public QWidget {
public:
    struct Bar { QString label; int value; QColor color; };
    explicit BarChart(QWidget* parent = nullptr) : QWidget(parent) {}
    void setBars(std::vector<Bar> bars, int total);
    QSize sizeHint() const override { return QSize(240, static_cast<int>(bars_.size()) * 30 + 8); }
    QSize minimumSizeHint() const override { return sizeHint(); }
protected:
    void paintEvent(QPaintEvent*) override;
private:
    std::vector<Bar> bars_;
    int total_ = 0;
};

class BadgeDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};
}
