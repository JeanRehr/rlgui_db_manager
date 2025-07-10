/**
 * @file logger.h
 * @brief Logger definition to log actions of users
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h> // For va_list
#include <stdio.h>
#include <time.h>

/**
 * @struct logger
 *
 * @brief Represents the logger structure to open/close file and log username
 *
 * @note The username pointer MUST be updated when users log in/out.
 */
struct logger {
    FILE *log_file;       ///< File to open/close to write to
    const char *username; ///< Pointer to the current user's username (managed externally)
};

/**
 * @brief Initializes the logger structure
 *
 * Will initialize the log structure with the desired parameters
 *
 * @param log Pointer to a logger struct to initialize
 * @param log_path pointer to the path to write to file (passing null will print to stdout)
 * @param username Pointer to the name of the username currently logged-in
 * 
 * @note Username is just a pointer, and can point to the current user logged in username to be set
 *       up automatically, just need to be careful with lifetime management
 *
 */
void logger_init(struct logger *log, const char *log_path, const char *username);

/**
 * @brief Logs events that happened
 *
 * Will log events that happened to the file in log made by the username set in log struct
 * printf style with variadic arguments
 *
 * @param log Pointer to an initialized logger struct
 * @param format format printf style (%d for integers, %s for strings, etc.)
 * @param ... Variadic arguments
 *
 * @code{.c}
 * #define LOG_PATH "log/application.log"
 * // Init logger
 * struct logger log = { 0 };
 * logger_init(&log, LOG_PATH, &current_user->username);
 * 
 * logger_log(&log, "Inserted %d %s.", 10, "Milk");
 * 
 * logger_deinit(&log);
 * @endcode
 * 
 * @example output:
 * 
 * [2025-06-05 16:30:45] User: alice | Inserted 10 Milk.
 *
 * @note __attribute__ is used to suppress warning about -Wformat-nonliteral
 *
 */
void logger_log(struct logger *log, const char *format, ...) __attribute__((format(printf, 2, 3)));

/**
 * @brief Cleans-up after the logger structure
 *
 * @param log Pointer to an initialized logger struct
 *
 */
void logger_deinit(struct logger *log);

#endif // LOGGER_H
