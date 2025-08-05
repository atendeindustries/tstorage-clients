using TStorage.Interfaces;

namespace TStorage.Tests.UnitTests
{
    public class KeyTests
    {
        [Fact]
        public void Min_ShouldReturnKeyWithMinValues()
        {
            // Act
            var minKey = Key.Min();

            // Assert
            Assert.Equal(Key.CID_MIN_VALUE, minKey.Cid);
            Assert.Equal(long.MinValue, minKey.Mid);
            Assert.Equal(int.MinValue, minKey.Moid);
            Assert.Equal(long.MinValue, minKey.Cap);
            Assert.Equal(long.MinValue, minKey.Acq);
        }

        [Fact]
        public void Max_ShouldReturnKeyWithMaxValues()
        {
            // Act
            var maxKey = Key.Max();

            // Assert
            Assert.Equal(int.MaxValue, maxKey.Cid);
            Assert.Equal(long.MaxValue, maxKey.Mid);
            Assert.Equal(int.MaxValue, maxKey.Moid);
            Assert.Equal(long.MaxValue, maxKey.Cap);
            Assert.Equal(long.MaxValue, maxKey.Acq);
        }

        [Fact]
        public void StructSize_ShouldReturnCorrectSize()
        {
            // Act
            var size = Key.StructSize();

            // Assert
            var expectedKeyStructSize = 32;
            Assert.Equal(expectedKeyStructSize, size);
        }

        [Fact]
        public void Constructor_ShouldThrowArgumentOutOfRangeException_ForInvalidCidValue()
        {
            // Act and assert
            int wrongCidValue = -1;
            Assert.Throws<ArgumentOutOfRangeException>(() => new Key(wrongCidValue, 0, 0, 0, 0));
        }

        [Fact]
        public void CompareTo_SameValuesShouldReturnZero()
        {
            // Arange
            Key key1 = new(1, 2, 3, 4, 5);
            Key key2 = new(1, 2, 3, 4, 5);

            // Act and assert
            Assert.Equal(0, key1.CompareTo(key2));
        }

        [Fact]
        public void CompareTo_FirstFieldDiffersShouldReturnNegative()
        {
            // Arange
            Key key1 = new(1, 0, 0, 0, 0);
            Key key2 = new(2, 0, 0, 0, 0);

            // Act and assert
            Assert.True(key1.CompareTo(key2) < 0);
            Assert.True(key2.CompareTo(key1) > 0);
        }

        [Fact]
        public void CompareTo_MiddleFieldDiffersShouldReturnCorrect()
        {
            // Arange
            Key key1 = new(1, 2, 3, 4, 5);
            Key key2 = new(1, 2, 4, 4, 5);

            // Act and assert
            Assert.True(key1.CompareTo(key2) < 0);
            Assert.True(key2.CompareTo(key1) > 0);
        }

        [Fact]
        public void Equals_SameValuesShouldReturnTrue()
        {
            // Arange
            Key key1 = new(1, 2, 3, 4, 5);
            Key key2 = new(1, 2, 3, 4, 5);

            // Act and assert
            Assert.True(key1.Equals(key2));
            Assert.True(key1 == key2);
            Assert.False(key1 != key2);
        }

        [Fact]
        public void Equals_DifferentValuesShouldReturnFalse()
        {
            // Arange
            Key key1 = new(1, 2, 3, 4, 5);
            Key key2 = new(1, 33, 3, 4, 5);

            // Act and assert
            Assert.False(key1.Equals(key2));
            Assert.False(key1 == key2);
            Assert.True(key1 != key2);
        }

        [Fact]
        public void CompareTo_ListSortWorksCorrectly()
        {
            // Arange
            List<Key> list = [
                new(30, 2, 3, 4, 5),
                new(20, 2, 3, 4, 5),
                new(10, 2, 3, 4, 5)];

            // Act
            list.Sort();

            // Assert
            Assert.Equal(10, list[0].Cid);
            Assert.Equal(20, list[1].Cid);
            Assert.Equal(30, list[2].Cid);
        }
    }
}
