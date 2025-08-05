# TStorage Clients

This repository contains implementation of TStorage client libraries in C, C++, C#, Java, and Python. Client libraries implement communication with TStorage's Connector interface, giving access to the following methods:

GET(keyMin, keyMax) - reads records from 5 dimensional key range given by (keyMin keyMax) tuple.

GET_ACQ(keyMin, keyMax) - returns acq0 - trimmed acq value for the key range given by (keyMin keyMax) tuple.

PUT(records) - Writes records to TStorage. TStorage manages assigning acq key component to each record.

PUTA(records) - Writes records to TStorage. Client is responsible for assigning acq key component to each record.


For more details, see README.md files in a folder for a given language and TStorage documentation (https://docs.tstorage.eu/).
