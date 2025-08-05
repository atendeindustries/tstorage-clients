/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.net.UnknownHostException;
import java.util.ArrayList;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

public class ChannelNotConnectedTests {

    private Channel<Integer> channel;

    @BeforeEach
    void createSampleChannel() {
        try {
            channel = new Channel<>(Globals.LOCALHOST, Globals.PORT, Globals.SOCKET_TIMEOUT, Globals.PAYLOAD_TYPE_INT, Globals.order);
        } catch (UnknownHostException ignored) {
        }
    }

    @Test
    void ChannelClosingReturnsOKTest() {
        assertSame(ResponseStatus.OK, channel.close().getStatus());
    }

    @Test
    void ChannelGETReturnsERRORTest() {
        assertSame(ResponseStatus.ERROR, channel.get(Key.min(), Key.max(), Globals.RESPONSE_BYTES_LIMIT).getStatus());
    }

    @Test
    void ChannelGETAcqReturnsERRORTest() {
        var responseAcq = channel.getAcq(Key.min(), Key.max());
        assertSame(ResponseStatus.ERROR, responseAcq.getStatus());
        assertEquals(ResponseAcq.INVALID_ACQ, responseAcq.getAcq());
    }

    @Test
    void ChannelGETStreamReturnsERRORTest() {
        assertSame(ResponseStatus.ERROR, channel.getStream(Key.min(), Key.max(), Globals.RESPONSE_BYTES_LIMIT, (recordSet) -> {
            return null;
        }).getStatus());
    }

    @Test
    void ChannelPUTReturnsERRORTest() {
        assertSame(ResponseStatus.ERROR, channel.put(new _RecordsSetCollection<>(new ArrayList<>())).getStatus());
    }

    @Test
    void ChannelPUTAReturnsERRORTest() {
        assertSame(ResponseStatus.ERROR, channel.puta(new _RecordsSetCollection<>(new ArrayList<>())).getStatus());
    }
}
