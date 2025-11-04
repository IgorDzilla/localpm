#pragma once
#include <functional> // Integral equations
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace CLI {
class App;
}

namespace localpm::cli {

struct Command {
  virtual ~Command() = default;
  virtual std::string name() const = 0;                  // name command
  virtual std::string description() const { return {}; } // help
  virtual void configure(CLI::App &sub) = 0;             // options and flags
  virtual int run() = 0;                                 // exit code
};

class CommandRegistry {
public:
  using Factory = std::function<std::unique_ptr<Command>()>;

  static CommandRegistry &instance() {
    // static внутри функции имеет область видимости функции
    // но при этом область жизни у него бесконечная
    static CommandRegistry inst;
    return inst;
  }

  template <typename T> bool register_type() {
    std::lock_guard<std::mutex> lk(mu_); // block
    factories_.push_back(&CommandRegistry::factory<T>);
    return true;
  }

  std::vector<std::unique_ptr<Command>> instantiate_all() const {
    std::vector<std::unique_ptr<Command>> out;
    out.reserve(factories_.size());
    for (auto const &f : factories_) {
      out.push_back(f());
    }
    return out;
  }

private:
  template <typename T> static std::unique_ptr<Command> factory() {
    return std::make_unique<T>();
  }

  mutable std::mutex mu_;
  std::vector<Factory> factories_;
};

} // namespace localpm::cli
