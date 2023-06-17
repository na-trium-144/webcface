#pragma once
#include "../registration.hpp"
#include <memory>
#include <string>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

namespace WebCFace
{
// https://stackoverflow.com/questions/23963978/c-protobuf-how-to-iterate-through-fields-of-message
template <typename MessageDef>
void addProtoBufValueFromRobot(MessageDef* m, std::string base_name = "")
{
    using namespace google::protobuf;
    const Descriptor* desc = m->GetDescriptor();
    const Reflection* refl = m->GetReflection();
    int fieldCount = desc->field_count();
    // fprintf(stderr, "The fullname of the message is %s \n", desc->full_name().c_str());
    for (int i = 0; i < fieldCount; i++) {
        const FieldDescriptor* field = desc->field(i);
        // fprintf(stderr, "The name of the %i th element is %s and the type is  %s \n", i,
        //     field->name().c_str(), field->type_name());
        std::string full_name;
        if(base_name != ""){
            full_name = base_name + "." + field->name();
        }else{
            full_name = field->name();
        }
        if (field->is_repeated()) {
            std::string full_name_without_idx = full_name;
            int field_size = refl->FieldSize(*m, field);
            for (int j = 0; j < field_size; j++) {
                full_name = full_name_without_idx + "[" + std::to_string(j) + "]";
                using T = FieldDescriptor::CppType;
                switch (field->cpp_type()) {
                case T::CPPTYPE_INT32:
                    addValueFromRobot(full_name, refl->GetRepeatedInt32(*m, field, j));
                    break;
                case T::CPPTYPE_INT64:
                    addValueFromRobot(full_name, refl->GetRepeatedInt64(*m, field, j));
                    break;
                case T::CPPTYPE_UINT32:
                    addValueFromRobot(full_name, refl->GetRepeatedUInt32(*m, field, j));
                    break;
                case T::CPPTYPE_UINT64:
                    addValueFromRobot(full_name, refl->GetRepeatedUInt64(*m, field, j));
                    break;
                case T::CPPTYPE_FLOAT:
                    addValueFromRobot(full_name, refl->GetRepeatedFloat(*m, field, j));
                    break;
                case T::CPPTYPE_DOUBLE:
                    addValueFromRobot(full_name, refl->GetRepeatedDouble(*m, field, j));
                    break;
                case T::CPPTYPE_BOOL:
                    addValueFromRobot(full_name, refl->GetRepeatedBool(*m, field, j));
                    break;
                case T::CPPTYPE_STRING:
                    addValueFromRobot(full_name, refl->GetRepeatedString(*m, field, j));
                    break;
                case T::CPPTYPE_ENUM:
                    addValueFromRobot(
                        full_name, refl->GetRepeatedEnum(*m, field, j)->name());
                    break;
                case T::CPPTYPE_MESSAGE: {
                    const Message& mfield = refl->GetRepeatedMessage(*m, field, j);
                    // Message* mcopy = mfield.New();
                    // mcopy->CopyFrom(mfield);
                    // void* ptr = new std::shared_ptr<Message>(mcopy);
                    // std::shared_ptr<Message>* m = static_cast<std::shared_ptr<Message>*>(ptr);
                    // addProtoBufValueFromRobot(*m);
                    addProtoBufValueFromRobot(&mfield, full_name);
                    break;
                }
                default:
                    std::cerr << "[WebCFace] Protobuf Unsupported Field Type? "
                              << full_name << ", type = " << field->type() << std::endl;
                    break;
                }
            }
        } else {
            using T = FieldDescriptor::CppType;
            switch (field->cpp_type()) {
            case T::CPPTYPE_INT32:
                addValueFromRobot(full_name, refl->GetInt32(*m, field));
                break;
            case T::CPPTYPE_INT64:
                addValueFromRobot(full_name, refl->GetInt64(*m, field));
                break;
            case T::CPPTYPE_UINT32:
                addValueFromRobot(full_name, refl->GetUInt32(*m, field));
                break;
            case T::CPPTYPE_UINT64:
                addValueFromRobot(full_name, refl->GetUInt64(*m, field));
                break;
            case T::CPPTYPE_FLOAT:
                addValueFromRobot(full_name, refl->GetFloat(*m, field));
                break;
            case T::CPPTYPE_DOUBLE:
                addValueFromRobot(full_name, refl->GetDouble(*m, field));
                break;
            case T::CPPTYPE_BOOL:
                addValueFromRobot(full_name, refl->GetBool(*m, field));
                break;
            case T::CPPTYPE_STRING:
                addValueFromRobot(full_name, refl->GetString(*m, field));
                break;
            case T::CPPTYPE_ENUM:
                addValueFromRobot(full_name, refl->GetEnum(*m, field)->name());
                break;
            case T::CPPTYPE_MESSAGE: {
                const Message& mfield = refl->GetMessage(*m, field);
                // Message* mcopy = mfield.New();
                // mcopy->CopyFrom(mfield);
                // void* ptr = new std::shared_ptr<Message>(mcopy);
                // std::shared_ptr<Message>* m = static_cast<std::shared_ptr<Message>*>(ptr);
                // addProtoBufValueFromRobot(*m);
                addProtoBufValueFromRobot(&mfield, full_name);
                break;
            }
            default:
                std::cerr << "[WebCFace] Protobuf Unsupported Field Type? " << full_name
                          << ", type = " << field->type() << std::endl;
                break;
            }
        }
    }
}
}  // namespace WebCFace