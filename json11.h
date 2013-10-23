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
        virtual Type type() const { return JSNULL; }
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
    struct Bool : public Node {
        Bool(bool x) { refcnt = 1; }
        Type type() const { return BOOL; }
        void print(std::ostream& out) const;
        static Bool T;
        static Bool F;
    };
    //
    struct Number : public Node {
        long double value;
        int prec;
        Number(long double x) { prec = LDBL_DIG; value = x; }
        Number(double x) { prec = DBL_DIG; value = x; }
        Number(float x) { prec = FLT_DIG; value = x; }
        Number(long long x) { prec = DBL_DIG; value = x; }
        Number(long x) { prec = -1; value = x; }
        Number(int x) { prec = -1; value = x; }
        Number(std::istream&);
        Type type() const { return NUMBER; }
        void print(std::ostream& out) const;
        bool operator == (const Node& that);
    };
    //
    struct String : public Node {
        std::string value;
        String(std::string s) { value = s; }
        String(std::istream&);
        Type type() const { return STRING; }
        void print(std::ostream& out) const;
        bool operator == (const Node& that);
    };
    //
    struct Object : public Node {
        std::map<const std::string*, Node*> map;
        virtual ~Object();
        Type type() const { return OBJECT; }
        void print(std::ostream&) const;
        void set(const std::string&, Node*);
        bool contains(const Node*) const;
        bool operator == (const Node& that);
    };
    //
    struct Array : public Node {
        std::vector<Node*> list;
        virtual ~Array();
        Type type() const { return ARRAY; }
        void print(std::ostream&) const;
        void add(Node*);
        void ins(unsigned, Node*);
        void del(unsigned);
        void repl(unsigned, Node*);
        bool contains(const Node*) const;
        bool operator == (const Node& that);
    };
    Array* mkarray();
    Object* mkobject();
    Json(Node* node) { (root = node)->refcnt++; }
    static std::set<std::string> keyset;   // all propery names
    static int level;   // for pretty printing
    //
    Node* root;
    //
public:
    // constructors
    Json() { (root = &Node::null)->refcnt++; }
    Json(const Json& that);
    Json(Json&& that);
    Json(std::istream&);   // parse
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
    void insert(unsigned index, const Json&);
    void erase(unsigned index);
    Json& replace(unsigned index, const Json&);
    //
    // subscript
    const Json operator [] (int) const;
    const Json operator [] (std::string& key) const { return get(key); }
    const Json operator [] (const char*) const;
    size_t size() const;
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
