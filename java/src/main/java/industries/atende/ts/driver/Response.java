/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.util.Objects;

/**
 * @class Response
 * @brief Represents the result of an operation, encapsulating a status.
 *
 * Used as a base response type for various operations, indicating success, failure,
 * or other outcome as represented by {@link ResponseStatus}.
 */
public class Response {

    /** @brief The status of the response. */
    private final ResponseStatus status;

    /**
     * @brief Constructs a new Response with the specified status.
     *
     * @param status The {@link ResponseStatus} of the operation. Must not be null.
     * @throws NullPointerException if the status is null.
     */
    Response(ResponseStatus status) {
        this.status = Objects.requireNonNull(status, "Status must not be null");
    }

    /**
     * @brief Returns the status of the response.
     *
     * @return The {@link ResponseStatus} associated with this response.
     */
    public ResponseStatus getStatus() {
        return status;
    }
}

