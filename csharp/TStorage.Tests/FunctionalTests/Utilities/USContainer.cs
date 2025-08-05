/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Tests.FunctionalTests.Utilities
{
    public class USContainer(string arguments = "")
    : BaseServer(TestConfig.USContainerBinPath, TestConfig.USContainerLogPath, TestConfig.USContainerParamsPath + arguments)
    {
        public override void Start()
        {
            CleanPartitions();
            base.Start();
        }

        public static void CleanPartitions()
        {
            string rootDir = TestConfig.USContainerSacksPath;
            if (Directory.Exists(rootDir))
            {
                string[] dirs = Directory.GetDirectories(rootDir);
                foreach (string dir in dirs)
                {
                    foreach (string file in Directory.GetFiles(dir))
                    {
                        File.Delete(file);
                    }
                }
            }
        }
    }
}
