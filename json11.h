/*
 * This file is part of json11 project (https://github.com/borisgontar/json11).
 *
 * Copyright (c) 2013 Boris Gontar.
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JSON11_H_
#define JSON11_H_

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <cfloat>
#include <stdexcept>
#include <initializer_list>

class Json {
public:
    enum Type {
        JSNULL, BOOL, NUMBER, STRING, ARRAY, OBJECT
    };
private:
    struct Node {
    	unsigned refcnt;
        Node(unsigned init = 0);
        virtual ~Node();
        virtual Type type() const { return Type::JSNULL; }
        virtual void print(std::ostream& out) const { out << "null"; }
        virtual bool contains(const Node* that) const { return false; }
        virtual bool operator == (const Node& that) { return this == &that; }
        void unref();
#ifdef TEST
        static std::vector<Node*> nodes;
        static void test();
#endif
        static Node null;
    };
    //
    struct Bool : Node {
        Bool(bool x) { refcnt = 1; }
        Type type() const override { return Type::BOOL; }
        void print(std::ostream& out) const override;
        static Bool T;
        static Bool F;
    };
    //
    struct Number : Node {
        long double value;
        int prec;
        Number(long double x) { prec = LDBL_DIG; value = x; }
        Number(double x) { prec = DBL_DIG; value = x; }
        Number(float x) { prec = FLT_DIG; value = x; }
        Number(long long x) { prec = DBL_DIG; value = x; }
        Number(long x) { prec = -1; value = x; }
        Number(int x) { prec = -1; value = x; }
        Number(std::istream&);
        Type type() const override { return Type::NUMBER; }
        void print(std::ostream& out) const override;
        bool operator == (const Node& that) override;
    };
    //
    struct String : Node {
        std::string value;
        String(std::string s) { value = s; }
        String(std::istream&);
        Type type() const override { return Type::STRING; }
        void print(std::ostream& out) const override;
        bool operator == (const Node& that) override;
    };
    //
    struct Object : Node {
        std::map<const std::string*, Node*> map;
        virtual ~Object();
        Type type() const  override{ return Type::OBJECT; }
        void print(std::ostream&) const override;
        Node* get(const std::string&);
        void set(const std::string&, Node*);
        bool contains(const Node*) const override;
        bool operator == (const Node& that) override;
    };
    //
    struct Array : Node {
        std::vector<Node*> list;
        virtual ~Array();
        Type type() const override { return Type::ARRAY; }
        void print(std::ostream&) const override;
        void add(Node*);
        void ins(int, Node*);
        void del(int);
        void repl(int, Node*);
        bool contains(const Node*) const override;
        bool operator == (const Node& that) override;
    };
    //
    class Property {
        Node* host;
        std::string key;
        int index;
    public:
        Property(Node*, const std::string&);
        Property(Node*, int);
        Json target() const;
        operator Json() const { return target(); }
        operator bool() { return target(); };
        operator int() { return target(); };
        operator long() { return target(); };
        operator long long() { return target(); };
        operator float() { return target(); };
        operator double() { return target(); };
        operator long double() { return target(); };
        operator std::string() { return target(); };
        Property operator [] (const std::string& k) { return target()[k]; }
        Property operator [] (const char* k) { return (*this)[std::string(k)]; }
        Property operator [] (int i) {return target()[i]; }
        Json operator = (const Json&);
        Json operator = (const Property&);
        friend std::ostream& operator << (std::ostream& out, const Property& p) {
            return out << (Json)p;
        }
    };
    Array* mkarray();
    Object* mkobject();
    static std::set<std::string> keyset;   // all propery names
    static int level;   // for pretty printing
    //
    Json(Node* node) {
        (root = (node == nullptr ? &Node::null : node))->refcnt++;
    }
    Node* root;
    //
public:
    // constructors
    Json() { (root = &Node::null)->refcnt++; }
    Json(const Json& that);
    Json(Json&& that);
    Json(std::istream&, bool full = true);   // parse
    virtual ~Json();
    //
    // initializers
    Json& operator = (const Json&);
    Json& operator = (Json&&);
    //
    // more constructors
    Json(bool x) { (root = (x ? &Bool::T : &Bool::F))->refcnt++; }
    Json(int x) { (root = new Number(x))->refcnt++; }
    Json(long x) { (root = new Number(x))->refcnt++; }
    Json(long long x) { (root = new Number(x))->refcnt++; }
    Json(float x) { (root = new Number(x))->refcnt++; }
    Json(double x) { (root = new Number(x))->refcnt++; }
    Json(long double x) { (root = new Number(x))->refcnt++; }
    Json(std::string& s) { (root = new String(s))->refcnt++; }
    Json(const char* s) { (root = new String(s))->refcnt++; }
    Json(std::initializer_list<Json>);
    //
    // casts
    Type type() const { return root->type(); }
    operator bool() const;
    operator int() const;
    operator long() const;
    operator long long() const;
    operator float() const;
    operator double() const;
    operator long double() const;
    operator std::string() const;
    //
    // object
    Json& set(std::string key, const Json& val);
    Json get(const std::string& key) const;
    bool has(const std::string& key) const;
    std::vector<std::string> keys();
    //
    // array
    Json& operator << (const Json&);
    void insert(int index, const Json&);
    void erase(int index);
    Json& replace(int index, const Json&);
    //
    // subscript
    size_t size() const;
    Json::Property operator [] (const std::string&);
    Json::Property operator [] (const char* k) { return (*this)[std::string(k)]; }
    Json::Property operator [] (int);
    //
    // stringify
    std::string stringify() { return format(); }
    std::string format();
    friend std::ostream& operator << (std::ostream&, const Json&);
    //
    // compare
    bool operator == (const Json&);
    bool operator != (const Json& that) { return !(*this == that); }
    //
    static Json null;
    static Json parse(const std::string&);
    static int indent;  // for pretty printing
    //
    struct parse_error : std::runtime_error {
        unsigned line = 0, col = 0;
        parse_error(const char* msg, std::istream& in);
    };
    struct use_error : std::logic_error {
        use_error(const char* msg) : std::logic_error(msg) {}
    };
#ifdef TEST
    static void test() { Node::test(); }
#endif
};

#endif /* JSON11_H_ */
