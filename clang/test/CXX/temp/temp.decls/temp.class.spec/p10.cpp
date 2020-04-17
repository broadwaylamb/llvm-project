// RUN: %clang_cc1 -fsyntax-only -verify %s
// expected-no-diagnostics

// C++20 [temp.class.spec] 17.6.5/10:
//   The usual access checking rules do not apply to non-dependent names used
//   to specify template arguments of the simple-template-id of the partial
//   specialization.

class TestClass {
private:
  void func();
  void funcOverloaded();
  void funcOverloaded(int);

  static void staticFunc();
  static void staticFuncOverloaded();
  static void staticFuncOverloaded(int);

  class Nested {};

  int field;
};

template <void (TestClass::*)()> class TemplateClass {};
template <> class TemplateClass<&TestClass::func> {};
template <> class TemplateClass<&TestClass::funcOverloaded> {};

template <void (*)()> class TemplateClass2 { };
template <> class TemplateClass2<&TestClass::staticFunc> {};
template <> class TemplateClass2<&TestClass::staticFuncOverloaded> {};

template<typename T, void (TestClass::*)()> class TemplateClass3 {};
template<typename T> class TemplateClass3<T, &TestClass::func> {};
template<typename T> class TemplateClass3<T, &TestClass::funcOverloaded> {};

template<typename T, void (*)()> class TemplateClass4 {};
template<typename T> class TemplateClass4<T, &TestClass::staticFunc> {};
template<typename T> class TemplateClass4<T, &TestClass::staticFuncOverloaded> {};

template<typename T> class TemplateClass5 {};
template<> class TemplateClass5<TestClass::Nested> {};

template<typename T, typename U> class TemplateClass6 {};
template<typename T> class TemplateClass6<T, TestClass::Nested> {};

template <int TestClass::*> class TemplateClass7 {};
template <> class TemplateClass7<&TestClass::field> {};

template <typename T, int TestClass::*> class TemplateClass8 {};
template <typename T> class TemplateClass8<T, &TestClass::field> {};

template<class T>
struct trait;

class class_ {
  template<class U>
  struct impl;
};

template<class U>
struct trait<class_::impl<U>>;
