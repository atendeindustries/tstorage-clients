/*
 * Copyright 2025 Atende Industries
 */

using TStorage.Interfaces;

namespace TStorage.Tests.FunctionalTests.Utilities
{
    public class PayloadTypeBytes : IPayloadType<byte[]>
    {
        public byte[]? FromBytes(byte[] bytes)
        {
            return bytes;
        }

        public byte[] ToBytes(byte[] value)
        {
            return value;
        }
    }
}