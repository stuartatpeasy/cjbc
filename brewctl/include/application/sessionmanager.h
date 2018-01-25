#ifndef APPLICATION_SESSIONMANAGER_H_INC
#define APPLICATION_SESSIONMANAGER_H_INC
/*
    sessionmanager.h: manages fermentation / conditioning sessions

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "include/application/session.h"
#include "include/application/display.h"
#include "include/framework/error.h"
#include "include/framework/registry.h"
#include "include/framework/thread.h"
#include "include/peripherals/defaulttempsensor.h"
#include <memory>
#include <map>


typedef std::map<session_id_t, Session *> SessionMap_t;

class SessionManager : public Thread
{
public:
                                SessionManager() noexcept;
                                SessionManager(const SessionManager& rhs) = delete;
                                SessionManager(SessionManager&& rhs) = delete;
                                ~SessionManager() noexcept;

    SessionManager&             operator=(const SessionManager& rhs) = delete;
    SessionManager&             operator=(SessionManager&& rhs) = delete;

    Temperature                 ambientTemp() noexcept;
    const SessionMap_t&         sessions() noexcept;

    bool                        init(Error * const err = nullptr) noexcept;
    bool                        run() noexcept override;

private:
    bool                        updateSessionList(Error * const err = nullptr) noexcept;

    SessionMap_t                sessions_;
    DefaultTempSensor_uptr_t    tempSensorAmbient_;
    Display *                   display_;
};

#endif // APPLICATION_SESSIONMANAGER_H_INC

