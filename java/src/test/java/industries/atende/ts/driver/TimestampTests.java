package industries.atende.ts.driver;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

class TimestampTests {

    @Test
    void toUnix() {
        var withns = Timestamp.toUnix(765462944303038001L);
        var nons = Timestamp.toUnix(765462944000000000L);
        assertEquals(1743770144, withns.getEpochSecond());
        assertEquals(1743770144, nons.getEpochSecond());
        assertEquals(withns.toString(), nons.toString());
    }

    @Test
    void fromUnix() {
        var tajm = Timestamp.fromUnix(Timestamp.toUnix(765462944303038001L));
        assertEquals(765462944000000000L, tajm);
    }

}