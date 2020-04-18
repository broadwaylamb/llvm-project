// RUN: %clang_cc1 -fsyntax-only -verify %s
// expected-no-diagnostics

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

  template <typename T>
  struct X {
    struct A {};
    void f();
    enum E : int;
    static int var;
  };

  class Y {
    using Z = int;
  };

  template <>
  struct X<Y::Z>::A {};

  template <>
  void X<Y::Z>::f() {}

  template <>
  enum X<Y::Z>::E : int {};

  template <>
  int X<Y::Z>::var = 76;

}
