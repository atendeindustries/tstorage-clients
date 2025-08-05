using TStorage.Interfaces;

namespace TStorage.Tests.UnitTests
{
    public class RecordSetTests
    {
        [Fact]
        public void GetEnumerator_InitiallyEnumeratorIsPositionedBeforeFirstElementInRecordSet()
        {
            // Arange
            RecordsSet<int> records = new();
            records.Append(new());

            // Act and assert
            Assert.Throws<InvalidOperationException>(() => records.GetEnumerator().Current);
        }

        [Fact]
        public void GetEnumerator_MoveNextShouldMoveEnumeratorForward()
        {
            // Arange
            RecordsSet<int> records = new();
            Record<int> providedRecord = new(Key.Min(), 10);
            records.Append(providedRecord);

            // Act and assert
            var enumerator = records.GetEnumerator();
            Assert.True(enumerator.MoveNext());
            Assert.Equal(providedRecord, enumerator.Current);
        }

        [Fact]
        public void Size_NoElementsAfterInitialization()
        {
            // Arange
            RecordsSet<int> records = new();

            // Act and assert
            Assert.Equal(0, records.Size);
        }

        [Fact]
        public void Append_SizePropertyCorrectlyTracksNumberOfData()
        {
            // Arange
            RecordsSet<int> records = new();

            // Act
            records.Append(new(Key.Min(), 1));

            // Assert
            Assert.Equal(1, records.Size);
        }

        [Fact]
        public void GetEnumerator_GoingThroughCollectionCorrectly()
        {
            // Arange
            RecordsSet<int> records = new();
            records.Append(new(Key.Min(), 1));
            records.Append(new(Key.Min(), 2));
            records.Append(new(Key.Min(), 3));

            // Act
            var enumerator = records.GetEnumerator();
            int counter = 0;
            while (enumerator.MoveNext())
            {
                counter++;
            }

            // Assert
            Assert.False(enumerator.MoveNext());
            Assert.Equal(3, counter);
        }
    }
}