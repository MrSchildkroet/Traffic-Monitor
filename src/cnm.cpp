// cnm.cpp
#include "cnm.hpp"

#include <iostream>

CnmEngine::~CnmEngine()
{
    close();
}

bool CnmEngine::open()
{
    if (engine_)
    {
        return true;
    }

    DWORD result = FwpmEngineOpen0(
        nullptr, RPC_C_AUTHN_WINNT, nullptr, nullptr, &engine_);

    if (result != ERROR_SUCCESS)
    {
        std::cerr << "[CNM] FwpmEngineOpen0 failed: " << result << "\n";
        engine_ = nullptr;

        return false;
    }

    if (!enable_collection())
    {
        close();

        return false;
    }

    std::cout << "[CNM] WFP engine connected, net-event collection enabled\n";

    return true;
}

bool CnmEngine::enable_collection()
{
    FWP_VALUE0 value = {};
    value.type = FWP_UINT32;
    value.uint32 = 1;

    DWORD result = FwpmEngineSetOption0(
        engine_, FWPM_ENGINE_COLLECT_NET_EVENTS, &value);

    if (result != ERROR_SUCCESS)
    {
        std::cerr << "[CNM] enable collection failed: " << result << "\n";

        return false;
    }

    return true;
};

void CnmEngine::close()
{
    if (engine_)
    {
        FwpmEngineClose0(engine_);
        engine_ = nullptr;
    }
}