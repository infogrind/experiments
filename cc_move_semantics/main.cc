#include <iostream>
#include <string>
#include <utility>

class Foo {
 public:
  Foo() = delete;
  explicit Foo(std::string str);
  Foo(const Foo& rhs);
  Foo(Foo&& rhs) noexcept;
  Foo& operator=(const Foo& rhs) = delete;
  Foo& operator=(Foo&& rhs) = delete;
  Foo operator+(const Foo& rhs) const;

 private:
  std::string val_;
};

Foo::Foo(std::string str) : val_(std::move(str)) {}

Foo::Foo(const Foo& rhs) : val_(rhs.val_) {
  std::cout << "Foo(" << val_ << "): copy\n";
}

Foo::Foo(Foo&& rhs) noexcept : val_(std::move(rhs.val_)) {
  std::cout << "Foo(" << val_ << "): move\n";
}

Foo Foo::operator+(const Foo& rhs) const { return Foo(val_ + rhs.val_); }

class Bar {
 public:
  Bar() = delete;
  explicit Bar(Foo foo) : foo_(std::move(foo)) {}

 private:
  Foo foo_;
};

void msg(const std::string& str) { std::cout << str << "\n"; }

int main() {
  std::string name = "Marius";
  // Here the string argument is copied (pass by value), then moved into val_.
  Foo foo1(name);
  // Here the string parameter is constructed directly from the literal, then
  // moved into val_. No copy is made.
  Foo foo2("Kleiner");
  msg("Creating Bar from lvalue Foo. This makes a copy (pass by value) and "
      "then a move (inside Bar's constructor).");
  Bar bar1(foo1);
  msg("Creating Bar from rvalue Foo. Copy elision is guaranteed since C++17, "
      "so Foo is only moved once, inside Bar's constructor.");
  Bar bar2(foo1 + foo2);
  msg("Creating Bar from std::move(foo2). This does not use copy elision, "
      "so there is first a move to the constructor argument "
      "and then a move inside the constructor.");
  Bar bar3(std::move(foo2));
}
