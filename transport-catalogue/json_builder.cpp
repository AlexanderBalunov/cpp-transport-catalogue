#include "json_builder.h"

namespace json {
    
Builder& Builder::Value(Node::Value value) {
    if (object_is_complete_) {
        throw std::logic_error("Calling Value(Node::Value) with the finished object");
    }
    if (!key_.has_value() && nodes_stack_.size() && !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Calling Value(Node::Value) in wrong place");
    }
    if (nodes_stack_.empty()) {
        root_.GetValue() = std::move(value);
        object_is_complete_ = true;
    } else {
        Node& last_complex_node = *nodes_stack_.back();
        if (last_complex_node.IsArray()) {
            last_complex_node.AsArray().emplace_back(MakeNodeFromValue(value));
        }
        if (last_complex_node.IsDict()) {
            last_complex_node.AsDict().emplace(key_.value(), MakeNodeFromValue(value));
            key_ = std::nullopt;
        } 
    }
    return *this;
}

DictItemContext Builder::StartDict() {
    if (object_is_complete_) {
        throw std::logic_error("Calling StartDict() with the finished object");
    }    
    if (!key_.has_value() && nodes_stack_.size() && !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Calling StartDict() in wrong place");
    }
    if (nodes_stack_.empty()) {
        root_.GetValue() = Dict();
        nodes_stack_.push_back(&root_);
    } else {    
        Node& last_complex_node = *nodes_stack_.back();
        if (last_complex_node.IsArray()) {
            Array& last_node_array = last_complex_node.AsArray();
            last_node_array.emplace_back(MakeNodeFromValue(Dict()));
            nodes_stack_.push_back(&last_node_array.back());
        }
        if (last_complex_node.IsDict()) {
            Dict& last_node_dict = last_complex_node.AsDict();
            last_node_dict.emplace(key_.value(), MakeNodeFromValue(Dict()));
            nodes_stack_.push_back(&last_node_dict.at(key_.value()));
            key_ = std::nullopt;
        }        
    }
    return *this;
}

ArrayItemContext Builder::StartArray() {
    if (object_is_complete_) {
        throw std::logic_error("Calling StartArray() with the finished object");
    }        
    if (!key_.has_value() && nodes_stack_.size() && !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Calling StartArray() in wrong place");
    }
    if (nodes_stack_.empty()) {
        root_.GetValue() = Array();
        nodes_stack_.push_back(&root_);
    } else {    
        Node& last_complex_node = *nodes_stack_.back();
        if (last_complex_node.IsArray()) {
            Array& last_node_array = last_complex_node.AsArray();
            last_node_array.emplace_back(MakeNodeFromValue(Array()));
            nodes_stack_.push_back(&last_node_array.back());
        }
        if (last_complex_node.IsDict()) {
            Dict& last_node_dict = last_complex_node.AsDict();
            last_node_dict.emplace(key_.value(), MakeNodeFromValue(Array()));
            nodes_stack_.push_back(&last_node_dict.at(key_.value()));
            key_ = std::nullopt;
        }             
    }       
    return *this;
}

DictKeyContext Builder::Key(std::string str) {
    if (object_is_complete_) {
        throw std::logic_error("Calling Key(std::string) with the finished object");
    }         
    if (!nodes_stack_.back()->IsDict() || key_) {
        throw std::logic_error("Calling Key(std::string) from outside the Dict or after another Key(std::string)");
    }
    key_ = str;
    return *this;
}
    
Builder& Builder::EndDict() {
    if (object_is_complete_) {
        throw std::logic_error("Calling EndDict() with the finished object");
    }     
    if (!nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Calling EndDict() in the context of another container");
    }
    nodes_stack_.pop_back();
    if (!nodes_stack_.size()) {
        object_is_complete_ = true;
    }
    return *this;
}
    
Builder& Builder::EndArray() {
    if (object_is_complete_) {
        throw std::logic_error("Calling EndArray() with the finished object");
    }         
    if (!nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Calling EndArray() in the context of another container");
    }    
    nodes_stack_.pop_back();
    if (!nodes_stack_.size()) {
        object_is_complete_ = true;
    }    
    return *this;
}
    
Node Builder::Build() const {
    if (!object_is_complete_) {
        throw std::logic_error("Calling Build() when the described object is not ready");
    }
    return root_;
}
    
Node Builder::MakeNodeFromValue(Node::Value value) const {
    if (std::holds_alternative<int>(value)) {
        return std::get<int>(value);
    }
    if (std::holds_alternative<double>(value)) {
        return std::get<double>(value);
    }
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value);
    }
    if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    }
    if (std::holds_alternative<Array>(value)) {
        return std::get<Array>(value);
    }
    if (std::holds_alternative<Dict>(value)) {
        return std::get<Dict>(value);
    }
    return {};
}    
    
DictKeyContext DictItemContext::Key(std::string str) {
    return builder_.Key(str);
}
        
Builder& DictItemContext::EndDict() {
    return builder_.EndDict();
}
    
ArrayItemContext ArrayItemContext::Value(Node::Value value) {
    return builder_.Value(value);
}
    
DictItemContext ArrayItemContext::StartDict() {
    return builder_.StartDict();
}
    
ArrayItemContext ArrayItemContext::StartArray() {
    return builder_.StartArray();
}
    
Builder& ArrayItemContext::EndArray() {
    return builder_.EndArray();
}

DictItemContext DictKeyContext::Value(Node::Value value) {
    return builder_.Value(value);
}
    
DictItemContext DictKeyContext::StartDict() {
    return builder_.StartDict();
}
    
ArrayItemContext DictKeyContext::StartArray() {
    return builder_.StartArray();
}
    
}
