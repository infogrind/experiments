#include <coroutine>
#include <exception>
#include <iostream>
#include <optional>
#include <utility>

// Terminology used in the comments below:
//
// - The *coroutine* is the function coro() further down. A function becomes a
//   coroutine simply by using co_await, co_yield or co_return in its body.
// - The *coroutine frame* (or coroutine state) is a heap-allocated block that
//   the compiler creates when coro() is called. It holds copies of the
//   parameters, the local variables that live across suspension points, the
//   promise object, and bookkeeping about where to continue on resume.
// - The *promise object* lives inside the frame. Its type is
//   MyCoroutine::promise_type. The compiler calls its member functions at fixed
//   points in the coroutine's life, and it is the only place where results can
//   be passed from the coroutine to the outside world.
// - The *return object* is what the caller of coro() gets back, here an
//   instance of MyCoroutine. The compiler finds promise_type through the
//   coroutine's return type, so MyCoroutine is what connects coro() to
//   Promise.
// - A *coroutine handle* (std::coroutine_handle<Promise>) is a non-owning
//   pointer to the frame. Through it you can resume() the coroutine, ask
//   whether it is done(), reach the promise(), and destroy() the frame.
//
// What happens in this program:
//
//   1. main() calls coro(). The frame is allocated and the Promise is
//      constructed inside it.
//   2. promise.get_return_object() creates the MyCoroutine that the caller will
//      receive.
//   3. The body starts with an implicit `co_await promise.initial_suspend()`.
//      That returns std::suspend_always, so the coroutine suspends before
//      running any of the code in its body, and coro() returns to main().
//   4. main() calls Resume(). The body runs, and `co_return x` calls
//      promise.return_value(x).
//   5. The body ends with an implicit `co_await promise.final_suspend()`. That
//      also returns std::suspend_always, so the coroutine suspends one last
//      time. The frame, and with it the stored value, stays alive, and
//      handle.done() now returns true.
//   6. main() reads the value through the handle. When the MyCoroutine goes out
//      of scope, its destructor calls handle.destroy() to free the frame.
//
// MyCoroutine is an RAII owner of the frame, similar to std::unique_ptr: it can
// be moved but not copied, and it destroys the frame exactly once.
class MyCoroutine {
 public:
  struct Promise {
    // The value passed to co_return. It is empty until the coroutine finishes.
    std::optional<int> value;

    // The exception that escaped the coroutine body, if there was one.
    std::exception_ptr exception;

    // Called once, right after the Promise is constructed, to create the object
    // that the caller of coro() receives. from_promise() computes the frame's
    // address from the promise's address. That works because the promise sits
    // at a fixed offset inside the frame.
    MyCoroutine get_return_object() {
      return MyCoroutine{std::coroutine_handle<Promise>::from_promise(*this)};
    }

    // Returning std::suspend_always makes the coroutine "lazy": nothing in the
    // body runs until someone calls Resume(). With std::suspend_never, the body
    // would start running right away, inside the call to coro().
    std::suspend_always initial_suspend() noexcept { return {}; }

    // Returning std::suspend_always keeps the frame alive after the body has
    // finished, so that the caller can still read `value` through the handle.
    // The caller is then responsible for calling destroy().
    //
    // With std::suspend_never, the frame would destroy itself right after
    // co_return. `value` would be gone, and the destroy() in ~MyCoroutine
    // would free the frame a second time. final_suspend() must be noexcept.
    std::suspend_always final_suspend() noexcept { return {}; }

    // `co_return x;` calls this function. The coroutine's local variables are
    // destroyed right after it, so the promise is where the value must go. A
    // coroutine that only uses `co_return;` would define return_void()
    // instead. A promise type may have one of them, but not both.
    void return_value(int x) { value.emplace(x); }

    // The compiler wraps the body in a try/catch, and this function is called
    // from inside the catch block if an exception escapes the body. Afterwards,
    // the coroutine goes to final_suspend() as if it had finished normally. If
    // this function did nothing, the exception would be lost without a trace.
    // So we store it here and rethrow it in GetValue().
    void unhandled_exception() noexcept {
      exception = std::current_exception();
    }
  };

  // The compiler looks up this exact name in the coroutine's return type.
  using promise_type = Promise;

  explicit MyCoroutine(std::coroutine_handle<promise_type> handle)
      : handle_(handle) {}

  // A copy would destroy the same frame twice.
  MyCoroutine(const MyCoroutine&) = delete;
  MyCoroutine& operator=(const MyCoroutine&) = delete;

  // Moving transfers ownership of the frame. The moved-from object's handle is
  // set to null, so that its destructor does nothing.
  MyCoroutine(MyCoroutine&& other) noexcept
      : handle_(std::exchange(other.handle_, nullptr)) {}

  MyCoroutine& operator=(MyCoroutine&& other) noexcept {
    if (this != &other) {
      if (handle_) {
        handle_.destroy();
      }
      handle_ = std::exchange(other.handle_, nullptr);
    }
    return *this;
  }

  ~MyCoroutine() {
    // A handle cannot tell whether the frame it points to is still alive. It
    // is just a pointer. Here, the frame is only ever destroyed by the object
    // that owns it, and moving clears the handle. So a non-null handle means
    // "this object owns a live frame". destroy() runs the destructors of the
    // promise, the parameter copies and any live local variables, and then
    // frees the frame's memory.
    if (handle_) {
      std::cout << "Destroying the coroutine handle!\n";
      handle_.destroy();
    }
  }

  // Returns the value passed to co_return. The optional is empty if the
  // coroutine hasn't finished yet. If the coroutine body threw an exception,
  // it is rethrown here.
  std::optional<int> GetValue() const {
    if (!handle_) {
      // Only happens after this object has been moved from.
      std::cout << "No handle!\n";
      return std::nullopt;
    }
    const Promise& promise = handle_.promise();
    if (promise.exception) {
      std::rethrow_exception(promise.exception);
    }
    return promise.value;
  }

  // Runs the coroutine until its next suspension point. Here, that is the
  // final suspend point, because coro() never suspends on its own. Resuming a
  // coroutine that is already done() is undefined behavior, so in that case we
  // do nothing.
  void Resume() {
    if (handle_ && !handle_.done()) {
      handle_.resume();
    }
  }

 private:
  // The handle is the only way for MyCoroutine to reach the frame, and
  // through the frame, the promise.
  std::coroutine_handle<promise_type> handle_;
};

// The simplest possible awaitable. `co_await expr` does the following:
//
//   1. It calls expr.await_ready(). If that returns true, the result is already
//      available and the coroutine doesn't suspend.
//   2. Otherwise, the coroutine suspends and await_suspend(handle) is called
//      with a handle to the awaiting coroutine. Whoever holds that handle can
//      resume the coroutine later.
//   3. When the coroutine continues (right away if await_ready() returned
//      true), the value returned by await_resume() is the result of the
//      co_await expression.
//
// Because await_ready() returns true here, await_suspend() is never called.
// It still has to exist, though, because the compiler checks all three
// functions whenever the type is used with co_await.
struct MyAwaitable {
  bool await_ready() const noexcept { return true; }

  void await_suspend(std::coroutine_handle<>) const noexcept {}

  int await_resume() const noexcept { return 42; }
};

// Since co_await never suspends here, this is effectively `co_return 42;`.
MyCoroutine coro() { co_return co_await MyAwaitable(); }

int main() {
  // Because of initial_suspend(), none of the code in coro()'s body has run
  // yet at this point.
  MyCoroutine the_coro = coro();

  // Runs the body to completion. The coroutine then remains suspended at its
  // final suspend point.
  the_coro.Resume();

  if (std::optional<int> value = the_coro.GetValue()) {
    std::cout << "The value is " << *value << std::endl;
  } else {
    std::cout << "It has no value!" << std::endl;
  }

  // the_coro goes out of scope here, and its destructor destroys the frame.
  return 0;
}
