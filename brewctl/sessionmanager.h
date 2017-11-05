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
#include "tempsensor.h"
#include <cstddef>
#include <vector>


class SessionManager
{
public:
                            SessionManager(Error * const err = nullptr) noexcept;
                            ~SessionManager() noexcept;

    bool                    init(Error * const err = nullptr) noexcept;
    void                    run() noexcept;

private:
    std::vector<Session *>  sessions_;
    TempSensor *            tempSensorAmbient_;
};

#endif // SESSIONMANAGER_H_INC

