#include <gtest/gtest.h>
#include <serialization/json_serializer.hpp>

struct data
{
    std::string _token;
};

auto operator==(data const& data1, data const& data2) -> bool {
    return data1._token == data2._token;
}

struct response
{
    bool _success;
    std::optional<std::map<std::string, data>> _data;
};

struct foo
{
    std::map<std::string, std::optional<std::vector<data>>> _f;
};

auto operator==(foo const& foo1, foo const& foo2) -> bool {
    return foo1._f == foo2._f;
}

template<typename T>
struct registry {};

template<>
struct registry<data>
{
    static constexpr auto reflector = refl::refl<data>("data")
            .add("token", &data::_token);
};

template<>
struct registry<response>
{
    static constexpr auto reflector = refl::refl<response>("response")
            .add("success", &response::_success)
            .add("data", &response::_data);
};

template<>
struct registry<foo>
{
    static constexpr auto reflector = refl::refl<foo>("foo")
            .add("f", &foo::_f);
};

TEST(JsonSerialization, FullJson)
{
    bool succeeded = false;
    try
    {
        std::string const json = "{"
                                 "\"success\":true, "
                                 "\"data\":"
                                 "  ["
                                 "      {"
                                 "          \"1\":"
                                 "              {"
                                 "                  \"token\":\"hey\""
                                 "              }"
                                 "      }"
                                 "  ]"
                                 "}";
        auto response = from_json<class response, registry>(json);
        succeeded = response._success &&
                    response._data.has_value() &&
                    response._data->find("1") != response._data->end() &&
                    response._data->at("1")._token == "hey";
    }
    catch(std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }

    ASSERT_TRUE(succeeded);
}

TEST(JsonSerialization, EmptyString)
{
    bool succeeded = false;
    try
    {
        std::string const json = "{"
                                 "\"success\":true, "
                                 "\"data\":"
                                 "  ["
                                 "      {"
                                 "          \"1\":"
                                 "              {"
                                 "                  \"token\":\"\""
                                 "              }"
                                 "      },"
                                 "      {"
                                 "          \"2\":"
                                 "              {"
                                 "                  \"token\":\"gloup\""
                                 "              }"
                                 "      }"
                                 "  ]"
                                 "}";
        auto response = from_json<class response, registry>(json);
        succeeded = response._success &&
                    response._data.has_value() &&
                    response._data->find("1") != response._data->end() &&
                    response._data->at("1")._token.empty() &&
                    response._data->find("2") != response._data->end() &&
                    response._data->at("2")._token == "gloup";
    }
    catch(std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }

    ASSERT_TRUE(succeeded);
}

TEST(JsonSerialization, NullData)
{
    bool succeeded = false;
    try
    {
        std::string const json = "{"
                                 "\"success\":true, "
                                 "\"data\":null"
                                 "}";
        auto response = from_json<class response, registry>(json);

        succeeded = response._success && response._data.has_value() == false;
    }
    catch(std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }

    ASSERT_TRUE(succeeded);
}

TEST(JsonSerialization, MapSerialization)
{
    bool succeeded = false;
    try
    {
        response r;
        r._success = true;
        r._data = std::map<std::string, data>{{"1",{._token = ""}},{"2", {._token="gloup"}}};

        auto a = to_json<response, registry>(r);

        std::string const str = "{"
                                 "\"success\":true, "
                                 "\"data\":"
                                 "  ["
                                 "      {"
                                 "          \"1\":"
                                 "              {"
                                 "                  \"token\":\"\""
                                 "              }"
                                 "      },"
                                 "      {"
                                 "          \"2\":"
                                 "              {"
                                 "                  \"token\":\"gloup\""
                                 "              }"
                                 "      }"
                                 "  ]"
                                 "}";

        succeeded = nlohmann::json::parse(a) == nlohmann::json::parse(str);
    }
    catch(std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }

    ASSERT_TRUE(succeeded);
}

TEST(JsonSerialization, SerializationDeserializationIsIdentity)
{
    bool succeeded = false;
    try
    {
        foo f{std::map<std::string, std::optional<std::vector<data>>>{
            {"cccccccccc", std::nullopt},
            {"bbbbbbbbbb", {std::vector<data>{data{._token="hey1"}, data{._token="hey2"}}}},
            {"eeeeeeeeee", {std::vector<data>{data{._token="hey6"}, data{._token="hey5"}, data{._token="hey4"}, data{._token="hey3"}}}}
        }};

        auto a = to_json<foo, registry>(f);
        std::cout << a << std::endl;
        auto b = from_json<foo, registry>(a);

        succeeded = (f == b);
    }
    catch(std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }

    ASSERT_TRUE(succeeded);
}

TEST(JsonSerialization, DeserializationSerializationIsIdentity)
{
    bool succeeded = false;
    try
    {
        std::string json_string =   "{                                      "
                                    "\"f\": [                               "
                                    "   {                                   "
                                    "        \"bbbbbbbbbb\": [              "
                                    "            {                          "
                                    "                \"token\": \"hey1\"    "
                                    "            },                         "
                                    "            {                          "
                                    "                \"token\": \"hey2\"    "
                                    "            }                          "
                                    "        ]                              "
                                    "    },                                 "
                                    "    {                                  "
                                    "        \"cccccccccc\": null           "
                                    "    },                                 "
                                    "    {                                  "
                                    "        \"eeeeeeeeee\": [              "
                                    "            {                          "
                                    "                \"token\": \"hey6\"    "
                                    "            },                         "
                                    "            {                          "
                                    "                \"token\": \"hey5\"    "
                                    "            },                         "
                                    "            {                          "
                                    "                \"token\": \"hey4\"    "
                                    "            },                         "
                                    "            {                          "
                                    "                \"token\": \"hey3\"    "
                                    "            }                          "
                                    "        ]                              "
                                    "    }                                  "
                                    "]                                      "
                                    "}                                      ";

        auto b = from_json<foo, registry>(json_string);
        auto a = to_json<foo, registry>(b);

        succeeded = (nlohmann::json::parse(json_string) == nlohmann::json::parse(a));
    }
    catch(std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }

    ASSERT_TRUE(succeeded);
}
