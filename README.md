
# Introduction

Json-11 is yet another C++ class implementing [JSON](http://json.org) data interchange format. It is designed with the (currenly) latest C++-11 standard in mind. The code is small (a single file less than 1000 lines in size), reasonably fast, and the interface (aka API) is made as simple as possible.

# Usage

    #include <json11.h>

## The basics

    Json jnull; // creates a null Json object.
    Json jtrue = true, pi = 3.14, str = "Hello";  // scalar objects
    Json arr { 1, 2, str };   // array object, using initializer list
    arr << false;  // do you prefer 'push_back' ?

To create key-value objects, start with a null object. Then use method `set(key,value)` to populate it.

    Json obj;
    obj.set("one", 1).set("pi", pi).set("arr", arr);
    cout << obj << endl;   // prints {"one":1,"pi":3.14,"arr":[1,2,"Hello",false]}

To extract scalar value from a Json instance, assign it to a scalar variable of matching type or use a cast. For collections use `operator[]` with either integer or string indexes.

    string text = str;     // text is Hello (without quotes)
    double num_pi = pi;
    int one = obj["one"];
    string hello = obj["arr"][2];

To 'stringify' a Json instance, write it to a std::ostream. To get it as a std::string use method `stringify()` or its alias `format()`:

    string jsontext = obj.format();

To parse a JSON text, read it from a std::istream. To parse it from std::string use static  method `parse()`:

    Json obj2 = Json::parse(jsontext);
    cout << obj == obj2 << endl;   // prints true; yes, Json_s_ are comparable
    ifstream fs("big.json");
    Json big(fs);

To determine the type and size of a Json instance use methods `type()` and `size()`. The first returns one of `enum Json::Type` values:

    enum Json::Type { JSNULL, BOOL, NUMBER, STRING, ARRAY, OBJECT };

The second returns either number of array elements or number of properties of an object. Property names are returned by `keys()` as `vector<string>`. These methods, together with `operator[]` allow to fully traverse an object.

This example also shows that Json instances can be compared. Scalars are equal if and only if they have same types and same values. Arrays are equal if their elements and order of them are the same. Objects are equal if their properties and corresponding values are equal, regardless of order.

## Exceptions

The Json class defines two exception classes, `parse_error` and `use_error`. The first one can be thrown only from `Json(istream&)`. The second is thrown when the class methods are used inappropriately, for example when `operator<<` is used for an object. It is also thrown when adding to a container would lead to circular dependency, like in this example:

    Json js1, js2;
    js1 << 1 << 2;
    js2 << js1;
    js1 << js2;   // error

The Json class methods can also throw standard exceptions out_of_range (from operator[]) and bad_cast.

## Constructors

| Json() | initializes to Json::null |
| Json(const Json&) | copy contructor |
| Json(Json&&) | move contructor |


## Miscellaneous notes

Json class defines a static member Json::null of type JSNULL. It can be used to test for 'nullity' and for removing all contents from an object:

    if (js == Json::null)
        big_object = Json::null;

Reading a file creates a representation of entire file in memory. When it is not desirable, use static method `traverse`:

    Json::traverse(fs, on_entry, on_leave, on_value);

which calls supplied procedures while traversing through the file.

The is a difference between s1 and s2 in the following example:

    Json hello = "hello";
    string s1 = hello;
    string s2 = hello.format();

The assignment to s1 is a cast, so the value of s1 is _hello_. The value of s2 is _"hello"_ (with double quotes) because `to_string` creates a well-formed JSON text and returns it as a string.

------
