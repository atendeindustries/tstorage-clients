/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

/**
 * @class Key
 * @brief Represents a unique identifier used in TStorage addressing.
 *
 * A Key consists of five components:
 * - cid: Client ID
 * - mid: Meter ID
 * - moid: Meter Object ID
 * - cap: Capture timestamp
 * - acq: Acquisition timestamp
 *
 * @note The `cid` must be greater than or equal to 0.
 */
public record Key(int cid, long mid, int moid, long cap, long acq) {

    /** @brief Error message when an illegal cid value is provided. */
    private static final String STR_ILLEGAL_CID = "Cid value must not be less than 0";

    /** @brief Number of bytes used to store the Client ID. */
    static final int BYTES_CID = Integer.BYTES;

    /** @brief Number of bytes used to store the Meter ID. */
    static final int BYTES_MID = Long.BYTES;

    /** @brief Number of bytes used to store the Meter Object ID. */
    static final int BYTES_MOID = Integer.BYTES;

    /** @brief Number of bytes used to store the Capture timestamp. */
    static final int BYTES_CAP = Long.BYTES;

    /** @brief Number of bytes used to store the Acquisition timestamp. */
    static final int BYTES_ACQ = Long.BYTES;

    /** @brief Total number of bytes required to store a Key. */
    static final int BYTES = BYTES_CID + BYTES_MID + BYTES_MOID + BYTES_CAP + BYTES_ACQ;

    /** @brief Minimum allowed value for the Client ID. */
    static final int CID_MIN_VALUE = 0;

    /**
     * @brief Returns a Key instance with the minimum value for each field.
     *
     * @return A Key where cid = 0, and all other fields are set to their minimum values.
     */
    public static Key min() {
        return new Key(CID_MIN_VALUE, Long.MIN_VALUE, Integer.MIN_VALUE, Long.MIN_VALUE,
            Long.MIN_VALUE);
    }

    /**
     * @brief Returns a Key instance with the maximum value for each field.
     *
     * @return A Key where all fields are set to their maximum values.
     */
    public static Key max() {
        return new Key(Integer.MAX_VALUE, Long.MAX_VALUE, Integer.MAX_VALUE, Long.MAX_VALUE,
            Long.MAX_VALUE);
    }

    /**
     * @brief Constructs a new Key.
     *
     * @param cid  Client ID. Must be greater than or equal to 0.
     * @param mid  Meter ID.
     * @param moid Meter Object ID.
     * @param cap  Capture timestamp.
     * @param acq  Acquisition timestamp.
     *
     * @throws IllegalArgumentException if cid is less than 0.
     */
    public Key {
        if (cid < CID_MIN_VALUE) {
            throw new IllegalArgumentException(STR_ILLEGAL_CID);
        }
    }
}

