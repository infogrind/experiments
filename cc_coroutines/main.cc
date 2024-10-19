#include <coroutine>
#include <iostream>
#include <optional>

// The simplest type that I could come up with for a couroutine that returns a
// value with co_return. (If the coroutine returned void, this would be even
// simpler.)
//
// The promise object's type is the inner `Promise` type (aliased to the
// mandatory `promise_type`). When the coroutine returns a value with co_return,
// the value is passed to the promise object via return_value().
//
// The return value of final_suspend() determines what happens after the
// coroutine calls co_return. If it is std::suspend_never, the coroutine is
// immediately destroyed and the handle becomes invalid. Here we therefore
// return std::suspend_always, so that the calling function can still access
// the return value via the promise handle.
class MyCoroutine {
 public:
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
    // Make sure that the promise object is destroyed, if it hasn't already
    // happened. The test if(handle_) is likely useless, since the handle itself
    // is like a pointer, it doesn't know if it points to something valid, and
    // it isn't set to nullptr when the promise object is destroyed.
    if (handle_) {
      std::cout << "Destroying the coroutine handle!\n";
      handle_.destroy();
    }
  }

  // This accesses the value via the handle to the promise object.
  std::optional<int> GetValue() {
    if (handle_) {
      return handle_.promise().value;
    } else {
      // This will likely never happen, see also comment above.
      std::cout << "No handle!\n";
      return std::nullopt;
    }
  }

  // Needed because initial_suspend returns std::suspend_always.
  void Resume() {
    if (handle_) {
      // Again, as in the example above, the test if(handle_) is likely useless.
      handle_.resume();
    }
  }

 private:
  // Through this handle can the coroutine object access the promise object.
  std::coroutine_handle<promise_type> handle_;

  // This is how we store the value for later retrieval.
  std::optional<int> value_;
};

// This is the simplest version I could find of an awaitable. Because
// await_ready() returns true, the coroutine is not suspended when calling
// co_await on the awaitable, but instead it immediately calls await_resume.
struct MyAwaitable {
  bool await_ready() { return true; }

  void await_suspend(std::coroutine_handle<>) {}

  int await_resume() noexcept { return 42; }
};

MyCoroutine coro() {
  // co_await std::suspend_never();
  co_return co_await MyAwaitable();
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
