#pragma once
#include "model/Appointment.h"
namespace scapms {
class Appointable {
public:
    virtual ~Appointable() = default;
    virtual void scheduleAppointment(const Appointment& a) = 0;
    virtual bool cancelAppointment(int appointmentId) = 0;
    virtual int pendingCount() const = 0;
};
}
