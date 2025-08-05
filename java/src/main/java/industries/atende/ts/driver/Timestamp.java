/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.time.Instant;

/**
 * @class Timestamp
 * @brief Utility class for converting between custom TStorage-resolution timestamps and Unix time.
 *
 * This class provides static methods to convert between a nanosecond-resolution timestamp
 * based on the epoch starting at 2001-01-01 (used in TStorage) and the standard Unix epoch
 * (1970-01-01T00:00:00Z). The conversion uses {@link java.time.Instant}.
 *
 * All timestamps are represented as {@code long} values in nanoseconds.
 */
public class Timestamp {

    /** @brief The difference in seconds between 2001-01-01 and 1970-01-01. */
    static final long DIFF_2001_1970_s = 978307200;

    /** @brief Constant representing one billion, used for nanosecond-to-second conversion. */
    static final long BILLION = 1_000_000_000L;

    /**
     * @brief Converts a custom nanosecond-based timestamp (since 2001-01-01) to Unix {@link Instant}.
     *
     * @param timestamp The timestamp in nanoseconds since 2001-01-01.
     * @return An {@link Instant} representing the same point in time in the Unix epoch.
     */
    public static Instant toUnix(long timestamp) {
        return Instant.ofEpochSecond(timestamp / BILLION + DIFF_2001_1970_s);
    }

    /**
     * @brief Converts a Unix {@link Instant} to a nanosecond-based timestamp since 2001-01-01.
     *
     * @param timestamp The {@link Instant} to convert.
     * @return The equivalent timestamp as a {@code long} in nanoseconds since 2001-01-01.
     */
    public static long fromUnix(Instant timestamp) {
        return (timestamp.getEpochSecond() - DIFF_2001_1970_s) * BILLION;
    }

    /**
     * @brief Returns the current timestamp in nanoseconds since 2001-01-01.
     *
     * @return The current timestamp as a {@code long} value.
     */
    public static long now() {
        return Timestamp.fromUnix(Instant.now());
    }
}

