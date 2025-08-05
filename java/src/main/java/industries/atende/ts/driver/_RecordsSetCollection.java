/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.util.Collection;
import java.util.Objects;

/**
 * Class implementing RecordsSet interface.
 * Takes in any Java Collection.
 * One class to fix them all.
 */
class _RecordsSetCollection<T> implements RecordsSet<T> {

    private final Collection<Record<T>> collection;

    _RecordsSetCollection(Collection<Record<T>> collection) {
        this.collection = Objects.requireNonNull(collection);
    }

    @Override
    public void append(Key key, T value) {
        collection.add(new Record<>(key, value));
    }

    @Override
    public Iterable<Record<T>> iterator() {
        return collection;
    }

    /**
     * Returns number of records stored within the collection.
     */
    @Override
    public int size() {
        return collection.size();
    }

    void clear() {
        collection.clear();
    }

}
