# Registry file system and indexing

# LocalPM: спецификация хранилища локальных пакетов и индекса (SQLite)

## 0) Термины

* **Package (пакет)**: именуемая сущность `namespace/name` с версиями.
* **Version (версия)**: SemVer (`MAJOR.MINOR.PATCH[-PRERELEASE][+BUILD]`) в нормализованном виде.
* **Store (хранилище)**: директория `~/.localpm` (по умолчанию) либо путь из конфигурации.
* **Index (индекс)**: база SQLite `index.db`, описывающая содержимое `packages/`.

---

## 1) Файловая структура Store

### 1.1 Корень

```
<STORE>/
├─ config.toml               # глобальная конфигурация LocalPM
├─ logs/                     # логи операций
├─ cache/                    # кэш загрузок/сборок
├─ index/                    # служебные индексы (необяз.)
│  ├─ remotes/               # кэш внешних реестров (необяз.)
│  └─ state.json             # ревизия сканирования (необяз.)
├─ packages/                 # ***основное хранилище пакетов***
└─ index.db                  # ***SQLite-индекс пакетов***
```

### 1.2 Дерево `packages/`

```
packages/
└─ <namespace>/                  # [a-z0-9][a-z0-9._-]* (≤ 64)
   └─ <package-name>/            # [a-z0-9][a-z0-9._-]* (≤ 64)
      ├─ <version>/              # нормализованный SemVer
      │  ├─ manifest.toml        # обязательный манифест пакета
      │  ├─ source/              # исходники/ресурсы
      │  ├─ build/               # артефакты сборки (опц.)
      │  └─ meta.json            # производные метаданные (опц.)
      └─ latest -> <version>/    # симлинк на последнюю стабильную (опц.)
```

### 1.3 Ограничения и нормализация

* `namespace`/`package-name`: нижний регистр, `a-z0-9._-`, длина ≤ 64.
* `version`: SemVer без ведущих нулей в `MAJOR.MINOR.PATCH`; пререлизы/build сохраняются.

### 1.4 Источник пакета (в `manifest.toml`)

```toml
[package]
name = "logger"                 # == <package-name>
namespace = "core"              # == <namespace>
version = "1.2.3"
type = "static-lib"             # enum: static-lib|shared-lib|abi|header-only|other

[source]                        # один из вариантов:
type = "local"                  # local|git|registry|remote
path = "/abs/or/relative/path"

# [source]
# type = "git"
# url = "https://github.com/myorg/logger.git"
# rev = "abc123"

# [source]
# type = "vendor"
# id = "acme-registry"
# ref = "logger/1.2.3"
```

### 1.5 Пример дерева

```
~/.localpm/
└─ packages/core/logger/
   ├─ 1.2.3/
   │  ├─ manifest.toml
   │  ├─ source/...
   │  └─ build/...
   └─ latest -> 1.2.3/
```

---

## 2) Формат `manifest.toml`

```toml
[package]
name = "logger"
namespace = "core"
version = "1.2.3"
type = "static-lib"

[metadata]                      # опц.
license = "MIT"
description = "Lightweight logger"
homepage = "https://example.org/logger"

[dependencies]                  # опц. (короткая форма)
fmt = ">=9.0, <11.0"
core/config = "~1.0"

# или длинная форма:
# [[deps]]
# name = "fmt"
# constraint = ">=9.0, <11.0"
# optional = false
```

> При индексации обе формы зависимостей нормализуются в таблицу `dependencies`.

---

## 3) Индексация (SQLite)

### 3.1 Файл

```
<STORE>/index.db
```

### 3.2 Схема БД

```sql
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS packages (
  id            INTEGER PRIMARY KEY AUTOINCREMENT,
  name          TEXT    NOT NULL,  -- "logger"
  namespace     TEXT    NOT NULL,  -- "core"
  version       TEXT    NOT NULL,  -- нормализованный SemVer
  path          TEXT    NOT NULL,  -- абсолютный путь к каталогу версии
  source_type   TEXT    NOT NULL CHECK(source_type IN ('local','git','vendor','remote')),
  pkg_type      TEXT    NOT NULL CHECK(pkg_type IN ('static-lib','shared-lib','abi','header-only','other')),
  manifest_hash TEXT,              -- SHA256(manifest.toml)
  created_at    INTEGER NOT NULL DEFAULT (strftime('%s','now')),
  updated_at    INTEGER NOT NULL DEFAULT (strftime('%s','now')),
  UNIQUE(namespace, name, version)
);

CREATE TABLE IF NOT EXISTS dependencies (
  package_id    INTEGER NOT NULL,
  dep_namespace TEXT    NOT NULL,  -- если не указан, берётся "default"
  dep_name      TEXT    NOT NULL,
  constraint    TEXT,              -- строка спецификации версий
  optional      INTEGER NOT NULL DEFAULT 0,
  FOREIGN KEY(package_id) REFERENCES packages(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_pkg_lookup
  ON packages(namespace, name, version);
CREATE INDEX IF NOT EXISTS idx_pkg_name
  ON packages(namespace, name);
CREATE INDEX IF NOT EXISTS idx_pkg_source
  ON packages(source_type);
CREATE INDEX IF NOT EXISTS idx_deps_pkg
  ON dependencies(package_id);
CREATE INDEX IF NOT EXISTS idx_deps_target
  ON dependencies(dep_namespace, dep_name);
```

### 3.3 Транзакции

* Любая операция установки/удаления — в транзакции:

  ```sql
  BEGIN IMMEDIATE;
  -- изменения
  COMMIT;
  ```
* Пакет и его зависимости записываются атомарно.

### 3.4 Базовые операции

**Upsert пакета**

```sql
INSERT INTO packages (namespace,name,version,path,source_type,pkg_type,manifest_hash)
VALUES (:ns,:name,:ver,:path,:src,:pkg,:hash)
ON CONFLICT(namespace,name,version) DO UPDATE SET
  path=excluded.path,
  source_type=excluded.source_type,
  pkg_type=excluded.pkg_type,
  manifest_hash=excluded.manifest_hash,
  updated_at=strftime('%s','now');
```

**Обновление зависимостей**

```sql
DELETE FROM dependencies
WHERE package_id = (SELECT id FROM packages WHERE namespace=:ns AND name=:name AND version=:ver);

INSERT INTO dependencies (package_id,dep_namespace,dep_name,constraint,optional)
VALUES (
  (SELECT id FROM packages WHERE namespace=:ns AND name=:name AND version=:ver),
  :dep_ns, :dep_name, :constraint, :optional
);
```

**Поиск конкретной версии**

```sql
SELECT * FROM packages
WHERE namespace=:ns AND name=:name AND version=:ver;
```

**Последняя стабильная версия**

> Если храните версии как нормализованные строки, допустима сортировка по `version`.

```sql
SELECT * FROM packages
WHERE namespace=:ns AND name=:name
ORDER BY version DESC
LIMIT 1;
```

**Обратные зависимости**

```sql
SELECT p.namespace, p.name, p.version
FROM packages p
JOIN dependencies d ON p.id = d.package_id
WHERE d.dep_namespace=:ns AND d.dep_name=:name;
```

---

## 4) Нормализация SemVer (правила)

* `MAJOR.MINOR.PATCH` — целые без ведущих нулей.
* Пререлизы (`-alpha.1`) и build-метки сохраняются.
* Для точной сортировки можно иметь вычисляемый ключ в памяти:

  ```
  key = <major:8d>.<minor:8d>.<patch:8d>~<pre-normalized>
  ```

  (в БД хранится исходная строка версии; ключ — transient).

---

## 5) Консистентность FS ↔ Index

### 5.1 Инварианты

* На каждый каталог `packages/<ns>/<name>/<ver>/manifest.toml` — **ровно одна** запись в `packages`.
* `path` указывает на существующий каталог.
* `manifest_hash` соответствует текущему `manifest.toml`.

### 5.2 Полное сканирование

1. `BEGIN IMMEDIATE;`
2. Рекурсивный обход `packages/`, валидация `manifest.toml`.
3. Для каждой версии — upsert в `packages` + полная перезапись `dependencies`.
4. Опционально: удалить «висячие» записи (есть в БД, нет на диске).
5. `COMMIT;`

### 5.3 Инкрементальное сканирование

* Хранить `index/state.json` с `mtime/size/hash` по `manifest.toml`.
* Обновлять только изменённые пакеты.
* При изменении хеша — пересчитать `dependencies`.

---

## 6) Симлинк `latest`

* Необязательный ускоритель для UX.
* Точка истины — индекс; симлинк не влияет на индексацию.

---

## 7) Коды ошибок

* `E_MANIFEST_MISSING` — отсутствует `manifest.toml`.
* `E_MANIFEST_INVALID` — невалидная схема/тип.
* `E_VERSION_INVALID` — версия не соответствует SemVer.
* `E_DUPLICATE_VERSION` — дублирующий каталог версии.
* `E_INDEX_CONFLICT` — конфликт уникальности в БД (диск ↔ индекс рассинхронизированы).

---

## 8) Права и безопасность

* `index.db` — права 0600 (только владелец).
* `packages/` — права пользователя; исполнение из `source/` запрещать политиками ОС/umask (на усмотрение).
* Перед сборкой/линковкой опционально сверять `manifest_hash`.

---

## 9) CLI/API контракты

### 9.1 CLI

```
localpm scan            # полное сканирование FS → index.db
localpm add <path>      # импорт локальной версии (создаёт каталог и индекс)
localpm rm <pkg>[@ver]  # удаление версии и записи из БД
localpm ls <pkg>        # перечислить версии из индекса
localpm deps <pkg>@ver  # показать зависимости (из БД)
```

