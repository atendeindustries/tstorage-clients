/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Tests.FunctionalTests.Utilities
{
    public class DTimestamp(string arguments = "")
    : BaseServer(TestConfig.DtimestampBinPath, TestConfig.DtimestampLogPath, TestConfig.DtimestampParamsPath + arguments)
    { }
}
