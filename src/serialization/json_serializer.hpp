#pragma once
#include "serializer.hpp"
#include "traits.hpp"

#include <string>
#include <ranges>
#include <algorithm>
#include <optional>

#include <refl/refl.hpp>
#include <refl/registry.hpp>
#include <nlohmann/json.hpp>

template<refl::meta::reflector reflector>
class json_serializer : public serializer<json_serializer<reflector>>
{
public:

    constexpr json_serializer(reflector const& refl) : _reflector(refl) {}

    /**
     * @brief Serialize a type T into a string
     * @return The serialized string
     */
    template<typename T, template<typename> typename Registry>
    auto serialize(T const& t) const -> std::string requires(std::is_base_of_v<typename reflector::inner_class, T>) {

        return serialize_obj<T, Registry>(t).dump();
    }

    template<typename T, template<typename> typename Registry>
    auto serialize_obj(T const& t) const -> decltype(auto) requires(std::is_base_of_v<typename reflector::inner_class, T>) {

        return refl::apply([&] (auto&& ... args) {
            nlohmann::json json_obj = {
                                            to_json_pair<std::remove_cvref_t<decltype(t.*args.ptr())>, Registry>(args.name(), t.*args.ptr())...
                                      };
            return json_obj;
        }, _reflector);
    }

    /**
     * @brief Deserialize a string into type T
     * @return The deserialized string as T
     *
     * @throws nlohmann::json::parse_error if the string is not json
     * @throws nlohmann::json::out_of_range if the requested reflected data does not exist
     * @throws nlohmann::json::type_error if the requested reflected data has not the same type
     */
    template<typename T, template<typename> typename Registry>
    constexpr auto deserialize(std::string const& json_string) const -> T requires(std::is_base_of_v<typename reflector::inner_class, T>) {
        return deserialize_obj<T, Registry>(nlohmann::json::parse(json_string));
    }

    template<typename T, template<typename> typename Registry>
    constexpr auto deserialize_obj(nlohmann::json const& node) const -> decltype(auto) requires(std::is_base_of_v<typename reflector::inner_class, T> && std::is_default_constructible_v<T>) {
        return refl::apply([&](auto&& ... args){
            T t;
            ((t.*args.ptr() = from_json_value<std::remove_cvref_t<decltype(t.*args.ptr())>, Registry>(node.at(args.name()))), ...);
            return t;//RVO
        }, _reflector);
    }
private:
    reflector const& _reflector;
};

template<typename T, template<typename> typename Registry> requires(refl::registered<T, Registry>)
auto get_serializer() -> decltype(auto) {
    static constexpr json_serializer serializer = json_serializer(Registry<T>::reflector);

    return serializer;
}

template<typename T, template<typename> typename Registry>
auto to_json_pair(std::string_view const key, T const& typed_value) -> std::pair<std::string_view, nlohmann::json> {
    auto v = to_json_value<T, Registry>(typed_value);
    using v_type = std::remove_cvref_t<decltype(v)>;
    if constexpr(dori::meta::is_optional_v<v_type>)
    {
        if(v.has_value())
        {
            return std::make_pair(key, nlohmann::json(to_json_value<typename v_type::value_type, Registry>(v.value())));
        }
        else
        {
            return std::make_pair(key, nlohmann::json(nullptr));
        }
    }
    else
    {
        return std::make_pair(key, nlohmann::json(v));
    }
}

template<typename T, template<typename> typename Registry>
auto to_json_value(T const& typed_value) -> decltype(auto) {
    if constexpr (std::is_pointer_v<std::decay_t<T>> == false &&
                  std::is_standard_layout_v<T> &&
                  refl::registered<T, Registry>)
    {
        auto const& serializer = get_serializer<T, Registry>();
        return serializer.template serialize_obj<T, Registry>(typed_value);
    }
    else if constexpr (std::is_fundamental_v<T>)
    {
        return typed_value;
    }
    else if constexpr (std::is_convertible_v<T, std::string const&>)
    {
        return static_cast<std::string const&>(typed_value);
    }
    else if constexpr (std::ranges::range<T>)//Maybe care with wstring etc... as you want them to be serialized as std::string
    {
        if constexpr(dori::meta::is_map_v<T>)
        {
            static_assert(std::is_same_v<typename T::key_type, std::string>, "Please, only use std::map with key_type as std::string");
            nlohmann::json data_array = nlohmann::json::array();

            for(auto& value : typed_value)
            {
                nlohmann::json obj = {to_json_pair<typename T::value_type::second_type, Registry>(value.first, value.second)};

                data_array.push_back(obj);
            }

            return data_array;
        }
        else
        {
            nlohmann::json data_array = nlohmann::json::array();

            for(auto& value : typed_value)
            {
                data_array.push_back(to_json_value<typename T::value_type, Registry>(value));
            }

            return data_array;
        }
    }
    else if constexpr (dori::meta::is_optional_v<T>)
    {
        return typed_value;
    }
    else
    {
        static_assert(std::is_pointer_v<std::decay_t<T>> == false &&
                        std::is_standard_layout_v<T> &&
                        refl::registered<T, Registry> ||
                        std::is_fundamental_v<T> ||
                        std::is_convertible_v<T, std::string const&> ||
                        std::ranges::range<T> ||
                        dori::meta::is_optional_v<T>,
                        "Type cannot be reflected and is not a range. Please provide a reflector for this class or a string conversion function.");
    }
}

template<typename T, template<typename> typename Registry>
auto from_json_value(nlohmann::json const& node) -> decltype(auto) {
    if constexpr(std::is_assignable_v<T, std::string const&>)
    {
        return node.get<std::string>();
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        return node.get<double>();
    }
    else if constexpr (std::is_integral_v<T>)
    {
        return node.get<int>();
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
        return node.get<bool>();
    }
    else if constexpr ( std::is_pointer_v<std::decay_t<T>> == false &&
                        std::is_standard_layout_v<T> &&
                        refl::registered<T, Registry>)
    {
        auto const& serializer = get_serializer<T, Registry>();
        return serializer.template deserialize_obj<T, Registry>(node);
    }
    else if constexpr (std::ranges::range<T>)
    {
        T tarray;

        if constexpr(std::is_array_v<T> || dori::meta::is_std_array_v<T>)
        {
            size_t i = 0;
            for(auto& t : tarray)
            {
                t = from_json_value<typename T::value_type, Registry>(node[i++]);
            }
        }
        else if constexpr(dori::meta::is_vector_v<T>)
        {
            for(auto& v : node)
            {
                tarray.push_back(from_json_value<typename T::value_type, Registry>(v));
            }
        }
        else if constexpr(dori::meta::is_map_v<T> && std::is_same_v<typename T::key_type, std::string>)
        {
            for(auto& v : node)
            {
                tarray[v.begin().key()] = from_json_value<typename T::value_type::second_type, Registry>(v.begin().value());
            }
        }
        else
        {
            static_assert(std::is_array_v<T> || dori::meta::is_std_array_v<T> || dori::meta::is_vector_v<T> || dori::meta::is_map_v<T>  && std::is_same_v<typename T::key_type, std::string>, "array is not supported by the json deserializer, please use std::array, std::vector, std::map with std::string key or plain array");
        }

        return tarray;
    }
    else if constexpr (std::is_pointer_v<T>)
    {
        if(node.is_null())
            return nullptr;
        else {
            return nullptr;
            // auto const& serializer = get_serializer<T, Registry>();
            // return new from_json_value<dori::meta::optional_value_t<T>, Registry>(node);
        }
    }
    else if constexpr (dori::meta::is_optional_v<T>)
    {
        if(node.is_null())
        {
            return T(std::nullopt);
        }
        else {
            return T(from_json_value<typename T::value_type, Registry>(node));
        }
    }
    else
    {
        static_assert(std::is_pointer_v<std::decay_t<T>> == false &&
                              std::is_standard_layout_v<T> &&
                              refl::registered<T, Registry> ||
                std::is_assignable_v<T, std::string const&> ||
                std::is_floating_point_v<T> ||
                std::is_integral_v<T> ||
                std::is_same_v<T, bool> ||
                std::is_assignable_v<T, std::nullptr_t> ||
                std::is_assignable_v<T, std::nullopt_t> ||
                std::ranges::range<T>,
                      "Type cannot be reflected. Please provide a reflector for this class ");
    }
}

template <std::ranges::range Range>
auto range_to_vector(const Range& range) {
    std::vector<std::ranges::range_value_t<Range>> result;
    std::ranges::copy(range, std::back_inserter(result));
    return result;
}

template<typename T, template<typename> typename Registry> requires(refl::registered<T, Registry>)
auto to_json(T const& b) -> std::string {
    auto& serializer = get_serializer<T, Registry>();
    return serializer.template serialize<T, Registry>(b);
}

template<typename T, template<typename> typename Registry> requires(refl::registered<T, Registry>)
auto from_json(std::string const& json_string) -> T {
    auto& serializer = get_serializer<T, Registry>();
    return serializer.template deserialize<T, Registry>(json_string);
}
