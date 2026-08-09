//functions to decompose structs into their constituents and vice versa



#include "Node.h"
#include "../../vulkan/EngineClasses/Lt_Console.h"
#include <string>
#include "boost/pfr.hpp"
#include <any>
#include <variant>
namespace NodeEditor
{
    //these are for the compose and decompose functions in the node editor system
    template <typename T>
    auto decomposeStruct(const T& obj) {
        //Get the names of all members as a compile-time std::array<std::string_view>
        constexpr auto names = boost::pfr::names_as_array<T>();

        //Unpack the struct values into a standard tuple
        auto values_tuple = boost::pfr::structure_to_tuple(obj);

        //Zip the names array and the values tuple together into a tuple of pairs
        return std::apply([&names](auto&&... elems) {
            size_t index = 0;
            return std::make_tuple(std::make_pair(names[index++], elems)...);
            }, values_tuple);
    }

    class composeStruct
    {
    public:
        // Registration blueprint: The compiler reads T, reflects its fields, 
        // and automatically writes a tailored input/creation routine under the hood.
        enum class ValidInputs
        {
            example
        };

        //turns out making a struct is a billion times harder than breaking one
        // ============================================================================
        // worse than epstein?!
        // ============================================================================
        // part 2 : the bullshit begins
        // 
        template <typename FieldType>
        void populate_field(FieldType& field, const VariantNode& source_node) {
            // determine at compile time if this specific field is a nested struct
            if constexpr (boost::pfr::tuple_size_v<FieldType> > 0 && !std::is_same_v<FieldType, std::string>) {
                // field is a struct! Extract the sub-vector and parse it recursively
                if (auto* sub_vector = std::get_if<std::vector<VariantNode>>(&source_node.data)) {
                    field = deserialize_complex<FieldType>(*sub_vector);
                }
                else {
                    throw std::runtime_error("Mismatched data complexity: Expected sub-vector payload for nested struct.");
                }
            }
            else {
                // field is a primitive type (int, double, string, bool)
                if (auto* val = std::get_if<FieldType>(&source_node.data)) {
                    field = *val;
                }
                else {
                    throw std::runtime_error("Type mismatch encountered during generic field extraction.");
                }
            }
        }

        //at this point no one , not even ai understands this bullshit
        template <typename T>
        T deserialize_complex(const std::vector<VariantNode>&sub_nodes) {
            if (sub_nodes.size() < boost::pfr::tuple_size_v<T>) {
                throw std::runtime_error("Insufficient structural variants passed for this target type.");
            }

            T struct_instance{};
            size_t data_index = 0;

            // Unroll the fields of struct T. The compiler automatically maps each individual field to its corresponding data handler.
            boost::pfr::for_each_field(struct_instance, [this,&sub_nodes, &data_index](auto& field) {
                populate_field(field, sub_nodes[data_index++]);
                });

            return struct_instance;
        }
        template <typename T>
        void register_type(const ValidInputs& type_key) {
            pipeline_[type_key] = [this](const std::vector<VariantNode>& root_payload) -> std::any {
                return std::any(deserialize_complex<T>(root_payload));
                };
        }

        std::any process(const ValidInputs& type_key, const std::vector<VariantNode>& raw_payload) {
            auto it = pipeline_.find(type_key);
            if (it == pipeline_.end()) {
                ::lte::Con::LogError("Unknown struct", HIGH_SEVERITY, TAG_ENGINE);
                return {};
            }
            return it->second(raw_payload);
        }
    private:
        std::unordered_map<ValidInputs, std::function<std::any(const std::vector<VariantNode>&)>> pipeline_;
        struct subStruct
        {
            int dat1;
            int dat2;
        };
        struct exampleStruct
        {
            int integer;
            bool boolean;
            subStruct substruct;
        };

        void registerexample()
        {
            register_type<exampleStruct>(ValidInputs::example);
        }

        void processexample(std::vector<VariantNode> datastream)
        {
            try {
                std::any response = process(ValidInputs::example, datastream);

                if (response.has_value()) {
                    auto report = std::any_cast<exampleStruct>(response);

                    std::cout << "Successfully Ingested Polymorphic Struct Tree!\n";
                    //std::cout << " -> string    : " << report.str << "\n";
                    std::cout << " -> integer   : " << report.integer << "\n";
                    std::cout << " -> boolean   : " << std::boolalpha << report.boolean << "\n";
                    std::cout << " -> substruct1: " << report.substruct.dat1 << "\n";
                    std::cout << " -> substruct2: " << report.substruct.dat2 << "\n";
                }
            }
            catch (const std::exception& ex) {
                std::cerr << "Pipeline runtime error: " << ex.what() << "\n";
            }
        }

    };

}