#pragma once
#include <functional>
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
    static CommandRegistry inst;
    return inst;
  }

  void add_factory(Factory f) {
    std::lock_guard<std::mutex> lk(mu_); // block
    factories_.push_back(std::move(f));
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
  mutable std::mutex mu_;
  std::vector<Factory> factories_;
};

// Регистратор загрузки команд
template <typename T> struct Registrar {
  explicit Registrar() {
    CommandRegistry::instance().add_factory(
        [] { return std::make_unique<T>(); });
  }
};

} // namespace localpm::cli

// Макрос для удобной регистрации команд
// ---------- макрос с безопасной генерацией уникального имени ----------
#define LP_JOIN_IMPL(x, y) x##y
#define LP_JOIN(x, y) LP_JOIN_IMPL(x, y)
#define REGISTER_COMMAND(CmdType)                                              \
  static ::localpm::cli::Registrar<CmdType> LP_JOIN(_lp_registrar_instance_, __COUNTER__)
