#pragma once

#include <windows.h>
#include <fwpmu.h>

class CnmEngine
{
public:
    CnmEngine() = default;
    ~CnmEngine();

    CnmEngine(const CnmEngine &) = delete;
    CnmEngine &operator=(const CnmEngine &) = delete;

    bool open();
    void close();

    bool is_open() const { return engine_ != nullptr; }
    HANDLE handle() const { return engine_; }

private:
    bool enable_collection();

    HANDLE engine_ = nullptr;
};