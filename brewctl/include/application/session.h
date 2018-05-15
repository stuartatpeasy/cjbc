#ifndef APPLICATION_SESSION_H_INC
#define APPLICATION_SESSION_H_INC
/*
    session.h: manages a single fermentation/conditioning session

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "include/application/temperature.h"
#include "include/framework/error.h"
#include "include/peripherals/defaulteffector.h"
#include "include/peripherals/defaulttempsensor.h"
#include <ctime>        // ::time()
#include <map>
#include <memory>
#include <string>
#include <vector>

typedef struct SessionStage
{
    time_t          duration;
    double          temperature;
    bool            forever;
} SessionStage_t;

typedef std::vector<SessionStage_t> SessionStages_t;
typedef int session_id_t;

typedef enum SessionTempControlState
{
    UNKNOWN,
    FAST_COOL,
    COOL,
    HOLD,
    HEAT,
    FAST_HEAT
} SessionTempControlState_t;

typedef enum SessionType
{
    NONE = 0,
    FERMENT,
    CONDITION,
    SERVE
} SessionType_t;


class Session
{
public:
                                Session(const session_id_t id, Error * const err = nullptr) noexcept;
    virtual                     ~Session() = default;

    Temperature                 targetTemp() noexcept;
    Temperature                 currentTemp() noexcept;
    bool                        vesselTempSensorInRange() const noexcept;
    bool                        isNotStartedYet() const noexcept;
    bool                        isActive() const noexcept;
    bool                        isComplete() const noexcept { return complete_; };
    bool                        iterate(Error * const err = nullptr) noexcept;
    void                        stop() noexcept;
    int                         gyleId() const noexcept { return gyle_id_; };
    std::string                 gyleName() const noexcept { return gyle_; };
    time_t                      remainingTime() const noexcept;
    SessionTempControlState_t   tempControlState() const noexcept { return tempControlState_; };
    SessionType_t               type() const noexcept { return type_; };
    bool                        markComplete(Error * const err) noexcept;

private:
    bool                        updateEffectors(Error * const err = nullptr) noexcept;
    bool                        deactivateEffectors() noexcept;

    const session_id_t          id_;
    int                         gyle_id_;
    std::string                 gyle_;
    int                         profile_;
    time_t                      start_ts_;
    time_t                      end_ts_;
    double                      deadZone_;
    time_t                      effectorUpdateInterval_;
    time_t                      lastEffectorUpdate_;
    SessionStages_t             stages_;
    DefaultTempSensor_uptr_t    tempSensorVessel_;
    DefaultEffector_uptr_t      effectorHeater_;
    DefaultEffector_uptr_t      effectorCooler_;
    SessionTempControlState_t   tempControlState_;
    SessionType_t               type_;
    bool                        complete_;
};

#endif // APPLICATION_SESSION_H_INC

