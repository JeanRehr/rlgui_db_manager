/**
 * @file error_handling.h
 * @brief Application Error Codes
 * 
 * Standardized error codes used throughout the application for consistent
 * error reporting and handling. These codes represent possible failure modes
 * across different modules.
 * 
 * @note These are not global variables - they're compile-time constants
 */

#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

/**
 * @enum error_code
 * @brief Numeric error codes for application failures
 * 
 * Values are designed to be used as function return codes and can be
 * converted to human-readable messages via error_code_to_string().
 */
enum error_code {
    NO_ERROR = 0,            ///< Operation completed successfully
    ERROR_OPENING_DB,        ///< Failed to open database connection
    ERROR_CREATING_TABLE_DB, ///< Failed to create database table
    ERROR_INSERT_DB,         ///< Failed to insert database record
    ERROR_UPDATE_DB,         ///< Failed to update database record
    ERROR_DELETE_DB,         ///< Failed to delete database record
};

#endif // ERROR_HANDLING_H