#ifndef FRAMEWORK_THREAD_H_INC
#define FRAMEWORK_THREAD_H_INC
/*
    thread.h: interface defining a class intended to run in its own thread

    Stuart Wallace <stuartw@atom.net>, November 2017.

    Part of brewctl
*/

#include <string>


class Thread
{
public:
                        Thread() noexcept;
    virtual             ~Thread() = default;

    virtual void        run() noexcept = 0;
    void                stop() noexcept;
    bool                isRunning() const noexcept { return running_; };
    bool                setName(const std::string& name) noexcept;

protected:
    volatile bool       stop_;
    volatile bool       running_;
};

#endif // FRAMEWORK_THREAD_H_INC

