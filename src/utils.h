#ifndef CPPLOX_UTILS_H
#define CPPLOX_UTILS_H

#include <type_traits>

template<typename E>
constexpr auto to_integral(E e) -> typename std::underlying_type<E>::type {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

#endif //CPPLOX_UTILS_H
