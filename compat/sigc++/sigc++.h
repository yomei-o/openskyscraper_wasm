// A stand-in for the one thing OpenSkyscraper uses libsigc++ for.
//
// The whole dependency is a single `sigc::signal<void> onClicked` in
// game/classic/ui/button.h, which is declared and emitted and never connected
// to.  Vendoring libsigc++ for that would be silly, so this provides the same
// shape: connect a callable, call the signal to fire them.
#pragma once

#include <functional>
#include <utility>
#include <vector>

namespace sigc {

template <typename R, typename... Args>
class signal {
public:
    template <typename F>
    void connect(F&& slot) { slots_.emplace_back(std::forward<F>(slot)); }

    void operator()(Args... args) const {
        for (const auto& slot : slots_) slot(args...);
    }

    void emit(Args... args) const { (*this)(args...); }

    bool empty() const { return slots_.empty(); }
    void clear() { slots_.clear(); }

private:
    std::vector<std::function<R(Args...)>> slots_;
};

}  // namespace sigc
