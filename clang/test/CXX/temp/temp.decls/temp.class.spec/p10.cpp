// RUN: %clang_cc1 -fsyntax-only -verify %s

// C++20 [temp.class.spec] 17.6.5/10:
//   The usual access checking rules do not apply to non-dependent names used
//   to specify template arguments of the simple-template-id of the partial
//   specialization.

class TestClass {
  // expected-note@+1 4 {{declared private here}}
  void func();

  // expected-note@+1 4 {{declared private here}}
  void funcOverloaded();

  void funcOverloaded(int);

  // expected-note@+1 2 {{declared private here}}
  static void staticFunc();

  // expected-note@+1 2 {{declared private here}}
  static void staticFuncOverloaded();

  static void staticFuncOverloaded(int);

  // expected-note@+1 {{declared private here}}
  class Nested {};

  // expected-note@+1 {{declared private here}}
  int field;
};

template <void (TestClass::*)()> class TemplateClass {};
template <> class TemplateClass<&TestClass::func> {};
template <> class TemplateClass<&TestClass::funcOverloaded> {};

// expected-error@+1 {{'func' is a private member of 'TestClass'}}
using alias1_1 = TemplateClass<&TestClass::func>;

// expected-error@+1 {{'funcOverloaded' is a private member of 'TestClass'}}
using alias1_2 = TemplateClass<&TestClass::funcOverloaded>;

template <void (*)()> class TemplateClass2 { };
template <> class TemplateClass2<&TestClass::staticFunc> {};
template <> class TemplateClass2<&TestClass::staticFuncOverloaded> {};

// expected-error@+1 {{'staticFunc' is a private member of 'TestClass'}}
using alias2_1 = TemplateClass2<&TestClass::staticFunc>;

// expected-error@+1 {{'staticFuncOverloaded' is a private member of 'TestClass'}}
using alias2_2 = TemplateClass2<&TestClass::staticFuncOverloaded>;

template<typename T, void (TestClass::*)()> class TemplateClass3 {};
template<typename T> class TemplateClass3<T, &TestClass::func> {};
template<typename T> class TemplateClass3<T, &TestClass::funcOverloaded> {};

// expected-error@+2 {{'func' is a private member of 'TestClass'}}
template <typename T>
using alias3_1 = TemplateClass3<T, &TestClass::func>;

// expected-error@+1 {{'func' is a private member of 'TestClass'}}
using alias3_2 = TemplateClass3<int, &TestClass::func>;

// expected-error@+2 {{'funcOverloaded' is a private member of 'TestClass'}}
template <typename T>
using alias3_3 = TemplateClass3<T, &TestClass::funcOverloaded>;

// expected-error@+1 {{'funcOverloaded' is a private member of 'TestClass'}}
using alias3_4 = TemplateClass3<int, &TestClass::funcOverloaded>;

// expected-error@+2 {{'func' is a private member of 'TestClass'}}
template <typename T>
class TemplateClass3<T, &TestClass::func> varTemplate3_1 {};

// expected-error@+2 {{'funcOverloaded' is a private member of 'TestClass'}}
template <typename T>
class TemplateClass3<T, &TestClass::funcOverloaded> varTemplate3_2 {};

template<typename T, void (*)()> class TemplateClass4 {};
template<typename T> class TemplateClass4<T, &TestClass::staticFunc> {};
template<typename T> class TemplateClass4<T, &TestClass::staticFuncOverloaded> {};

// expected-error@+2 {{'staticFunc' is a private member of 'TestClass'}}
template <typename T>
class TemplateClass4<T, &TestClass::staticFunc> varTemplate4_1 {};

// expected-error@+2 {{'staticFuncOverloaded' is a private member of 'TestClass'}}
template <typename T>
class TemplateClass4<T, &TestClass::staticFuncOverloaded> varTemplate4_2 {};

template<typename T> class TemplateClass5 {};
template<> class TemplateClass5<TestClass::Nested> {};

template<typename T, typename U> class TemplateClass6 {};
template<typename T> class TemplateClass6<T, TestClass::Nested> {};

// expected-error@+2 {{'Nested' is a private member of 'TestClass'}}
template <typename T>
class TemplateClass6<T, TestClass::Nested> varTemplate6_1 {};

template <int TestClass::*> class TemplateClass7 {};
template <> class TemplateClass7<&TestClass::field> {};

template <typename T, int TestClass::*> class TemplateClass8 {};
template <typename T> class TemplateClass8<T, &TestClass::field> {};

// expected-error@+2 {{'field' is a private member of 'TestClass'}}
template <typename T>
class TemplateClass8<T, &TestClass::field> varTemplate8_1 {};

template<class T>
struct trait;

class class_ {
  template<class U>
  struct impl;
};

template<class U>
struct trait<class_::impl<U>>;
