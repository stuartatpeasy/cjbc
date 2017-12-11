#ifndef SESSIONMANAGER_H_INC
#define SESSIONMANAGER_H_INC
/*
    sessionmanager.h: manages fermentation / conditioning sessions

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "include/application/session.h"
#include "include/framework/error.h"
#include "include/framework/registry.h"
#include "include/framework/thread.h"
#include "include/peripherals/defaulttempsensor.h"
#include <cstddef>
#include <memory>
#include <vector>


class SessionManager : public Thread
{
public:
                                SessionManager() noexcept;
                                SessionManager(const SessionManager& rhs) = delete;
                                SessionManager(SessionManager&& rhs) = delete;
                                ~SessionManager() noexcept;

    SessionManager&             operator=(const SessionManager& rhs) = delete;
    SessionManager&             operator=(SessionManager&& rhs) = delete;

    bool                        init(Error * const err = nullptr) noexcept;
    bool                        run() noexcept;

private:
    char                        getTempControlIndicator(const SessionTempControlState_t state) const noexcept;
    char                        getSessionTypeIndicator(const SessionType_t type) const noexcept;

    std::vector<Session *>      sessions_;
    DefaultTempSensor_uptr_t    tempSensorAmbient_;
    time_t                      lastDisplayUpdate_;
    time_t                      displayUpdateInterval_;
};

#endif // SESSIONMANAGER_H_INC

