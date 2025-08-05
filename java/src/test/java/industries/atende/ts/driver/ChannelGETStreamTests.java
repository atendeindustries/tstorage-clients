/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import org.junit.jupiter.api.Test;

import java.io.*;
import java.nio.ByteOrder;
import java.util.ArrayList;

import static org.junit.jupiter.api.Assertions.*;

public class ChannelGETStreamTests {

    static ByteOrder order = Globals.order;

    public abstract static class SocketMock implements __net_Connection {

        public ByteArrayOutputStream out;
        public ByteArrayInputStream in;
        public ArrayList<Record<byte[]>> inRecords;

        public SocketMock() {
            out = new ByteArrayOutputStream();
            in = new ByteArrayInputStream(new byte[0]);
            prepareResponseBytes();
        }

        @Override
        public void close() throws IOException {
            out.close();
            in.close();
        }

        @Override
        public InputStream getInputStream() throws IOException {
            return in;
        }

        @Override
        public OutputStream getOutputStream() throws IOException {
            return out;
        }

        @Override
        public void open() throws IOException {
        }

        protected void prepareResponseBytes() {
        }

    }

    @Test
    void testChannelBasicSuccess() {
        final int recsCount = 2;
        var sm = new SocketMock(){
            protected void prepareResponseBytes() {
                var builder = new GETResponseInputBuilder(order);
                builder.useSuccessFirstHeader();
                inRecords = builder.generateRecords(recsCount, 4); // One rec is 40 bytes
                builder.useSuccessSecondHeader();
                final long acq = Key.max().acq();
                builder.setAcq(acq);
                in = new ByteArrayInputStream(builder.build());
            }
        };
        var channel = new Channel<>(sm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        channel.connect();
        byte[] out = new byte[1];
        var responseAcq = channel.getStream(Key.min(), Key.max(), __proto_Header.BYTES*0 + recsCount * 40, (recs) -> {
            out[0]++;
            assertEquals(recsCount-1, recs.size());
            return null;
        });
        assertEquals(2, out[0]);
        assertEquals(Key.max().acq(), responseAcq.getAcq());
    }

    @Test
    void testChannelBasicSuccess2() {
        final int recsCount = 3;
        var sm = new SocketMock(){
            protected void prepareResponseBytes() {
                var builder = new GETResponseInputBuilder(order);
                builder.useSuccessFirstHeader();
                inRecords = builder.generateRecords(recsCount, 4); // One rec is 40 bytes
                builder.useSuccessSecondHeader();
                final long acq = Key.max().acq();
                builder.setAcq(acq);
                in = new ByteArrayInputStream(builder.build());
            }
        };
        var channel = new Channel<>(sm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        channel.connect();
        byte[] out = new byte[2];
        // 12 header bytes + 120 recs bytes + 4 terminating zero
        // 12 + 8 = 20 confirm bytes
        // So we split by 12 by moving terminating zero to confirm block which gives us 24 bytes:
        // 1*12 header + 10*12 recs + 2*12 terminating + confirm
        // 12*(1+10+2) = 12*13
        // The 13th last time will not be fired off by the limited stream as it's not going to reach it -
        // this would have been normally true, except that readNBytes is implemented in a way that after
        // it had read N bytes it always tries to read 0 bytes hence triggering the readExhausted...
        // User callback will be called after 4*12 bytes (we got 1 record) then 8+3*12 (second record),
        // finally 4+3*12 (third record) making up to 10 limit hits.
        try {
            var GETStream = new __proto_GETStream<>(
                sm.getOutputStream(),
                sm.getInputStream(),
                Globals.PAYLOAD_TYPE_BYTE_ARRAY,
                Globals.order,
                12,
                (recs) -> {
                    out[0]++;
                    assertEquals(1, recs.size()); // always one record
                    return null;
                }
            ) {
                @Override
                protected void flushRecords() {
                    super.flushRecords();
                    out[1]++;
                }
            };
            var responseAcq = channel.getStream(GETStream, Key.min(), Key.max());
            assertEquals(3, out[0]);
            assertEquals(13, out[1]);
            assertSame(ResponseStatus.OK, responseAcq.getStatus());
            assertEquals(Key.max().acq(), responseAcq.getAcq());
        } catch (IOException ignore) {
            fail();
        }
    }

}
