#ifndef DEVICE_H_INC
#define DEVICE_H_INC
/*
    device.h: ABC collecting various device-generic stuff

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/


class Device
{
protected:
                        Device();
    virtual             ~Device();

public:
    int                 errNo() const { return errno_; };
    void                resetErrNo() { errno_ = 0; };

protected:
    int                 errno_;
};

#endif // DEVICE_H_INC

