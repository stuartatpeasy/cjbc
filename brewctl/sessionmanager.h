#ifndef SESSIONMANAGER_H_INC
#define SESSIONMANAGER_H_INC
/*
    sessionmanager.h: manages fermentation / conditioning sessions

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "error.h"
#include "registry.h"
#include "session.h"
#include "defaulttempsensor.h"
#include <cstddef>
#include <vector>


class SessionManager
{
public:
                            SessionManager() = default;
                            SessionManager(const SessionManager& rhs) = delete;
                            SessionManager(SessionManager&& rhs) = delete;
                            ~SessionManager() noexcept;

    SessionManager&         operator=(const SessionManager& rhs) = delete;
    SessionManager&         operator=(SessionManager&& rhs) = delete;

    bool                    init(Error * const err = nullptr) noexcept;
    void                    run() noexcept;

private:
    std::vector<Session *>  sessions_;
    std::unique_ptr<DefaultTempSensor>       tempSensorAmbient_;
};

#endif // SESSIONMANAGER_H_INC

