/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

/**
 * @enum ResponseStatus
 * @brief Represents the status of an operation, indicating success or various types of failure.
 *
 * This enum defines the possible outcomes for operations and is used to indicate the
 * result of various methods and network interactions. Each status is associated with
 * a specific integer value.
 *
 * The following statuses are defined:
 * - {@code OK} (0) - The operation was successful.
 * - {@code ERROR} (-1) - A general error occurred during the operation.
 * - {@code LIMIT_TOO_LOW} (1) - The provided limit for the GET* operation is too low.
 * - {@code CONVERSION_ERROR} (-2) - There was an error during conversion of payload data.
 *
 * @note TODO: Ensure consistency across language implementations.
 */
public enum ResponseStatus {

    /** @brief Operation completed successfully. */
    OK(0),

    /** @brief General error occurred during the operation. */
    ERROR(-1),

    /** @brief The limit provided for the GET* operation is too low. */
    LIMIT_TOO_LOW(1),

    /** @brief An error occurred during payload data conversion. */
    CONVERSION_ERROR(-2);

    /**
     * @brief Constructs a ResponseStatus with the specified integer value.
     *
     * @param value The integer value associated with the status.
     */
    ResponseStatus(int value) {
    }
}
