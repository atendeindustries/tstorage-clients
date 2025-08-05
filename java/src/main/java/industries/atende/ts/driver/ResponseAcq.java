/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

/**
 * @class ResponseAcq
 * @brief Represents the result of an getAcq operation, including acquisition timestamp.
 *
 * Extends {@link Response} by adding an acquisition timestamp (`acq`).
 */
public class ResponseAcq extends Response {

    /** @brief Acquisition timestamp returned by the operation. */
    private final long acq;

    /** @brief Special constant indicating an invalid or unavailable acquisition time. */
    static final long INVALID_ACQ = -1L;

    /**
     * @brief Constructs a new ResponseAcq with the given status and acquisition time.
     *
     * @param status The {@link ResponseStatus} of the operation.
     * @param acq    The acquisition timestamp. May be {@code INVALID_ACQ} if invalid.
     */
    ResponseAcq(ResponseStatus status, long acq) {
        super(status);
        this.acq = acq;
    }

    /**
     * @brief Constructs a new ResponseAcq with the given status and an invalid acquisition time.
     *
     * @param status The {@link ResponseStatus} of the operation.
     */
    ResponseAcq(ResponseStatus status) {
        this(status, ResponseAcq.INVALID_ACQ);
    }

    /**
     * @brief Returns the acquisition timestamp.
     *
     * @return The acquisition time, or {@code INVALID_ACQ} if unavailable.
     */
    public long getAcq() {
        return acq;
    }
}

