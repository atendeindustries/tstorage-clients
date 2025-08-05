using TStorage.Interfaces;
using TStorage.Main;
using TStorage.Tests.FunctionalTests.Utilities;
using TStorage.Utilities;

namespace TStorage.Tests.FunctionalTests
{
    [Collection("SynchronisedTests")]
    public class GetAcqTests
    {
        [Fact]
        public void GetAcq_BasicTest()
        {
            // Arrange
            using var serverManager = new ServerManager();
            TestHelper.StartTStorage(serverManager);
            using Channel<byte[]> channel = new(serverManager.ListenAddress, serverManager.ListenPort, new PayloadTypeBytes());
            channel.Connect();

            // Act and assert
            var result = channel.GetAcq(Key.Min(), Key.Max());
            Assert.Equal(ResponseStatus.OK, result.Status);
            Assert.NotEqual(Key.Max().Acq, result.Acq);

            var requestMaxAcq = 1;
            Key minCustomKey = new(0, 0, 0, 0, 0);
            Key maxCustomKey = new(1, 1, 1, 1, requestMaxAcq);
            result = channel.GetAcq(minCustomKey, maxCustomKey);
            Assert.Equal(ResponseStatus.OK, result.Status);
            Assert.Equal(requestMaxAcq, result.Acq);
        }
    }
}