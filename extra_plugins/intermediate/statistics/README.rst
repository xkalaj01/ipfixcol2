Statistics (intermediate plugin)
===================================

Intermediate plugin Statistics is intended to collect operational parameters, store them and provide
way of their export. Data are stored in structures which are based on MIB module IPFIX-MIB, so most
of the elements included in this MIB module are implemented in the internal structures as well.

Currently there is only one submodule that can be used for data export, which is SNMP submodule.
This submodule uses Net-SNMP library, thus this library is needed to be installed for compilation
of whole plugin. SNMP submodule uses snmpd-demon to receive and dispatch SNMP request, so snmp is
also needed to be installed. After start the submodule registers to snmp-demon as AgentX, so there
is no need to configure IP addresses or user accounts for SNMP communication inside of the plugin
as the snmp-demon handles this itself.

Net-SNMP library offers cache mechanism, which is also implemented in SNMP submodule. Caches are used
for storing information for export. These information are transferred from internal data structure
every time the cache expires and new SNMP request appears. Timeout for cache expiration of each MIB
table can be defined in configuration file. If you wish to get the most actual data from collector,
set the timeout to low values (lowest possible value is 1). If you want to save resources and accesses
to internal structures you can adjust the values to your need. Default value of the timeout is set to 1s,
so if you pull values occasionally you don't have to bother with setting the timeouts as the cache will
be mostly in expired state and it will update itself with the new request. But if you use some SNMP manager
application for pulling the values periodically, it is recommended to adjust the timeouts accordingly.

How to build
------------

By default, the plugin is not distributed with IPFIXcol due to extra dependencies.
To build the plugin, IPFIXcol (and its header files) and the following dependencies must be
installed on your system:

- `Net-SNMP <http://www.net-snmp.org/>`_

You can install necessary Net-SNMP packages using this command:

.. code-block:: sh

    # Ubuntu distributions
    $ sudo apt-get install libsnmp-base libsnmp-dev snmpd
    # Fedora distributions
    $ sudo dnf install net-snmp net-snmp-dev

After all dependencies are met, you NEED TO check AgentX settings in file /etc/snmp/snmpd.conf
This file must contain line

.. code-block:: sh

    master agentx

And the line must be uncommented (without # at the beginning of the line).
After the changes, restart and check status of the snmpd service

.. code-block:: sh

    $ sudo service snmpd restart
    $ sudo service snmpd status

You should see that the service is active and running and you should also see line
"Turning on AgentX master support"

Finally, compile and install the plugin:

.. code-block:: sh

    $ mkdir build && cd build && cmake ..
    $ make
    # make install


Example configuration
---------------------
.. code-block:: xml

    <intermediate>
        <name>Collector stats</name>
        <plugin>statistics</plugin>
        <params>
            <sessionActivityTimeout>20</sessionActivityTimeout>
            <outputs>
            <snmp>
                <cacheTimeout table="ipfixTransportSessionTable">10</cacheTimeout>
                <cacheTimeout table="ipfixTemplateTable">5</cacheTimeout>
                <cacheTimeout table="ipfixTemplateDefinitionTable">5</cacheTimeout>
                <cacheTimeout table="ipfixTransportSessionStatsTable">1</cacheTimeout>
                <cacheTimeout table="ipfixTemplateStatsTable">5</cacheTimeout>
            </snmp>
            </outputs>
        </params>
    </intermediate>

Parameters
----------

:``sessionActivityTimeout``:
    One of the information stored about session is activity of the session, which indicates active transmission
    between exporter and collector. This timeout is used to determine, when the session becomes inactive if no
    data are being transmitted. [default:10]

:``outputs``:
    Specifying output submodules, which are used for data export from the plugin. Currently only submodule
    available is SNMP module.

    :``snmp``:
        SNMP output submodule. Just by specifying this tag without any extra parameters enables the submodule
        which is initialized with default values

        :``timeout``:
            Timeout of cache expiration for MIB tables. Name of the table must be specified in ``table`` argument.
            Value can be only positive integer. [default:1]

Notes
-----

Coming soon
