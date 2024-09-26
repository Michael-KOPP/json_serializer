#pragma once
#include <type_traits>
#include <refl/refl.hpp>

template<typename serializer_impl>
class serializer
{
public:
    constexpr serializer() = default;

    template<refl::meta::reflector refl, typename T>
    constexpr auto serialize(refl const& reflector, T const& t) const -> decltype(auto) requires(std::is_base_of_v<typename refl::meta::reflector::inner_class, T>)
    {
        return static_cast<serializer_impl&>(*this).serialize(t);
    }
};
