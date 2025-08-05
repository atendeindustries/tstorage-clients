/*
 * Copyright 2025 Atende Industries
 */

using Microsoft.Extensions.Configuration;

namespace TStorage.Tests.FunctionalTests.Utilities
{
    public class TestConfig
    {
        public static string DtimestampBinPath => _config.Value.GetValue<string>("TestConfig:dtimestampBinPath") ?? string.Empty;
        public static string DtimestampParamsPath => _config.Value.GetValue<string>("TestConfig:dtimestampParamsPath") ?? string.Empty;
        public static string DtimestampLogPath => _config.Value.GetValue<string>("TestConfig:dtimestampLogPath") ?? string.Empty;

        public static string USContainerBinPath => _config.Value.GetValue<string>("TestConfig:uscontainerBinPath") ?? string.Empty;
        public static string USContainerParamsPath => _config.Value.GetValue<string>("TestConfig:uscontainerParamsPath") ?? string.Empty;
        public static string USContainerLogPath => _config.Value.GetValue<string>("TestConfig:uscontainerLogPath") ?? string.Empty;
        public static string USContainerSacksPath => _config.Value.GetValue<string>("TestConfig:uscontainerSacksPath") ?? string.Empty;

        public static string USConnectorBinPath => _config.Value.GetValue<string>("TestConfig:usconnectorBinPath") ?? string.Empty;
        public static string USConnectorParamsPath => _config.Value.GetValue<string>("TestConfig:usconnectorParamsPath") ?? string.Empty;
        public static string USConnectorLogPath => _config.Value.GetValue<string>("TestConfig:usconnectorLogPath") ?? string.Empty;

        public static string USTrackerBinPath => _config.Value.GetValue<string>("TestConfig:ustrackerBinPath") ?? string.Empty;
        public static string USTrackerParamsPath => _config.Value.GetValue<string>("TestConfig:ustrackerParamsPath") ?? string.Empty;
        public static string USTrackerLogPath => _config.Value.GetValue<string>("TestConfig:ustrackerLogPath") ?? string.Empty;

        public static int NetworkToleranceIntervalMs => _config.Value.GetValue<int>("TestConfig:networkToleranceIntervalMs");

        public static string USConnectorListenAddres => _config.Value.GetValue<string>("TestConfig:usconnectorListenAddres") ?? string.Empty;
        public static int USConnectorListenPort => _config.Value.GetValue<int>("TestConfig:usconnectorListenPort");

        private static readonly Lazy<IConfigurationRoot> _config = new(LoadConfiguration);
        private static IConfigurationRoot LoadConfiguration()
        {
            return new ConfigurationBuilder()
                .AddJsonFile("FunctionalTests/Config/TestConfig.json", optional: false, reloadOnChange: false)
                .Build();
        }
        private TestConfig() { }
    }
}