/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

/**
 * One of the reasons why I defined stand-alone reader/writer and not put methods
 * for sending request and receiving response inside one of the Protocol{Input|Output}Stream
 * is the fact that both read and write returns domain type object and
 * don't throw IOExceptions like read/write methods from streams do.
 * One more reason is that reader/writer should use input/output stream object not be one.
 * It seems it's better to separate how to make a sandwich from being one.
 * It can be seen better while testing where such methods would start referencing themselves.
 * @param <R> Data to be written.
 */
interface __proto_RequestWriter<R> {

    ResponseStatus write(R request);

}
