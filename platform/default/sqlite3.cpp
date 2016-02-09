#include "sqlite3.hpp"
#include <sqlite3.h>

#include <cassert>
#include <cstring>
#include <cstdio>
#include <chrono>
#include <experimental/optional>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
// Check sqlite3 library version.
const static bool sqliteVersionCheck = []() {
    if (sqlite3_libversion_number() / 1000000 != SQLITE_VERSION_NUMBER / 1000000) {
        char message[96];
        snprintf(message, 96,
                 "sqlite3 libversion mismatch: headers report %d, but library reports %d",
                 SQLITE_VERSION_NUMBER, sqlite3_libversion_number());
        throw std::runtime_error(message);
    }

    return true;
}();
#pragma GCC diagnostic pop

namespace mapbox {
namespace sqlite {

template <typename T>
using optional = std::experimental::optional<T>;

Database::Database(const std::string &filename, int flags) {
    const int err = sqlite3_open_v2(filename.c_str(), &db, flags, nullptr);
    if (err != SQLITE_OK) {
        const auto message = sqlite3_errmsg(db);
        db = nullptr;
        throw Exception { err, message };
    }
}

Database::Database(Database &&other)
    : db(std::move(other.db)) {}

Database &Database::operator=(Database &&other) {
    std::swap(db, other.db);
    return *this;
}

Database::~Database() {
    if (db) {
        const int err = sqlite3_close(db);
        if (err != SQLITE_OK) {
            throw Exception { err, sqlite3_errmsg(db) };
        }
    }
}

Database::operator bool() const {
    return db != nullptr;
}

void Database::setBusyTimeout(std::chrono::milliseconds timeout) {
    assert(db);
    const int err = sqlite3_busy_timeout(db,
        int(std::min<std::chrono::milliseconds::rep>(timeout.count(), std::numeric_limits<int>::max())));
    if (err != SQLITE_OK) {
        throw Exception { err, sqlite3_errmsg(db) };
    }
}

void Database::exec(const std::string &sql) {
    assert(db);
    char *msg = nullptr;
    const int err = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &msg);
    if (msg) {
        const std::string message = msg;
        sqlite3_free(msg);
        throw Exception { err, message };
    } else if (err != SQLITE_OK) {
        throw Exception { err, sqlite3_errmsg(db) };
    }
}

Statement Database::prepare(const char *query) {
    assert(db);
    return Statement(db, query);
}

int64_t Database::lastInsertRowid() const {
    assert(db);
    return sqlite3_last_insert_rowid(db);
}

Statement::Statement(sqlite3 *db, const char *sql) {
    const int err = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (err != SQLITE_OK) {
        stmt = nullptr;
        throw Exception { err, sqlite3_errmsg(db) };
    }
}

Statement::Statement(Statement &&other) {
    *this = std::move(other);
}

Statement &Statement::operator=(Statement &&other) {
    std::swap(stmt, other.stmt);
    return *this;
}

Statement::~Statement() {
    if (stmt) {
        sqlite3_finalize(stmt);
    }
}

Statement::operator bool() const {
    return stmt != nullptr;
}

void Statement::check(int err) {
    if (err != SQLITE_OK) {
        throw Exception { err, sqlite3_errmsg(sqlite3_db_handle(stmt)) };
    }
}

template <> void Statement::bind(int offset, std::nullptr_t) {
    assert(stmt);
    check(sqlite3_bind_null(stmt, offset));
}

template <> void Statement::bind(int offset, int8_t value) {
    assert(stmt);
    check(sqlite3_bind_int64(stmt, offset, value));
}

template <> void Statement::bind(int offset, int16_t value) {
    assert(stmt);
    check(sqlite3_bind_int64(stmt, offset, value));
}

template <> void Statement::bind(int offset, int32_t value) {
    assert(stmt);
    check(sqlite3_bind_int64(stmt, offset, value));
}

template <> void Statement::bind(int offset, int64_t value) {
    assert(stmt);
    check(sqlite3_bind_int64(stmt, offset, value));
}

template <> void Statement::bind(int offset, uint8_t value) {
    assert(stmt);
    check(sqlite3_bind_int64(stmt, offset, value));
}

template <> void Statement::bind(int offset, uint16_t value) {
    assert(stmt);
    check(sqlite3_bind_int64(stmt, offset, value));
}

template <> void Statement::bind(int offset, uint32_t value) {
    assert(stmt);
    check(sqlite3_bind_int64(stmt, offset, value));
}

template <> void Statement::bind(int offset, float value) {
    assert(stmt);
    check(sqlite3_bind_double(stmt, offset, value));
}

template <> void Statement::bind(int offset, double value) {
    assert(stmt);
    check(sqlite3_bind_double(stmt, offset, value));
}

template <> void Statement::bind(int offset, bool value) {
    assert(stmt);
    check(sqlite3_bind_int(stmt, offset, value));
}

template <> void Statement::bind(int offset, const char *value) {
    assert(stmt);
    check(sqlite3_bind_text(stmt, offset, value, -1, SQLITE_STATIC));
}

// We currently cannot use sqlite3_bind_blob64 / sqlite3_bind_text64 because they
// was introduced in SQLite 3.8.7, and we need to support earlier versions:
//    iOS 7.0: 3.7.13
//    iOS 8.2: 3.8.5
// According to http://stackoverflow.com/questions/14288128/what-version-of-sqlite-does-ios-provide,
// the first iOS version with 3.8.7+ was 9.0, with 3.8.10.2.

void Statement::bind(int offset, const char * value, std::size_t length, bool retain) {
    assert(stmt);
    if (length > std::numeric_limits<int>::max()) {
        throw std::range_error("value too long for sqlite3_bind_text");
    }
    check(sqlite3_bind_text(stmt, offset, value, int(length),
                            retain ? SQLITE_TRANSIENT : SQLITE_STATIC));
}

void Statement::bind(int offset, const std::string& value, bool retain) {
    bind(offset, value.data(), value.size(), retain);
}

void Statement::bindBlob(int offset, const void * value, std::size_t length, bool retain) {
    assert(stmt);
    if (length > std::numeric_limits<int>::max()) {
        throw std::range_error("value too long for sqlite3_bind_text");
    }
    check(sqlite3_bind_blob(stmt, offset, value, int(length),
                            retain ? SQLITE_TRANSIENT : SQLITE_STATIC));
}

void Statement::bindBlob(int offset, const std::vector<uint8_t>& value, bool retain) {
    bindBlob(offset, value.data(), value.size(), retain);
}

template <> void Statement::bind(int offset, std::chrono::system_clock::time_point value) {
    assert(stmt);
    check(sqlite3_bind_int64(stmt, offset, std::chrono::system_clock::to_time_t(value)));
}

template <> void Statement::bind(int offset, optional<std::string> value) {
    if (!value) {
        bind(offset, nullptr);
    } else {
        bind(offset, *value);
    }
}

template <> void Statement::bind(int offset, optional<std::chrono::system_clock::time_point> value) {
    if (!value) {
        bind(offset, nullptr);
    } else {
        bind(offset, *value);
    }
}

bool Statement::run() {
    assert(stmt);
    const int err = sqlite3_step(stmt);
    if (err == SQLITE_DONE) {
        return false;
    } else if (err == SQLITE_ROW) {
        return true;
    } else if (err != SQLITE_OK) {
        throw Exception { err, sqlite3_errmsg(sqlite3_db_handle(stmt)) };
    } else {
        return false;
    }
}

template <> int Statement::get(int offset) {
    assert(stmt);
    return sqlite3_column_int(stmt, offset);
}

template <> int64_t Statement::get(int offset) {
    assert(stmt);
    return sqlite3_column_int64(stmt, offset);
}

template <> double Statement::get(int offset) {
    assert(stmt);
    return sqlite3_column_double(stmt, offset);
}

template <> std::string Statement::get(int offset) {
    assert(stmt);
    return {
        reinterpret_cast<const char *>(sqlite3_column_blob(stmt, offset)),
        size_t(sqlite3_column_bytes(stmt, offset))
    };
}

template <> std::vector<uint8_t> Statement::get(int offset) {
    assert(stmt);
    const uint8_t* begin = reinterpret_cast<const uint8_t*>(sqlite3_column_blob(stmt, offset));
    const uint8_t* end   = begin + sqlite3_column_bytes(stmt, offset);
    return { begin, end };
}

template <> std::chrono::system_clock::time_point Statement::get(int offset) {
    assert(stmt);
    return std::chrono::system_clock::from_time_t(sqlite3_column_int64(stmt, offset));
}

template <> optional<std::string> Statement::get(int offset) {
    assert(stmt);
    if (sqlite3_column_type(stmt, offset) == SQLITE_NULL) {
        return optional<std::string>();
    } else {
        return get<std::string>(offset);
    }
}

template <> optional<std::chrono::system_clock::time_point> Statement::get(int offset) {
    assert(stmt);
    if (sqlite3_column_type(stmt, offset) == SQLITE_NULL) {
        return optional<std::chrono::system_clock::time_point>();
    } else {
        return get<std::chrono::system_clock::time_point>(offset);
    }
}

void Statement::reset() {
    assert(stmt);
    sqlite3_reset(stmt);
}

void Statement::clearBindings() {
    assert(stmt);
    sqlite3_clear_bindings(stmt);
}

} // namespace sqlite
} // namespace mapbox
