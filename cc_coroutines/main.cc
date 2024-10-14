#include <coroutine>
#include <iostream>
#include <optional>

class MyCoroutine {
 public:
  struct Promise;

  struct Promise {
    std::optional<int> value;

    // This is how the coroutine machinery knows how to construct the returned
    // object. This is how we can pass a handle to the promise object to the
    // returned object, so that a caller has acces to it.
    MyCoroutine get_return_object() {
      return std::coroutine_handle<Promise>::from_promise(*this);
    }

    // Because initial_suspend() returns suspend_always, the coroutine first
    // needs to be resumed via its handle.
    std::suspend_always initial_suspend() noexcept { return {}; }

    // This is needed in this example, because the caller of coro() below still
    // must be able to access the value via the handle to the promise object. If
    // this returned std::suspend_never, the coroutine state including the saved
    // value would be destroyed after co_return, and accessing the value would
    // be an error.
    std::suspend_always final_suspend() noexcept { return {}; }

    // This function is called when the coroutine retuns a value with co_return.
    // The only place we can store it is inside the promise object.
    void return_value(int x) { value.emplace(x); }

    // This is just a mandatory function for promise_type.
    void unhandled_exception() noexcept {}
  };

  using promise_type = Promise;

  MyCoroutine(std::coroutine_handle<promise_type> handle) : handle_(handle) {}

  ~MyCoroutine() {
    if (handle_) {
      handle_.destroy();
    }
  }

  std::optional<int> GetValue() {
    if (handle_) {
      return handle_.promise().value;
    } else {
      std::cout << "No handle!\n";
      return std::nullopt;
    }
  }

  void Resume() {
    if (handle_) {
      handle_.resume();
    }
  }

 private:
  std::coroutine_handle<promise_type> handle_;
  std::optional<int> value_;
};

MyCoroutine coro() {
  // co_await std::suspend_never();
  co_return 42;
}

int main() {
  MyCoroutine the_coro = coro();
  the_coro.Resume();

  if (the_coro.GetValue().has_value()) {
    std::cout << "The value is " << the_coro.GetValue().value() << std::endl;
  } else {
    std::cout << "It has no value!" << std::endl;
  }

  return 0;
}
