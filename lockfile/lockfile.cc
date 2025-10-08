#include <iostream>
#include <optional>
#include <string>
#include <tomlplusplus/toml.hpp>
#include <unordered_map>
#include <vector>

struct CompilerMin {
  std::string cc;
  std::vector<std::string> cflags;
};

struct ProjectMin {
  std::string name;
  std::string version;
  std::optional<CompilerMin> compiler;
};

struct SourceMin {
  std::string type; // registry|git|path|tar (в мини-примере — registry)
  std::string url;
  std::string package;
  std::string rev; // версия/тег
};

struct IntegrityMin {
  std::optional<std::string> tarball_sha256;
};

struct DepMin {
  std::string name;
  std::string constraint;
  std::string resolved;
};

struct PackageMin {
  std::string key; // "name@version"
  std::string name;
  std::string version;
  std::optional<SourceMin> source;
  std::optional<IntegrityMin> integrity;
  std::vector<DepMin> dependencies;
};

struct LockMin {
  int64_t schema = 0;
  ProjectMin project;
  std::unordered_map<std::string, PackageMin> packages; // key -> pkg
};

static std::vector<std::string> arr_str(const toml::array *a) {
  std::vector<std::string> out;
  if (!a)
    return out;
  for (auto &n : *a)
    if (auto v = n.value<std::string>())
      out.push_back(*v);
  return out;
}

static std::optional<CompilerMin> parse_compiler_min(const toml::table &t) {
  CompilerMin c;
  if (auto v = t.get("cc"))
    if (auto s = v->value<std::string>())
      c.cc = *s;
  c.cflags = arr_str(t.get_as<toml::array>("cflags"));
  if (c.cc.empty() && c.cflags.empty())
    return std::nullopt;
  return c;
}

static std::optional<SourceMin> parse_source_min(const toml::table &t) {
  SourceMin s;
  if (auto v = t.get("type"))
    if (auto x = v->value<std::string>())
      s.type = *x;
  if (auto v = t.get("url"))
    if (auto x = v->value<std::string>())
      s.url = *x;
  if (auto v = t.get("package"))
    if (auto x = v->value<std::string>())
      s.package = *x;
  if (auto v = t.get("rev"))
    if (auto x = v->value<std::string>())
      s.rev = *x;
  if (s.type.empty())
    return std::nullopt;
  return s;
}

static std::optional<IntegrityMin> parse_integrity_min(const toml::table &t) {
  IntegrityMin i;
  if (auto v = t.get("tarball_sha256"))
    if (auto x = v->value<std::string>())
      i.tarball_sha256 = *x;
  if (!i.tarball_sha256)
    return std::nullopt;
  return i;
}

static DepMin parse_dep_min(const toml::table &t) {
  DepMin d;
  if (auto v = t.get("name"))
    if (auto x = v->value<std::string>())
      d.name = *x;
  if (auto v = t.get("constraint"))
    if (auto x = v->value<std::string>())
      d.constraint = *x;
  if (auto v = t.get("resolved"))
    if (auto x = v->value<std::string>())
      d.resolved = *x;
  return d;
}

static PackageMin parse_pkg_min(const std::string &key, const toml::table &t) {
  PackageMin p;
  p.key = key;
  if (auto v = t.get("name"))
    if (auto x = v->value<std::string>())
      p.name = *x;
  if (auto v = t.get("version"))
    if (auto x = v->value<std::string>())
      p.version = *x;

  if (auto s = t.get_as<toml::table>("source"))
    p.source = parse_source_min(*s);
  if (auto i = t.get_as<toml::table>("integrity"))
    p.integrity = parse_integrity_min(*i);

  if (auto deps = t.get_as<toml::array>("dependencies")) {
    for (auto &n : *deps)
      if (auto *dt = n.as_table())
        p.dependencies.push_back(parse_dep_min(*dt));
  }
  return p;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "usage: mini-lock-validate <path/to/localpm.lock>\n";
    return 2;
  }
  toml::parse_result pr = toml::parse_file(argv[1]);
  if (!pr) {
    auto err = pr.error();
    std::cerr << "TOML parse error: " << err.description() << "\n";
    if (err.source().path)
      std::cerr << "  file: " << err.source().path->str() << "\n";
    if (err.source().begin)
      std::cerr << "  at: " << err.source().begin->line << ":"
                << err.source().begin->column << "\n";
    return 1;
  }
  auto &root = pr.table();

  LockMin lock;

  // [lockfile]
  if (auto *lf = root.get_as<toml::table>("lockfile")) {
    if (auto v = lf->get("schema"))
      if (auto s = v->value<int64_t>())
        lock.schema = *s;
  } else {
    std::cerr << "error: missing [lockfile]\n";
    return 1;
  }
  if (lock.schema != 1) {
    std::cerr << "error: lockfile.schema must be 1 (got " << lock.schema
              << ")\n";
    return 1;
  }

  // [project]
  if (auto *pj = root.get_as<toml::table>("project")) {
    if (auto v = pj->get("name"))
      if (auto s = v->value<std::string>())
        lock.project.name = *s;
    if (auto v = pj->get("version"))
      if (auto s = v->value<std::string>())
        lock.project.version = *s;
    if (auto *comp = pj->get_as<toml::table>("compiler"))
      lock.project.compiler = parse_compiler_min(*comp);
  } else {
    std::cerr << "error: missing [project]\n";
    return 1;
  }
  if (lock.project.name.empty() || lock.project.version.empty()) {
    std::cerr << "error: project.name and project.version are required\n";
    return 1;
  }

  // [packages]
  if (auto *pkgs = root.get_as<toml::table>("packages")) {
    for (auto &&[k, v] : *pkgs) {
      if (auto *t = v.as_table()) {
        auto pkg = parse_pkg_min(std::string{k}, *t);
        lock.packages.emplace(pkg.key, std::move(pkg));
      }
    }
  } else {
    std::cerr << "error: missing [packages]\n";
    return 1;
  }
  if (lock.packages.empty()) {
    std::cerr << "error: [packages] must have at least one entry\n";
    return 1;
  }

  // Выведем краткую сводку (как smoke-test)
  std::cout << "OK: " << lock.project.name << "@" << lock.project.version
            << ", packages=" << lock.packages.size() << "\n";
  for (auto &[key, p] : lock.packages) {
    std::cout << "  - " << key;
    if (p.source && !p.source->type.empty())
      std::cout << " src=" << p.source->type;
    if (p.integrity && p.integrity->tarball_sha256)
      std::cout << " sha256=" << *p.integrity->tarball_sha256;
    std::cout << "\n";
    for (auto &d : p.dependencies) {
      std::cout << "      dep " << d.name << " " << d.constraint << " -> "
                << d.resolved << "\n";
    }
  }
  return 0;
}
