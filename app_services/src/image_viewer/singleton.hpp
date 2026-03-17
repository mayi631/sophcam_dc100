#pragma once

namespace image_viewer {

template <class T>
class Singleton
{
public:
    static const T& getInstance()
    {
        static T instance;
        return instance;
    }

    // not copyable
    Singleton(const Singleton&) = delete;
    void operator=(const Singleton&) = delete;

protected:
    Singleton() = default;
    virtual ~Singleton() = default;
};

} // namespace image_viewer
