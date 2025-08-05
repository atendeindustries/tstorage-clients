/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Tests.FunctionalTests.Utilities
{
    public class USTracker(string arguments = "")
    : BaseServer(TestConfig.USTrackerBinPath, TestConfig.USTrackerLogPath, TestConfig.USTrackerParamsPath + arguments)
    { }
}
