Forwarding (output plugin)
====================
Output plugin

Example configuration
---------------------

Below you see a complex configuration with all available output options enabled.
Don't forget to remove (or comment) outputs that you don't want to use!

.. code-block:: xml

    <output>
        <name>Forwarding output</name>
        <plugin>forwarding</plugin>
        <params>
            <protocol>TCP</protocol>
            <mode>round-robin</mode>
            <mtu>1500</mtu>

            <hosts>

                <host>
                    <name>Backup collector 1</name>
                    <ipv4>127.0.0.1</ipv4>
                    <port>4251</port>
                </host>

                <host>
                    <name>Backup collector 2</name>
                    <ipv4>127.0.0.1</ipv4>
                    <port>4252</port>
                </host>

            </hosts>
        </params>
    </output>

Parameters
----------

Formatting parameters:

:``protocol``:
    Convert TCP flags to common textual representation (formatted, e.g. ".A..S.")
    or to a number (raw). [values: formatted/raw, default: formatted]

:``mode``:
    Convert timestamp to ISO 8601 textual representation (all timestamps in UTC and milliseconds,
    e.g. "2018-01-22T09:29:57.828Z") or to a unix timestamp (all timestamps in milliseconds).
    [values: formatted/unix, default: formatted]

:``mtu``:
    Convert protocol identification to formatted style (e.g. instead 6 writes "TCP") or to a number.
    [values: formatted/raw, default: formatted]

----

:``hosts``:

    :``host``:
        :``name``: Identification name of the connected host.
        :``ipv4``: IPv4 address of the host.
        :``port``: Remote port of the host.

