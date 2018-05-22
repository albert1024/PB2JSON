// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.
// http://code.google.com/p/protobuf/
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Author: cyrusn@google.com (Cyrus Najmabadi)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include <vector>

#include <google/protobuf/stubs/hash.h>
#include <google/protobuf/compiler/objectivec/objectivec_helpers.h>
#include <google/protobuf/objectivec-descriptor.pb.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/stubs/strutil.h>

namespace google { namespace protobuf { namespace compiler { namespace objectivec {
    namespace {
        const string& FieldName(const FieldDescriptor* field) {
            // Groups are hacky:  The name of the field is just the lower-cased name
            // of the group type.  In ObjectiveC, though, we would like to retain the original
            // capitalization of the type name.
            if (field->type() == FieldDescriptor::TYPE_GROUP) {
                return field->message_type()->name();
            } else {
                return field->name();
            }
        }
    }
    
    
    bool AreFieldsDifferent(const map<string, string>& variables, const string& name1, const string& name2) {
        string field1;
        string field2;
        
        map<string, string>::const_iterator it = variables.find(name1);
        if (it != variables.end()) {
            field1 = it->second;
        }
        
        it = variables.find(name2);
        if (it != variables.end()) {
            field2 = it->second;
        }
        
        return !(field1 == field2);
    }
    
    
    bool HasOwnedObjectPrefixInField(const map<string, string>& variables, const string& field) {
        map<string, string>::const_iterator it = variables.find(field);
        if (it == variables.end()) {
            return false;
        }
        
        return HasOwnedObjectPrefix(it->second);
    }
    
    
    bool IsObjectiveCKeyWord(const string& word) {
        static string keywords[] = {"id"};
        for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); ++i) {
            if (word == keywords[i]) {
                return true;
            }
        }
        return false;
    }
    
    
    bool IsComplieKeyWord(const string& word) {
        static string keywords[] = {"value", "iValue", "oValue", "values", "data", "input", "output"};
        for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); ++i) {
            if (word == keywords[i]) {
                return true;
            }
        }
        return false;
    }
    
    
    bool HasOwnedObjectPrefix(const string& word) {
        static string prefixes[] = {"alloc", "new", "copy", "mutableCopy"};
        for (size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
            const string& prefix = prefixes[i];
            if (word.length() > prefix.length()
                && 0 == word.compare(0, prefix.length(), prefix)) {
                return true;
            }
        }
        return false;
    }
    
    string UnderscoresToCapitalizedCamelCase(const string& input) {
        vector<string> values;
        string current;
        
        bool last_char_was_number = false;
        bool last_char_was_lower = false;
        bool last_char_was_upper = false;
        for (int i = 0; i < input.size(); i++) {
            char c = input[i];
            if (c >= '0' && c <= '9') {
                if (!last_char_was_number) {
                    values.push_back(current);
                    current = "";
                }
                current += c;
                last_char_was_number = last_char_was_lower = last_char_was_upper = false;
                last_char_was_number = true;
            } else if (c >= 'a' && c <= 'z') {
                // lowercase letter can follow a lowercase or uppercase letter
                if (!last_char_was_lower && !last_char_was_upper) {
                    values.push_back(current);
                    current = "";
                }
                current += c;
                last_char_was_number = last_char_was_lower = last_char_was_upper = false;
                last_char_was_lower = true;
            } else if (c >= 'A' && c <= 'Z') {
                if (!last_char_was_upper) {
                    values.push_back(current);
                    current = "";
                }
                current += c;
                last_char_was_number = last_char_was_lower = last_char_was_upper = false;
                last_char_was_upper = true;
            } else {
                last_char_was_number = last_char_was_lower = last_char_was_upper = false;
            }
        }
        values.push_back(current);
        
        for (vector<string>::iterator i = values.begin(); i != values.end(); ++i) {
            string value = *i;
            for (int j = 0; j < value.length(); j++) {
                if (j == 0) {
                    value[j] = toupper(value[j]);
                } else {
                    value[j] = tolower(value[j]);
                }
            }
            *i = value;
        }
        string result;
        for (vector<string>::iterator i = values.begin(); i != values.end(); ++i) {
            result += *i;
        }
        return result;
    }
    
    
    string UnderscoresToCamelCase(const string& input) {
        string result = UnderscoresToCapitalizedCamelCase(input);
        if (result.length() == 0) {
            return result;
        }
        
        result[0] = tolower(result[0]);
        return result;
    }
    
    
    string UnderscoresToCamelCase(const FieldDescriptor* field) {
        return UnderscoresToCamelCase(FieldName(field));
    }
    
    
    string UnderscoresToCapitalizedCamelCase(const FieldDescriptor* field) {
        return UnderscoresToCapitalizedCamelCase(FieldName(field));
    }
    
    
    string UnderscoresToCamelCase(const MethodDescriptor* method) {
        return UnderscoresToCamelCase(method->name());
    }
    
    
    string StripProto(const string& filename) {
        if (HasSuffixString(filename, ".protodevel")) {
            return StripSuffixString(filename, ".protodevel");
        } else {
            return StripSuffixString(filename, ".proto");
        }
    }
    
    
    bool IsBootstrapFile(const FileDescriptor* file) {
        return file->name() == "google/protobuf/descriptor.proto";
    }
    
    
    string FileName(const FileDescriptor* file) {
        string basename;
        
        string::size_type last_slash = file->name().find_last_of('/');
        if (last_slash == string::npos) {
            basename += file->name();
        } else {
            basename += file->name().substr(last_slash + 1);
        }
        
        //return UnderscoresToCapitalizedCamelCase(StripProto(basename));
        return StripProto(basename);
    }
    
    
    string FilePath(const FileDescriptor* file) {
        string path = FileName(file);
        
        if (file->options().HasExtension(objectivec_file_options)) {
            ObjectiveCFileOptions options = file->options().GetExtension(objectivec_file_options);
            
            if (options.package() != "") {
                path = options.package() + "/" + path;
            }
        }
        
        return path;
    }
    
    
    string FileClassPrefix(const FileDescriptor* file) {
        if (IsBootstrapFile(file)) {
            return "PB";
        } else if (file->options().HasExtension(objectivec_file_options)) {
            ObjectiveCFileOptions options = file->options().GetExtension(objectivec_file_options);
            
            return options.class_prefix();
        } else {
            return "";
        }
    }
    
    
    string FileClassName(const FileDescriptor* file) {
        return FileClassPrefix(file) + FileName(file) + "Root";
    }
    
    
    string ToObjectiveCName(const string& full_name, const FileDescriptor* file) {
        string result;
        result += FileClassPrefix(file);
        result += full_name;
        return result;
    }
    
    
    string ClassNameWorker(const Descriptor* descriptor) {
        string name;
        if (descriptor->containing_type() != NULL) {
            name = ClassNameWorker(descriptor->containing_type());
            name += "_";
        }
        return name + descriptor->name();
    }
    
    
    string ClassNameWorker(const EnumDescriptor* descriptor) {
        string name;
        if (descriptor->containing_type() != NULL) {
            name = ClassNameWorker(descriptor->containing_type());
            name += "_";
        }
        return name + descriptor->name();
    }
    
    
    string ClassName(const Descriptor* descriptor) {
        string name;
        name += FileClassPrefix(descriptor->file());
        name += ClassNameWorker(descriptor);
        return name;
    }
    
    
    string ClassName(const EnumDescriptor* descriptor) {
        string name;
        name += FileClassPrefix(descriptor->file());
        name += ClassNameWorker(descriptor);
        return name;
    }
    
    
    string ClassName(const ServiceDescriptor* descriptor) {
        string name;
        name += FileClassPrefix(descriptor->file());
        name += descriptor->name();
        return name;
    }
    
    
    string EnumValueName(const EnumValueDescriptor* descriptor) {
        return
        ClassName(descriptor->type()) +
        UnderscoresToCapitalizedCamelCase(SafeName(descriptor->name()));
    }
    
    
    ObjectiveCType GetObjectiveCType(FieldDescriptor::Type field_type) {
        switch (field_type) {
            case FieldDescriptor::TYPE_INT32:
            case FieldDescriptor::TYPE_SINT32:
            case FieldDescriptor::TYPE_FIXED32:
            case FieldDescriptor::TYPE_SFIXED32:
                return OBJECTIVECTYPE_INT;
                
            case FieldDescriptor::TYPE_UINT32:
                return OBJECTIVECTYPE_UINT;
                
            case FieldDescriptor::TYPE_INT64:
            case FieldDescriptor::TYPE_SINT64:
            case FieldDescriptor::TYPE_FIXED64:
            case FieldDescriptor::TYPE_SFIXED64:
                return OBJECTIVECTYPE_LONG;
                
            case FieldDescriptor::TYPE_UINT64:
                return OBJECTIVECTYPE_ULONG;
                
            case FieldDescriptor::TYPE_FLOAT:
                return OBJECTIVECTYPE_FLOAT;
                
            case FieldDescriptor::TYPE_DOUBLE:
                return OBJECTIVECTYPE_DOUBLE;
                
            case FieldDescriptor::TYPE_BOOL:
                return OBJECTIVECTYPE_BOOLEAN;
                
            case FieldDescriptor::TYPE_STRING:
                return OBJECTIVECTYPE_STRING;
                
            case FieldDescriptor::TYPE_BYTES:
                return OBJECTIVECTYPE_DATA;
                
            case FieldDescriptor::TYPE_ENUM:
                return OBJECTIVECTYPE_ENUM;
                
            case FieldDescriptor::TYPE_GROUP:
            case FieldDescriptor::TYPE_MESSAGE:
                return OBJECTIVECTYPE_MESSAGE;
        }
        
        GOOGLE_LOG(FATAL) << "Can't get here.";
        return OBJECTIVECTYPE_INT;
    }
    
    
    const char* BoxedPrimitiveTypeName(ObjectiveCType type) {
        switch (type) {
            case OBJECTIVECTYPE_INT    : return "NSNumber";
            case OBJECTIVECTYPE_UINT   : return "NSNumber";
            case OBJECTIVECTYPE_LONG   : return "NSNumber";
            case OBJECTIVECTYPE_ULONG  : return "NSNumber";
            case OBJECTIVECTYPE_FLOAT  : return "NSNumber";
            case OBJECTIVECTYPE_DOUBLE : return "NSNumber";
            case OBJECTIVECTYPE_BOOLEAN: return "NSNumber";
            case OBJECTIVECTYPE_STRING : return "NSString";
            case OBJECTIVECTYPE_DATA   : return "NSData";
            case OBJECTIVECTYPE_ENUM   : return "NSNumber";
            case OBJECTIVECTYPE_MESSAGE: return NULL;
        }
        
        GOOGLE_LOG(FATAL) << "Can't get here.";
        return NULL;
    }
    
    
    bool IsPrimitiveType(ObjectiveCType type) {
        switch (type) {
            case OBJECTIVECTYPE_INT    :
            case OBJECTIVECTYPE_UINT   :
            case OBJECTIVECTYPE_LONG   :
            case OBJECTIVECTYPE_ULONG  :
            case OBJECTIVECTYPE_FLOAT  :
            case OBJECTIVECTYPE_DOUBLE :
            case OBJECTIVECTYPE_BOOLEAN:
            case OBJECTIVECTYPE_ENUM   :
                return true;
        }
        
        return false;
    }
    
    
    bool IsReferenceType(ObjectiveCType type) {
        return !IsPrimitiveType(type);
    }
    
    
    bool ReturnsPrimitiveType(const FieldDescriptor* field) {
        return IsPrimitiveType(GetObjectiveCType(field->type()));
    }
    
    
    bool ReturnsReferenceType(const FieldDescriptor* field) {
        return !ReturnsPrimitiveType(field);
    }
    
    
    namespace {
        string DotsToUnderscores(const string& name) {
            return StringReplace(name, ".", "_", true);
        }
        
        const char* const kKeywordList[] = {
            "TYPE_BOOL"
        };
        
        
        hash_set<string> MakeKeywordsMap() {
            hash_set<string> result;
            for (int i = 0; i < GOOGLE_ARRAYSIZE(kKeywordList); i++) {
                result.insert(kKeywordList[i]);
            }
            return result;
        }
        
        hash_set<string> kKeywords = MakeKeywordsMap();
    }
    
    
    string SafeName(const string& name) {
        string result = name;
        if (kKeywords.count(result) > 0) {
            result.append("_");
        }
        return result;
    }
    
    string BoxValue(const FieldDescriptor* field, const string& value) {
        switch (GetObjectiveCType(field)) {
            case OBJECTIVECTYPE_INT:
                return "[NSNumber numberWithInt:" + value + "]";
            case OBJECTIVECTYPE_UINT:
                return "[NSNumber numberWithUnsignedInt:" + value + "]";
            case OBJECTIVECTYPE_LONG:
                return "[NSNumber numberWithLongLong:" + value + "]";
            case OBJECTIVECTYPE_ULONG:
                return "[NSNumber numberWithUnsignedLongLong:" + value + "]";
            case OBJECTIVECTYPE_FLOAT:
                return "[NSNumber numberWithFloat:" + value + "]";
            case OBJECTIVECTYPE_DOUBLE:
                return "[NSNumber numberWithDouble:" + value + "]";
            case OBJECTIVECTYPE_BOOLEAN:
                return "[NSNumber numberWithBool:" + value + "]";
            case OBJECTIVECTYPE_ENUM:
                return "[NSNumber numberWithInt:" + value + "]";
        }
        
        return value;
    }
    
    bool AllAscii(const string& text) {
        for (int i = 0; i < text.size(); i++) {
            if ((text[i] & 0x80) != 0) {
                return false;
            }
        }
        return true;
    }
    
    string DefaultValue(const FieldDescriptor* field) {
        // Switch on cpp_type since we need to know which default_value_* method
        // of FieldDescriptor to call.
        switch (field->cpp_type()) {
            case FieldDescriptor::CPPTYPE_INT32:  return SimpleItoa(field->default_value_int32());
            case FieldDescriptor::CPPTYPE_UINT32: return SimpleItoa(static_cast<int32>(field->default_value_uint32()));
            case FieldDescriptor::CPPTYPE_INT64:  return SimpleItoa(field->default_value_int64()) + "L";
            case FieldDescriptor::CPPTYPE_UINT64: return SimpleItoa(static_cast<int64>(field->default_value_uint64())) + "L";
            case FieldDescriptor::CPPTYPE_DOUBLE: return SimpleDtoa(field->default_value_double());
            case FieldDescriptor::CPPTYPE_FLOAT:  return SimpleFtoa(field->default_value_float());
            case FieldDescriptor::CPPTYPE_BOOL:   return field->default_value_bool() ? "YES" : "NO";
            case FieldDescriptor::CPPTYPE_STRING:
                if (field->type() == FieldDescriptor::TYPE_BYTES) {
                    if (field->has_default_value()) {
                        return
                        "[NSData dataWithBytes:\"" +
                        CEscape(field->default_value_string()) +
                        "\" length:" + SimpleItoa(field->default_value_string().length()) +
                        "]";
                    } else {
                        return "[NSData data]";
                    }
                } else {
                    if (AllAscii(field->default_value_string())) {
                        return "@\"" + CEscape(field->default_value_string()) + "\"";
                    } else {
                        return 
                        "[NSString stringWithUTF8String:\"" +
                        CEscape(field->default_value_string()) +
                        "\"]";
                    }
                }
            case FieldDescriptor::CPPTYPE_ENUM:
                return EnumValueName(field->default_value_enum());
            case FieldDescriptor::CPPTYPE_MESSAGE:
                //return "[" + ClassName(field->message_type()) + " defaultInstance]";
                return "nil";
        }
        
        GOOGLE_LOG(FATAL) << "Can't get here.";
        return "";
    }
    
    string FieldLabelName(const FieldDescriptor* field) {
        switch (field->label()) {
            case FieldDescriptor::LABEL_OPTIONAL:
                return "FieldLabelOptional";
            case FieldDescriptor::LABEL_REQUIRED:
                return "FieldLabelRequired";
            case FieldDescriptor::LABEL_REPEATED:
                return "FieldLabelRepeated";
            default:
                return "error";
        }
    }
    
    string FieldTypeName(const FieldDescriptor* field) {
        switch (field->type()) {
            case FieldDescriptor::TYPE_DOUBLE:
                return "FieldTypeDouble";
            case FieldDescriptor::TYPE_FLOAT:
                return "FieldTypeFloat";
            case FieldDescriptor::TYPE_INT64:
                return "FieldTypeInt64";
            case FieldDescriptor::TYPE_UINT64:
                return "FieldTypeUint64";
            case FieldDescriptor::TYPE_INT32:
                return "FieldTypeInt32";
            case FieldDescriptor::TYPE_FIXED64:
                return "FieldTypeFixed64";
            case FieldDescriptor::TYPE_FIXED32:
                return "FieldTypeFixed32";
            case FieldDescriptor::TYPE_BOOL:
                return "FieldTypeBool";
            case FieldDescriptor::TYPE_STRING:
                return "FieldTypeString";
            case FieldDescriptor::TYPE_GROUP:
                return "FieldTypeGroup";
            case FieldDescriptor::TYPE_MESSAGE:
                return "FieldTypeMessage";
            case FieldDescriptor::TYPE_BYTES:
                return "FieldTypeBytes";
            case FieldDescriptor::TYPE_UINT32:
                return "FieldTypeUint32";
            case FieldDescriptor::TYPE_ENUM:
                return "FieldTypeEnum";
            case FieldDescriptor::TYPE_SFIXED32:
                return "FieldTypeSfixed32";
            case FieldDescriptor::TYPE_SFIXED64:
                return "FieldTypeSfixed64";
            case FieldDescriptor::TYPE_SINT32:
                return "FieldTypeSint32";
            case FieldDescriptor::TYPE_SINT64:
                return "FieldTypeSint64";
            default:
                return "error";
        }
    }
}  // namespace objectivec
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
