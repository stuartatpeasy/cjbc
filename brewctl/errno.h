#ifndef ERRNO_H_INC
#define ERRNO_H_INC
/*
    errno.cc: base class providing error-number tracking/discovery functions

    Stuart Wallace <stuartw@atom.net>, September 2017.

    Part of brewctl
*/


class ErrNo
{
protected:
                        ErrNo();
    virtual             ~ErrNo();

public:
    int                 errNo() const { return errno_; };
    void                resetErrNo() { errno_ = 0; };

protected:
    int                 errno_;
};

#endif // ERRNO_H_INC

