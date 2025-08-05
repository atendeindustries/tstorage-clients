/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Tests.FunctionalTests.Utilities
{
    public class USConnector(string arguments = "")
    : BaseServer(TestConfig.USConnectorBinPath, TestConfig.USConnectorLogPath, TestConfig.USConnectorParamsPath + arguments)
    { }
}
