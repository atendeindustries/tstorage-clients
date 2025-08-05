package industries.atende.ts;

import industries.atende.ts.driver.*;
import industries.atende.ts.driver.Record;

import java.nio.ByteOrder;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Optional;

class Globals {
    static int MILLION = 1000000;
    static ByteOrder order = ByteOrder.LITTLE_ENDIAN;
}

class PayloadTypeByteArray implements PayloadType<byte[]> {

    public PayloadTypeByteArray() {
    }

    @Override
    public byte[] toBytes(byte[] value) {
        return value;
    }

    @Override
    public Optional<byte[]> fromBytes(byte[] buffer) {
        return Optional.ofNullable(buffer);
    }

}

/**
 * User must provide his own RecordsSet implementation to use channel's put* API.
 * @param <T>
 */
class RecordsSetImpl<T> implements RecordsSet<T> {

    ArrayList<Record<T>> records = new ArrayList<>(Globals.MILLION);

    @Override
    public void append(Key key, T t) {
        records.add(new Record<>(key, t));
    }

    @Override
    public Iterable<Record<T>> iterator() {
        return records;
    }

    @Override
    public int size() {
        return records.size();
    }
}

public class Main {

    static byte[] hexStringToByteArray(String hex) {
        int len = hex.length();
        if (len % 2 != 0) {
            throw new IllegalArgumentException("Hex string must have even number of characters");
        }

        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(hex.charAt(i), 16) << 4)
                + Character.digit(hex.charAt(i + 1), 16));
        }
        return data;
    }

    /**
     * Read records to be stored with PUT call
     * All records have their acq set to -1
     */
    static <T> RecordsSetImpl<T> getRecordsForPUT(Path csvFilePath, PayloadType<T> payloadType) {

        var recs = new RecordsSetImpl<T>();

        try (BufferedReader reader = Files.newBufferedReader(csvFilePath)) {
            String line;
            while ((line = reader.readLine()) != null) {
                String[] parts = line.trim().split(",");
                if (parts.length != 5) continue; // no acq

                int f0 = Integer.parseInt(parts[0]); // cid
                long f1 = Long.parseLong(parts[1]); // mid
                int f2 = Integer.parseInt(parts[2]); // moid
                long f3 = Long.parseLong(parts[3]); // cap
                String f4 = parts[4]; // payload

                T t = payloadType.fromBytes(hexStringToByteArray(f4)).get();

                recs.append(
                    new Key(f0, f1, f2, f3, -1),
                    t
                );
            }
        } catch (IOException e) {
            System.out.println(e.getMessage());
        }

        return recs;
    }

    static String readHost(String[] args) {
        return args[0];
    }

    static int readPort(String[] args) {
        return Integer.parseInt(args[1]); // 2025
    }

    static Key readMinKey(String[] args) {
        return new Key(
            Integer.parseInt(args[2]), // cid
            Long.parseLong(args[3]), // mid
            Integer.parseInt(args[4]), // moid
            Long.parseLong(args[5]), //cap
            Long.parseLong(args[6]) // acq
        );
    }

    static Key readMaxKey(String[] args) {
        int offset = 7;
        return new Key(
            Integer.parseInt(args[offset]), // cid
            Long.parseLong(args[offset + 1]), // mid
            Integer.parseInt(args[offset + 2]), // moid
            Long.parseLong(args[offset + 3]), // cap
            Long.parseLong(args[offset + 4]) // acq
        );
    }

    static Path readCsvFilePath(String[] args) {
        return Paths.get(args[12]);
    }

    static <T> String recsToString(RecordsSet<T> recs, PayloadType<T> payloadType) {
        StringBuilder sb = new StringBuilder();
        for (var rec : recs.iterator()) {
            sb.append(rec.key().cid());
            sb.append(",");
            sb.append(rec.key().mid());
            sb.append(",");
            sb.append(rec.key().moid());
            sb.append(",");
            sb.append(rec.key().cap());
            sb.append(",");
            sb.append(rec.key().acq());
            sb.append(",");
            sb.append(bytesToHex(payloadType.toBytes(rec.value())));
            sb.append("\n");
        }
        if (!sb.isEmpty()) {
            sb.deleteCharAt(sb.length() - 1);
        }
        return sb.toString();
    }

    private static final byte[] HEX_ARRAY = "0123456789abcdef".getBytes(StandardCharsets.US_ASCII);
    public static String bytesToHex(byte[] bytes) {
        byte[] hexChars = new byte[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = HEX_ARRAY[v >>> 4];
            hexChars[j * 2 + 1] = HEX_ARRAY[v & 0x0F];
        }
        return new String(hexChars, StandardCharsets.UTF_8);
    }

    public static void main(String[] args) {

        try {
            if (args.length < 13) {
                System.out.println("Usage: java -cp \"ts_driver_java-1.0.0.jar:ts_driver_java_tester-1.0.0.jar\" industries.atende.ts.Main <0:host> <1:port> <2:cid1> <3:mid1> <4:moid1> <5:cap1> <6:acq1> <7:cid2> <8:mid2> <9:moid2> <10:cap2> <11:acq2> <12:csvFilePath>");
                return;
            }

            String host = readHost(args);
            int port = readPort(args);

            Key min = readMinKey(args);
            Key max = readMaxKey(args);

            Path csvFilePath = readCsvFilePath(args);

            PayloadTypeByteArray payloadType = new PayloadTypeByteArray();

            var csvRecordsSet = getRecordsForPUT(csvFilePath, payloadType);

            Channel<byte[]> channel = new Channel<>(host, port, 1000, payloadType, Globals.order);

            Response response = channel.connect();
            if (response.getStatus() != ResponseStatus.OK) {
                System.out.println("Cannot connect to " + host + ":" + port);
                return;
            }

            var respPut = channel.put(csvRecordsSet);
            if (respPut.getStatus() != ResponseStatus.OK) {
                System.out.println("Cannot put " + csvFilePath + " of size=" + csvRecordsSet.size() + " to " + host + ":" + port);
                return;
            }

            var respGET = channel.get(min, max, 4 + 8 + 1000000 * 100 + 4 + 4 + 8 + 8);
            if (respGET.getStatus() != ResponseStatus.OK) {
                System.out.println("Cannot get data from " + host + ":" + port);
                return;
            }

            String stdOutput = recsToString(respGET.getData(), payloadType);
            if (!stdOutput.isEmpty()) {
                System.out.println(stdOutput);
            }

            response = channel.close();
            if (response.getStatus() != ResponseStatus.OK) {
                System.out.println("Cannot close channel " + host + ":" + port);
            }
        } catch (Exception e) {
            System.out.println(e.getMessage());
        }
    }

}
