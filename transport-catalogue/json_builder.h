#pragma once

#include "json.h"

#include <optional>

namespace json {

class DictItemContext;
class ArrayItemContext;
class DictKeyContext;
    
class Builder {
public:
    Builder& Value(Node::Value value);

    DictItemContext StartDict();
    
    ArrayItemContext StartArray();
    
    DictKeyContext Key(std::string str);
    
    Builder& EndDict();
    
    Builder& EndArray();
    
    Node Build() const;
    
private:
    Node MakeNodeFromValue(Node::Value value) const;
    
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> key_;
    
    mutable bool object_is_complete_ = false;    
};    
    
class DictItemContext {
public:
    DictItemContext(Builder& builder)
        : builder_(builder) {            
    }
    
    DictKeyContext Key(std::string str);
    Builder& EndDict();
    
private:
    Builder& builder_;    
};

class ArrayItemContext {
public:
    ArrayItemContext(Builder& builder)
        : builder_(builder) {            
    }
    
    ArrayItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();

private:
    Builder& builder_;    
};
    
class DictKeyContext {
public:    
    DictKeyContext(Builder& builder)
        : builder_(builder) {            
    }    

    DictItemContext Value(Node::Value value);
    DictItemContext StartDict(); 
    ArrayItemContext StartArray();
    
private:
    Builder& builder_;       
}; 
       
}
