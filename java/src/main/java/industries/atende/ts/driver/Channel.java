/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.IOException;
import java.net.UnknownHostException;
import java.nio.ByteOrder;
import java.util.Objects;
import java.util.function.Function;

/**
 * @class Channel
 * @brief A generic channel for sending and receiving data over a network connection to TStorage.
 *
 * @tparam T The payload type for communication.
 *
 * This class manages the lifecycle of a network connection and supports operations
 * like `get`, `put`, `puta`, `getAcq`, and streaming data with `getStream`.
 *
 * It is strongly recommended that the Channel be promptly closed if an error occurs.
 */
public class Channel<T> {

    private final __net_Connection connection;
    private final PayloadType<T> payloadType;
    private ByteOrder order;

    /**
     * @brief Constructs a Channel with an existing connection.
     *
     * @param connection The underlying network connection.
     * @param payloadType The payload type handler.
     * @param order The byte order for communication.
     */
    Channel(__net_Connection connection, PayloadType<T> payloadType, ByteOrder order) {
        this.connection = Objects.requireNonNull(connection, "Connection must not be null");
        this.payloadType = Objects.requireNonNull(payloadType, "PayloadType must not be null");
        this.order = Objects.requireNonNull(order, "Order must not be null");
    }

    /**
     * @brief Constructs a Channel with connection parameters.
     *
     * @param host The remote host.
     * @param port The remote port.
     * @param timeout Connection timeout in milliseconds.
     * @param payloadType The payload type handler.
     * @param order The byte order for communication.
     *
     * @throws UnknownHostException If the host is unknown.
     * @throws IllegalArgumentException If arguments are invalid.
     */
    public Channel(String host, int port, int timeout, PayloadType<T> payloadType, ByteOrder order)
        throws UnknownHostException, IllegalArgumentException {
        this(new __net_ConnectionSocket(host, port, timeout), payloadType, order);
    }

    /**
     * @brief Gets the current byte order.
     * @return The byte order.
     */
    public ByteOrder getOrder() {
        return order;
    }

    /**
     * @brief Sets the byte order.
     * @param order The byte order to use.
     */
    public void setOrder(ByteOrder order) {
        this.order = order;
    }

    /**
     * @brief Opens the network connection.
     * @return Response indicating success or failure.
     */
    public Response connect() {
        try {
            connection.open();
            return new Response(ResponseStatus.OK);
        } catch (IOException e) {
            return new Response(ResponseStatus.ERROR);
        }
    }

    /**
     * @brief Closes the network connection.
     * @return Response indicating success or failure.
     */
    public Response close() {
        try {
            connection.close();
            return new Response(ResponseStatus.OK);
        } catch (IOException e) {
            return new Response(ResponseStatus.ERROR);
        }
    }

    /**
     * @brief Retrieves records within a key range.
     *
     * @param min The minimum key.
     * @param max The maximum key.
     * @param responseBytesLimit The byte limit of the TStorage response.
     *
     * @return A ResponseGet containing the result or an error.
     *
     * @throws IllegalArgumentException If parameters are invalid.
     */
    public ResponseGet<T> get(Key min, Key max, int responseBytesLimit) throws IllegalArgumentException {
        try {
            return get(
                new __proto_GET<>(
                    connection.getOutputStream(),
                    connection.getInputStream(),
                    payloadType,
                    getOrder(),
                    responseBytesLimit
                ),
                min,
                max
            );
        } catch (IOException ignore) {
            return new ResponseGet<>(ResponseStatus.ERROR);
        }
    }

    /**
     * @brief Internal GET execution with provided protocol.
     *
     * @param get The GET protocol instance.
     * @param min Minimum key.
     * @param max Maximum key.
     *
     * @return The result of the operation.
     */
    ResponseGet<T> get(__proto_GET<T> get, Key min, Key max) {
        return get.run(min, max);
    }

    /**
     * @brief Sends records to TStorage using safe PUT.
     *
     * @param data The records to be sent.
     * @return Response indicating success or failure.
     */
    public Response put(RecordsSet<T> data) {
        try {
            return put(
                new __proto_PUTs<>(
                    __proto_PUTsRequest.RequestType.PUTSAFE,
                    connection.getOutputStream(),
                    connection.getInputStream(),
                    payloadType,
                    getOrder()
                ),
                data
            );
        } catch (IOException e) {
            return new Response(ResponseStatus.ERROR);
        }
    }

    /**
     * @brief Internal PUT execution with provided protocol.
     *
     * @param put The PUT protocol instance.
     * @param data The records to send.
     * @return The result of the operation.
     */
    Response put(__proto_PUTs<T> put, RecordsSet<T> data) {
        return put.run(data);
    }

    /**
     * @brief Sends records to TStorage using safe PUTA.
     *
     * @param data The records to be sent.
     * @return Response indicating success or failure.
     */
    public Response puta(RecordsSet<T> data) {
        try {
            return puta(
                new __proto_PUTs<>(
                    __proto_PUTsRequest.RequestType.PUTASAFE,
                    connection.getOutputStream(),
                    connection.getInputStream(),
                    payloadType,
                    getOrder()
                ),
                data
            );
        } catch (IOException e) {
            return new Response(ResponseStatus.ERROR);
        }
    }

    /**
     * @brief Internal PUTA execution with provided protocol.
     *
     * @param puta The PUTA protocol instance.
     * @param data The records to send.
     * @return The result of the operation.
     */
    Response puta(__proto_PUTs<T> puta, RecordsSet<T> data) {
        return puta.run(data);
    }

    /**
     * @brief Gets acquisition value within a key range.
     *
     * @param min Minimum key.
     * @param max Maximum key.
     *
     * @return ResponseAcq containing result or error.
     */
    public ResponseAcq getAcq(Key min, Key max) {
        try {
            return getAcq(
                new __proto_GETAcq(
                    connection.getOutputStream(),
                    connection.getInputStream(),
                    getOrder()
                ),
                min,
                max
            );
        } catch (IOException e) {
            return new ResponseAcq(ResponseStatus.ERROR);
        }
    }

    /**
     * @brief Internal GETAcq execution with provided protocol.
     *
     * @param getAcq The acquisition protocol instance.
     * @param min Minimum key.
     * @param max Maximum key.
     *
     * @return The result of the operation.
     */
    ResponseAcq getAcq(__proto_GETAcq getAcq, Key min, Key max) {
        return getAcq.run(min, max);
    }

    /**
     * @brief Gets streaming data in chunks from TStorage with a callback on each chunk.
     *
     * The chunk size is limited by responseBytesLimit. When this limit is reached,
     * the callback is invoked with the current chunk. If the limit is too small to deliver
     * even a single valid chunk, an error response is returned.
     *
     * @param min Minimum key.
     * @param max Maximum key.
     * @param responseBytesLimit Maximum size in bytes for each chunk (including protocol overhead).
     * @param callback User-defined function called with each received chunk.
     *
     * @return ResponseAcq containing result or error.
     */
    public ResponseAcq getStream(Key min, Key max, int responseBytesLimit, Function<RecordsSet<T>, Void> callback) {
        try {
            return getStream(
                new __proto_GETStream<>(
                    connection.getOutputStream(),
                    connection.getInputStream(),
                    payloadType,
                    getOrder(),
                    responseBytesLimit,
                    callback
                ),
                min,
                max
            );
        } catch (IOException ignore) {
            return new ResponseAcq(ResponseStatus.ERROR, ResponseAcq.INVALID_ACQ);
        }
    }

    /**
     * @brief Internal GETStream execution with provided protocol.
     *
     * @param getStream The GETStream protocol instance.
     * @param min Minimum key.
     * @param max Maximum key.
     *
     * @return The result of the streaming operation.
     */
    ResponseAcq getStream(__proto_GETStream<T> getStream, Key min, Key max) {
        return getStream.run(min, max);
    }
}
