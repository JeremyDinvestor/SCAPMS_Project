#pragma once
#include <stdexcept>
#include <QString>
namespace scapms {
class ClinicException : public std::runtime_error {
public:
    explicit ClinicException(const QString& m) : std::runtime_error(m.toStdString()), msg_(m) {}
    QString message() const { return msg_; }
private:
    QString msg_;
};
}
