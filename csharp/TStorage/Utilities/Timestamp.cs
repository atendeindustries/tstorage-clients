/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Utilities
{
    /// <summary>
    /// Utility class for converting between custom TStorage-resolution timestamps and Unix time <see cref="DateTime"/>.
    /// This class provides static methods to convert between a nanosecond-resolution timestamp
    /// based on the epoch starting at 2001-01-01 (used in TStorage) and the standard Unix epoch.
    /// The functions accuracy is 1 Tick of DateTime which is equal to 100ns.
    /// </summary>
    public class Timestamp
    {
        /// <summary> Represents nanoseconds per DateTime.Tick. </summary>
        private const int NANOSECONDS_PER_DATETIME_TICK = (int)TimeSpan.NanosecondsPerTick;

        /// <summary> TStorage starting epoch in <see cref="DateTime"/>. </summary>
        private static readonly DateTime TStorageEpoch = new(year: 2001, month: 1, day: 1, hour: 0, minute: 0, second: 0, DateTimeKind.Utc);

        /// <summary>
        /// Converts the TStorage nanosecond-based timestamp (since 2001-01-01) to Unix <see cref="DateTime"/>.
        /// The function's accuracy is 1 Tick which is equal to 100ns. Conversion results are rounded down to the nearest Tick.
        /// </summary>
        /// <param name="timestamp"> The timestamp in nanoseconds since 2001-01-01. </param>
        /// <returns> A <see cref="DateTime"/> representing the same point in time in the Unix time. </returns>
        public static DateTime ToUnix(long timestamp)
        {
            return TStorageEpoch.AddTicks(timestamp / NANOSECONDS_PER_DATETIME_TICK);
        }

        /// <summary>
        /// Converts a Unix <see cref="DateTime"/> to a nanosecond-based timestamp since 2001-01-01.
        /// The function's accuracy is 1 Tick which is equal to 100ns.
        /// </summary>
        /// <param name="timestamp"> The Unix time <see cref="DateTime"/> to convert. </param>
        /// <returns> A <see cref="DateTime"/> representing the same time point in the Unix time. </returns>
        public static long FromUnix(DateTime timestamp)
        {
            if (timestamp.Kind != DateTimeKind.Utc)
            {
                timestamp = timestamp.ToUniversalTime();
            }
            return (timestamp - TStorageEpoch).Ticks * NANOSECONDS_PER_DATETIME_TICK;
        }

        /// <summary> Returns the current timestamp in nanoseconds since 2001-01-01. </summary>
        /// <returns> The current timestamp as a <see cref="long"/> value. </returns>
        public static long Now()
        {
            return FromUnix(DateTime.UtcNow);
        }
    }
}
