// RUN: %clang_cc1 -fsyntax-only -verify %s

class X {
  template <typename T> class Y {};
};

class A {
  class B {};
  class C {};

  void func();
  static void staticFunc();

  // See https://llvm.org/PR37424
  void funcOverloaded();
  void funcOverloaded(int);
  static void staticFuncOverloaded();
  static void staticFuncOverloaded(int);

  int field;
};

// C++20 [temp.spec] 17.8/6:
//   The usual access checking rules do not apply to names in a declaration of
//   an explicit instantiation or explicit specialization, with the exception
//   of names appearing in a function body, default argument, base-clause,
//   member-specification, enumerator-list, or static data member or variable
//   template initializer.
template class X::Y<A::B>;

template <void (A::*)()> class D {};
template class D<&A::func>;
template class D<&A::funcOverloaded>;

template <void (*)()> class E { };
template class E<&A::staticFunc>;
template class E<&A::staticFuncOverloaded>;

template <int A::*> class G {};
template class G<&A::field>;

template <> class X::Y<A::C> {};

namespace member_spec {

  struct A { void f(); };

  template <typename T>
  struct X {
    struct A {};
    void f(T);
    void g() {}
    enum E : int;
    static int var;
  };

  class Y {
    using Z = int;
    using T = A; // expected-note {{declared private here}}
  };

  template <>
  struct X<Y::Z>::A {};

  template <>
  void X<Y::Z>::f(Y::Z) {}

  template <typename T>
  void X<T>::f(T) {}

  template void X<Y::Z>::g();

  template <>
  enum X<Y::Z>::E : int {};

  template <>
  int X<Y::Z>::var = 76;

  void Y::T::f() {} // expected-error {{'T' is a private member of 'member_spec::Y'}}
}

namespace member_function_definition_of_class_template_specialization {

  template<typename T> struct A;

  class X {
    struct Y {}; // expected-note {{declared private here}}
  };

  template<> struct A<X::Y> {
    void f();
    int g();
  };

  void A<X::Y>::f() {}

  // A free function that returns a pointer to a field of A.
  int A<X::Y>::* g() { // expected-error {{'Y' is a private member of 'member_function_definition_of_class_template_specialization::X'}}
    return nullptr;
  }
}
