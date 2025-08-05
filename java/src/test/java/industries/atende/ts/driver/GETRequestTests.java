/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import org.junit.jupiter.api.Test;

import java.io.*;
import java.nio.ByteOrder;


public class GETRequestTests {

    static ByteOrder order = Globals.order;

    @Test
    void testGETRequestToBytes() {
        var socketOutputStreamMock = new ByteArrayOutputStream();
        var getOut = new __proto_ProtocolOutputStream(socketOutputStreamMock, order);
        var writer = new __proto_GETWriterInPlace(getOut);
        writer.write(new __proto_GETRequest(Key.min(), Key.max()));
        var bytes = socketOutputStreamMock.toByteArray();
        Globals.assertGETRequestAsBytes(bytes, Key.min(), Key.max());
    }

}