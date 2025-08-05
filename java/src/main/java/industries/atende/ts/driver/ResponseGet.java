/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.util.Objects;

/**
 * @class ResponseGet
 * @brief Represents the result of a GET operation, including acquired records.
 *
 * Extends {@link ResponseAcq} by including a {@link RecordsSet} containing the retrieved data.
 * Even if the operation is successful, the data set may be empty.
 *
 * @tparam T The type of values stored in the returned records.
 */
public class ResponseGet<T> extends ResponseAcq {

    /** @brief The data retrieved by the GET operation. */
    private RecordsSet<T> data;

    /**
     * @brief Constructs a new ResponseGet with the specified status, acquisition time, and data.
     *
     * @param status The {@link ResponseStatus} of the operation.
     * @param acq    The acquisition timestamp associated with the data.
     * @param data   The {@link RecordsSet} of retrieved data. Must not be null.
     *
     * @throws NullPointerException if {@code data} is null.
     */
    ResponseGet(ResponseStatus status, long acq, RecordsSet<T> data) {
        super(status, acq);
        this.data = Objects.requireNonNull(data, "Data must not be null");
    }

    /**
     * @brief Constructs a new ResponseGet with the specified status and no data.
     *
     * The acquisition timestamp is set to {@code INVALID_ACQ}, and the data is not initialized.
     *
     * @param status The {@link ResponseStatus} of the operation.
     */
    ResponseGet(ResponseStatus status) {
        super(status, ResponseAcq.INVALID_ACQ);
    }

    /**
     * @brief Returns the data retrieved by the GET operation.
     *
     * @return The {@link RecordsSet} of data.
     *
     * @note An empty data set is a valid result for a successful operation.
     *       Use {@link Response#getStatus()} to determine if the operation was successful.
     */
    public RecordsSet<T> getData() {
        return data;
    }
}

