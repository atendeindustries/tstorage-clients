using TStorage.Utilities;

namespace TStorage.Tests.UnitTests
{
    public class TimestampTests
    {
        private const int NANOSECONDS_PER_DATETIME_TICK = 100;

        /// <summary>
        /// TStorage starting epoch in <see cref="DateTime"/>.
        /// Used in tests as a base for calculations.
        /// </summary>
        private static readonly DateTime TStorageEpoch = new(year: 2001, month: 1, day: 1, hour: 0, minute: 0, second: 0, DateTimeKind.Utc);

        [Fact]
        public void Now_ShouldReturnTimestampCloseToCurrentTime()
        {
            // Act
            long before = Timestamp.FromUnix(DateTime.UtcNow);
            long result = Timestamp.Now();
            long after = Timestamp.FromUnix(DateTime.UtcNow);

            // Assert
            Assert.InRange(result, before, after);
        }

        [Theory]
        [InlineData(0)]
        [InlineData(1)]
        [InlineData(20)]
        [InlineData(300)]
        [InlineData(4000)]
        [InlineData(50000)]
        public void FromUnix_ForTStorageEpochIncreasedByVariousNumberOfTicks_ShouldReturnCorrectValues(int numberOfTicks)
        {
            // Arrange
            DateTime input = TStorageEpoch.AddTicks(numberOfTicks);

            // Act
            long result = Timestamp.FromUnix(input);

            // Assert
            long expectedTStorageTimestampNs = numberOfTicks * NANOSECONDS_PER_DATETIME_TICK;
            Assert.Equal(expectedTStorageTimestampNs, result);
        }

        [Theory]
        [InlineData(0)]
        [InlineData(100)]
        [InlineData(200)]
        [InlineData(3000)]
        [InlineData(50000)]
        [InlineData(-60000)]
        [InlineData(-70000)]
        public void ToUnix_ForVariousTStorageTimestamps_ShouldReturnCorrectValues(int nanoseconds)
        {
            // Act
            DateTime result = Timestamp.ToUnix(nanoseconds);

            // Assert
            long numberOfTicks = nanoseconds / NANOSECONDS_PER_DATETIME_TICK;
            DateTime expectedDateTime = TStorageEpoch.AddTicks(numberOfTicks);
            Assert.Equal(expectedDateTime, result);
        }

        [Fact]
        public void RoundTrip()
        {
            // Act
            DateTime now = DateTime.UtcNow;
            long tstorageTimestamp = Timestamp.FromUnix(now);
            DateTime convertedBack = Timestamp.ToUnix(tstorageTimestamp);

            // Assert
            Assert.Equal(now.Ticks, convertedBack.Ticks);
        }
    }
}