#pragma once

template<typename T>
bool is(const StatementPtr& p)
{
    return dynamic_cast<T*>(p.get());
}

template<typename T>
T* cast(const StatementPtr& p)
{
    return static_cast<T*>(p.get());
}

