#pragma once
#include <type_traits>
#include <optional>
#include <array>
#include <vector>
#include <map>

namespace dori::meta
{


    template<typename T>
    struct is_optional
    {
        static constexpr bool value = false;
    };

    template<typename T>
    struct is_optional<std::optional<T>>
    {
        static constexpr bool value = true;
        using type = T;
    };

    template<typename T>
    constexpr bool is_optional_v = is_optional<T>::value;

    template<typename T>
    using optional_value_t = is_optional<T>::type;

    template<typename T>
    struct is_std_array
    {
        static constexpr bool value = false;
    };

    template<typename T, size_t N>
    struct is_std_array<std::array<T, N>>
    {
        static constexpr bool value = true;
    };

    template<typename T>
    constexpr bool is_std_array_v = is_std_array<T>::value;

    template<typename T>
    struct is_vector
    {
        static constexpr bool value = false;
    };

    template<typename T, typename Alloc>
    struct is_vector<std::vector<T, Alloc>>
    {
        static constexpr bool value = true;
    };

    template<typename T>
    constexpr bool is_vector_v = is_vector<T>::value;

    template<typename T>
    struct is_map
    {
        static constexpr bool value = false;
    };

    template<typename K, typename T, typename Compare, typename Alloc>
    struct is_map<std::map<K, T, Compare, Alloc>>
    {
        static constexpr bool value = true;
    };

    template<typename T>
    constexpr bool is_map_v = is_map<T>::value;
}
